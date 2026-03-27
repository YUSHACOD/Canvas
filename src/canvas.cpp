
#include <base/include.cpp>

#include "base/interfaces.hpp"
#include "canvas_platform.hpp"
#include "canvas_game.hpp"

#include "renderer.hpp"

#include <cmath>
#include <cstring>
#include <math.h>

// #include "canvas_utils.cpp"
#include "interpolations.cpp"
#include "canvas_math.cpp"
#include "renderer.cpp"


//  Camera control : ----------------------------------------------------------------- (section)  //

internal void rot_cam_euler(render_camera* cam, f32 ax_radi, f32 ay_radi, f32 az_radi) {

    f32 cx = cosf(ax_radi), sx = sinf(ax_radi);
    f32 cy = cosf(ay_radi), sy = sinf(ay_radi);
    f32 cz = cosf(az_radi), sz = sinf(az_radi);

    // Combined rotation matrix R = Rz * Ry * Rx
    mat3 m = {0};
    V3_veci(m.vec1, cy * cz, cz * sx * sy - cx * sz, cx * cz * sy + sx * sz);
    V3_veci(m.vec2, cy * sz, cx * cz + sx * sy * sz, cx * sy * sz - cz * sx);
    V3_veci(m.vec3, -sy, cy * sx, cx * cy);

    cam->front = m * (cam->front);
    cam->side  = m * (cam->side);
    cam->up    = m * (cam->up);
}

inline void set_camera_to_look_at(render_camera* cam, v3 at, v3 pos) {

    V3_vecd(up, 0.0f, 1.0f, 0.0f);

    v3 front = normal(at - pos);
    v3 side  = normal(front ^ up);
    v3 n_up  = normal(side ^ front);

    cam->pos   = pos;
    cam->front = front;
    cam->side  = side;
    cam->up    = n_up;
}
//  (section) ----------------------------------------------------------------- : Camera control  //

internal v3 grid_to_world_pos(Grid grid, GridPos pos) {

    f32 size = grid.cube_size;

    f32 x_off = (size * (grid.cols - 1)) / 2;
    f32 y_off = (size * (grid.layers - 1)) / 2;
    f32 z_off = (size * (grid.rows - 1)) / 2;

    v3 res = {0};
    V3_veci(res,
            size * pos.x - x_off + grid.pos.x,
            size * pos.y - y_off + grid.pos.y,
            (-size * pos.z) + z_off + grid.pos.z);

    return res;
}

internal void draw_grid3d(render_push_buffer* push_buffer, Grid grid) {

    f32 size = grid.cube_size;

    f32 grey = 0.6f;

    for (u32 y = 0; y < grid.layers; y += 1) {
        for (u32 z = 0; z < grid.rows; z += 1) {
            for (u32 x = 0; x < grid.cols; x += 1) {

                RC_cube_wf cube = {0};

                cube.pos = grid_to_world_pos(grid, {(u8)x, (u8)y, (u8)z});
                V3_veci(cube.scale, size, size, size);
                V4_colori(cube.color, grey, grey, grey, 1.0f);

                R_PushCubeWF(push_buffer, cube);
            }
        }
    }
}

internal void draw_grid2d(render_push_buffer* push_buffer, Grid grid) {

    f32 size = grid.cube_size;


    for (u32 z = 0; z < grid.rows; z += 1) {
        for (u32 x = 0; x < grid.cols; x += 1) {

            RC_cube_wf cube = {0};

            cube.pos = grid_to_world_pos(grid, {(u8)x, 0, (u8)z});
            // cube.pos.y = 0.0f;
            V3_veci(cube.scale, size, size, size);
            V4_colori(cube.color, 1.0f, 1.0f, 1.0f, 0.0f);

            R_PushCubeWF(push_buffer, cube);
        }
    }
}
internal void draw_piece(canvas_state* state, render_push_buffer* push_buffer) {
    RC_cube piece = {0};

    if (state->p_anim.active) {
        f32 lerp_off = (state->p_anim.t / PIECE_MOVE_TIME);
        if (lerp_off >= 1.f) {
            state->p_anim.active = false;
            state->p_anim.t      = 0.f;
        }
        lerp_off = ClampTop(lerp_off, 1);

        lerp_off = interps[state->interp_type](lerp_off);

        piece.pos = lerp(grid_to_world_pos(state->grid, state->piece.pos),
                         state->p_anim.prev_position,
                         lerp_off);


        piece.scale = lerp({PIECE_SIZE, PIECE_SIZE, PIECE_SIZE},
                           {PIECE_SIZE - 10.f, PIECE_SIZE - 10.f, PIECE_SIZE - 10.f},
                           lerp_off);

    } else {

        piece.pos = grid_to_world_pos(state->grid, state->piece.pos);
        V3_veci(piece.scale, PIECE_SIZE, PIECE_SIZE, PIECE_SIZE);
    }

    R_PushCube(push_buffer, piece);
}

