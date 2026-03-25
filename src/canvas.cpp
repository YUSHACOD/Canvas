
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

inline void
camera_control(canvas_state* state, canvas_input* input, render_push_buffer* push_buffer) {

    Mat4_di(view_mat);
    V3_vecd(pos, -state->x_off, -state->y_off, -state->z_off);

    view_mat.vec4 = pos;

    push_buffer->view_mat = view_mat;
}

inline void set_camera_to_look_at(render_push_buffer* push_buffer, v3 at, v3 pos) {

    V3_vecd(up, 0.0f, 1.0f, 0.0f);

    v3 front = normal(at - pos);
    v3 side  = normal(front ^ up);
    v3 n_up  = normal(side ^ front);

    push_buffer->cam.pos   = pos;
    push_buffer->cam.front = front;
    push_buffer->cam.side  = side;
    push_buffer->cam.up    = n_up;
}

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

//  main game entry : ---------------------------------------------------------------- (section)  //
//
extern "C" CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRender) {

#ifdef DEBUG
    Assert(sizeof(canvas_state) <= memory->perma_size);
#endif

    canvas_state* state = (canvas_state*)memory->perma_store;
    if (!memory->is_valid) {
        memory->is_valid = true;
    }

    //  game state init : ------------------------------------------------------------ (section)  //
    state->grid.cube_size = 30.f;
    state->grid.rows      = 8;
    state->grid.cols      = 8;
    state->grid.layers    = 1;


    //  input handling : ------------------------------------------------------------- (section)  //
    canvas_controller_input* input1 = &input->gamepads[0];



    f32 input_factor = 1.0f;
    state->x_off -= (input1->Left.ended_down) ? input_factor : 0.0f;
    state->x_off += (input1->Right.ended_down) ? input_factor : 0.0f;

    state->y_off += (input1->Up.ended_down) ? input_factor : 0.0f;
    state->y_off -= (input1->Down.ended_down) ? input_factor : 0.0f;


    if (input1->Stop.ended_down) {
        *running = false;
    }

    state->x_off -= (input->keyboard.A.ended_down) ? input_factor : 0.0f;
    state->x_off += (input->keyboard.D.ended_down) ? input_factor : 0.0f;

    state->y_off += (input->keyboard.Q.ended_down) ? input_factor : 0.0f;
    state->y_off -= (input->keyboard.E.ended_down) ? input_factor : 0.0f;

    state->z_off += (input->keyboard.S.ended_down) ? input_factor : 0.0f;
    state->z_off -= (input->keyboard.W.ended_down) ? input_factor : 0.0f;

    state->interp_type = (input->keyboard.I.ended_down)
                             ? ((state->interp_type + 1) % EnumCount(Interpolation_Kind))
                             : (state->interp_type);

    if (!state->p_anim.active) {
        if (input->keyboard.D.ended_down) {
            if (state->piece.pos.x < (state->grid.cols - 1)) {
                state->p_anim.t += (f32)time_elapsed;
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.x += 1;
            }
        }
        if (input->keyboard.A.ended_down) {
            if (state->piece.pos.x >= 1) {
                state->p_anim.t += (f32)time_elapsed;
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.x -= 1;
            }
        }
        if (input->keyboard.W.ended_down) {
            if (state->piece.pos.z < (state->grid.rows - 1)) {
                state->p_anim.t += (f32)time_elapsed;
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.z += 1;
            }
        }
        if (input->keyboard.S.ended_down) {
            if (state->piece.pos.z >= 1) {
                state->p_anim.t += (f32)time_elapsed;
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.z -= 1;
            }
        }
        if (input->keyboard.Q.ended_down) {
            if (state->piece.pos.y < (state->grid.layers - 1)) {
                state->p_anim.t += (f32)time_elapsed;
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.y += 1;
            }
        }
        if (input->keyboard.E.ended_down) {
            if (state->piece.pos.y >= 1) {
                state->p_anim.t += (f32)time_elapsed;
                state->p_anim.active        = true;
                state->p_anim.prev_position = grid_to_world_pos(state->grid, state->piece.pos);
                state->piece.pos.y -= 1;
            }
        }
    } else {
        state->p_anim.t += (f32)time_elapsed;
    }

    if (input->keyboard.Control.ended_down && input->keyboard.R.ended_down) {
        memset(state, 0, sizeof(canvas_state));
    }


    //  todo(debug camera) : --------------------------------------------------------- (section)  //
    // state->debug_camera_mode =
    //     (input->keyboard.Control.ended_down && input->keyboard.Num4.ended_down)

    //         ? !(state->debug_camera_mode)
    //         : (state->debug_camera_mode);

    // if (state->debug_camera_mode) {
    // camera_control(state, input, push_buffer);
    // }
    //  ----------------------------------------------------------------------------------------  //



    //  test updates and draws : ----------------------------------------------------- (section)  //
    state->dt += (f32)time_elapsed * 0.0005f;
    state->interp_type = IK_OutSine;

    // Update clear color
    f32 red_shift      = ((f32)sin(state->dt) * 0.5f + 0.5f);
    f32 green_shift    = 0.0f;
    f32 blue_shift     = ((f32)cos(state->dt) * 0.5f + 0.5f);
    v4  changing_color = {red_shift, green_shift, blue_shift, 1.0f};

    // v4 color = {0.1f, 0.1f, 0.1f, 1.0f};
    v4 corn_blue = {0.494f, 0.620f, 0.969f, 1.0};

    // Draw clear
    RC_clear2d clear = {0};
    // clear.color      = corn_blue;
    // V4_colori(clear.color, 0.1f, .1f, .1f, 1.f);
    R_PushClear(push_buffer, clear);


    // f32 t = ((f32)cos(state->dt) * 0.5f + 0.5f);
    f32 t = triangle_wave(state->dt, 8.f);
    t     = interps[state->interp_type](t);

    V3_vecd(pos, 0.0f, 0.0f, 0.0f);
    draw_grid3d(push_buffer, state->grid);

    // V3_vecd(vp, -250.0f, 400.0f, 250.0f);
    V3_vecd(vp, 0, 200.0f, 250.0f);
    set_camera_to_look_at(push_buffer, pos, vp);

    draw_piece(state, push_buffer);

    if (0) { // random bezier move
        V3_vecd(a, 150.f, 120.f, -400.f);
        V3_vecd(c, -180.f, -180.f, 400.f);

        V3_vecd(b, 484.f, 260.f, 0.f);


        f32 r = 200.0f;
        // V3_vecd(vantage_point, 25.f, -52.f, Lerp(t, -320.f, 320.f));
        set_camera_to_look_at(push_buffer, pos, quad_bezier(a, b, c, t));
    }
}
//  (section) ---------------------------------------------------------------- : main game entry  //