f32 triangle_wave(f32 time_ms, f32 period_ms) {
    f32 t     = time_ms / period_ms; // normalize time to cycles
    f32 phase = t - (int)t;          // fract(t)

    if (phase < 0.5f) {
        return phase * 2.0f; // rising 0 → 1
    } else {
        return 2.0f - phase * 2.0f; // falling 1 → 0
    }
}

internal void load_default_cam(render_camera* cam) {
    // default cam state
    cam->fov_radians = DegToRad(60);
    // V3_vecd(vp, -250.0f, 400.0f, 250.0f);
    V3_vecd(vp, 0, 200.0f, 250.0f);
    set_camera_to_look_at(cam, {0}, vp);
}

internal void reload_game_state(canvas_state* state) {
    memset(state, 0, sizeof(canvas_state));
    load_default_cam(&state->cam);
}

//  main game entry : ---------------------------------------------------------------- (section)  //
//
extern "C" CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRender) {

#ifdef DEBUG
    Assert(sizeof(canvas_state) <= memory->perma_size);
#endif

    //  unpacking stuff : ------------------------------------------------------------ (section)  //
    canvas_state* state = (canvas_state*)memory->perma_store;
    if (!memory->is_valid) {
        memory->is_valid = true;
    }

    canvas_keyboard_input*   key_b = &input->keyboard;
    canvas_controller_input* gmpds = input->gamepads;

    canvas_controller_input* ctrl1 = &input->gamepads[0];

    render_camera* cam = &state->cam;


    //  game state init : ------------------------------------------------------------ (section)  //
    state->grid.cube_size = 30.f;
    state->grid.rows      = 8;
    state->grid.cols      = 8;
    state->grid.layers    = 1;

    if (is_first_time) {
        load_default_cam(cam);
    }



    //  input handling : ------------------------------------------------------------- (section)  //
    f32 input_factor = 1.0f;
    state->x_off -= Held(ctrl1->Left) ? input_factor : 0.0f;
    state->x_off += Held(ctrl1->Right) ? input_factor : 0.0f;

    state->y_off += Held(ctrl1->Up) ? input_factor : 0.0f;
    state->y_off -= Held(ctrl1->Down) ? input_factor : 0.0f;


    if (Held(ctrl1->Stop)) {
        *running = false;
    }

    state->x_off -= Held(key_b->A) ? input_factor : 0.0f;
    state->x_off += Held(key_b->D) ? input_factor : 0.0f;

    state->y_off += Held(key_b->Q) ? input_factor : 0.0f;
    state->y_off -= Held(key_b->E) ? input_factor : 0.0f;

    state->z_off += Held(key_b->S) ? input_factor : 0.0f;
    state->z_off -= Held(key_b->W) ? input_factor : 0.0f;

    if Pushed (key_b->N) {
        state->interp_type = ((state->interp_type + 1) % EnumCount(Interpolation_Kind));
    }

    if Pushed (key_b->F2) {
        state->in_debug_mode = !(state->in_debug_mode);
    }

    if (!state->in_debug_mode) {
        if Pushed (key_b->D) {
            if (state->piece.pos.x < (state->grid.cols - 1)) {
                if (state->p_anim.active) {
                    state->p_anim.t = (f32)dt;
                } else {
                    state->p_anim.t += (f32)dt;
                }
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.x += 1;
            }
        }
        if Pushed (key_b->A) {
            if (state->piece.pos.x >= 1) {
                if (state->p_anim.active) {
                    state->p_anim.t = (f32)dt;
                } else {
                    state->p_anim.t += (f32)dt;
                }
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.x -= 1;
            }
        }
        if Pushed (key_b->W) {
            if (state->piece.pos.z < (state->grid.rows - 1)) {
                if (state->p_anim.active) {
                    state->p_anim.t = (f32)dt;
                } else {
                    state->p_anim.t += (f32)dt;
                }
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.z += 1;
            }
        }
        if Pushed (key_b->S) {
            if (state->piece.pos.z >= 1) {
                if (state->p_anim.active) {
                    state->p_anim.t = (f32)dt;
                } else {
                    state->p_anim.t += (f32)dt;
                }
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.z -= 1;
            }
        }
        if Pushed (key_b->Q) {
            if (state->piece.pos.y < (state->grid.layers - 1)) {
                if (state->p_anim.active) {
                    state->p_anim.t = (f32)dt;
                } else {
                    state->p_anim.t += (f32)dt;
                }
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.y += 1;
            }
        }
        if Pushed (key_b->E) {
            if (state->piece.pos.y >= 1) {
                if (state->p_anim.active) {
                    state->p_anim.t = (f32)dt;
                } else {
                    state->p_anim.t += (f32)dt;
                }
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.y -= 1;
            }
        }

        if (state->p_anim.active) {
            state->p_anim.t += (f32)dt;
        }
    }

    if (Held(key_b->Control) && Held(key_b->R)) {
        reload_game_state(state);
    }


    //  todo(debug camera) : --------------------------------------------------------- (section)
    //  //

    if (state->in_debug_mode) {

        // translation
        if Held (key_b->W) {
            cam->pos = cam->pos + (CAM_TRANSLATION_SPEED * (f32)dt) * cam->front;
        }
        if Held (key_b->S) {
            cam->pos = cam->pos - (CAM_TRANSLATION_SPEED * (f32)dt) * cam->front;
        }

        if Held (key_b->D) {
            cam->pos = cam->pos + (CAM_TRANSLATION_SPEED * (f32)dt) * cam->side;
        }
        if Held (key_b->A) {
            cam->pos = cam->pos - (CAM_TRANSLATION_SPEED * (f32)dt) * cam->side;
        }

        if Held (key_b->Q) {
            cam->pos = cam->pos + (CAM_TRANSLATION_SPEED * (f32)dt) * cam->up;
        }
        if Held (key_b->E) {
            cam->pos = cam->pos - (CAM_TRANSLATION_SPEED * (f32)dt) * cam->up;
        }


        // orientation
        if Held (key_b->I) {
            rot_cam_euler(cam, CAM_ORIENTATION_SPEED * (f32)dt, 0, 0);
        }
        if Held (key_b->K) {
            rot_cam_euler(cam, -CAM_ORIENTATION_SPEED * (f32)dt, 0, 0);
        }

        if Held (key_b->J) {
            rot_cam_euler(cam, 0, CAM_ORIENTATION_SPEED * (f32)dt, 0);
        }
        if Held (key_b->L) {
            rot_cam_euler(cam, 0, -CAM_ORIENTATION_SPEED * (f32)dt, 0);
        }

        if Held (key_b->U) {
            rot_cam_euler(cam, 0, 0, CAM_ORIENTATION_SPEED * (f32)dt);
        }
        if Held (key_b->O) {
            rot_cam_euler(cam, 0, 0, -CAM_ORIENTATION_SPEED * (f32)dt);
        }
    }

    //  ----------------------------------------------------------------------------------------
    //  //



    //  test updates and draws : ----------------------------------------------------- (section)
    //  //
    state->time_elapsed += (f32)dt * 0.0005f;
    state->interp_type = IK_OutSine;

    // Update clear color
    f32 red_shift      = ((f32)sin(state->time_elapsed) * 0.5f + 0.5f);
    f32 green_shift    = 0.0f;
    f32 blue_shift     = ((f32)cos(state->time_elapsed) * 0.5f + 0.5f);
    v4  changing_color = {red_shift, green_shift, blue_shift, 1.0f};

    // v4 color = {0.1f, 0.1f, 0.1f, 1.0f};
    v4 corn_blue = {0.494f, 0.620f, 0.969f, 1.0};

    // Draw clear
    RC_clear2d clear = {0};
    // clear.color      = corn_blue;
    // V4_colori(clear.color, 0.1f, .1f, .1f, 1.f);
    R_PushClear(push_buffer, clear);


    // f32 t = ((f32)cos(state->dt) * 0.5f + 0.5f);
    f32 t = triangle_wave(state->time_elapsed, 8.f);
    t     = interps[state->interp_type](t);

    draw_grid3d(push_buffer, state->grid);


    draw_piece(state, push_buffer);

    if (0) { // random bezier move
        V3_vecd(a, 150.f, 120.f, -400.f);
        V3_vecd(c, -180.f, -180.f, 400.f);

        V3_vecd(b, 484.f, 260.f, 0.f);


        f32 r = 200.0f;
        // V3_vecd(vantage_point, 25.f, -52.f, Lerp(t, -320.f, 320.f));
        set_camera_to_look_at(cam, {0}, quad_bezier(a, b, c, t));
    }

    //  Draw : ----------------------------------------------------------------------- (section)
    //  //

    // I am stupid to directly update the cam state in push_buffer
    // I should've always done that in game state and then at last
    // pushed it to the buffer, I am stupid
    push_buffer->cam = state->cam;
}
//  (section) ---------------------------------------------------------------- : main game entry
//  //
