
#include "base/include.cpp"

#include "canvas_platform.hpp"
#include "canvas_game.hpp"

#include "renderer.hpp"

#include <cmath>
#include <cstring>
#include <math.h>

// #include "canvas_utils.cpp"
#include "canvas_math.cpp"
#include "renderer.cpp"

inline void
camera_control(canvas_state* state, canvas_input* input, render_push_buffer* push_buffer) {

    Mat4_I(view_mat);
    V3_vecd(pos, -state->x_off, -state->y_off, -state->z_off);

    view_mat.vec4 = pos;

    push_buffer->view_mat = view_mat;
}

inline void set_camera_to_look_at(render_push_buffer* push_buffer, v3 at, v3 from) {

    V3_vecd(up, 0.0f, 1.0f, 0.0f);

    v3 z_a = normal(at - from);
    v3 x_a = normal(z_a ^ up);
    v3 y_a = normal(x_a ^ z_a);

    Mat4_I(view_mat);

    V3_veci(view_mat.vec1, x_a.x, y_a.x, -z_a.x);
    V3_veci(view_mat.vec2, x_a.y, y_a.y, -z_a.y);
    V3_veci(view_mat.vec3, x_a.z, y_a.z, -z_a.z);

    V3_veci(view_mat.vec4, -(x_a * from), -(y_a * from), (z_a * from));

    push_buffer->view_mat = view_mat;
}

internal v3 grid_to_world_pos(Grid grid, u8 x, u8 y, u8 z) {

    f32 size = grid.cube_size;

    f32 x_off = (size * (grid.cols - 1)) / 2;
    f32 y_off = (size * (grid.layers - 1)) / 2;
    f32 z_off = (size * (grid.rows - 1)) / 2;

    v3 res = {0};
    V3_veci(res,
            size * x - x_off + grid.pos.x,
            size * y - y_off + grid.pos.y,
            (-size * z) + z_off + grid.pos.z);

    return res;
}

internal void draw_grid3d(render_push_buffer* push_buffer, Grid grid) {

    f32 size = grid.cube_size;

    for (u32 y = 0; y < grid.layers; y += 1) {
        for (u32 z = 0; z < grid.rows; z += 1) {
            for (u32 x = 0; x < grid.cols; x += 1) {

                RC_cube_wf cube = {0};

                cube.pos = grid_to_world_pos(grid, (u8)x, (u8)y, (u8)z);
                V3_veci(cube.scale, size, size, size);
                V4_colori(cube.color, 0.5f, 0.5f, 0.5f, 1.0f);

                PushCubeWF(push_buffer, cube);
            }
        }
    }
}

internal void draw_grid2d(render_push_buffer* push_buffer, Grid grid) {

    f32 size = grid.cube_size;


    for (u32 z = 0; z < grid.rows; z += 1) {
        for (u32 x = 0; x < grid.cols; x += 1) {

            RC_cube_wf cube = {0};

            cube.pos = grid_to_world_pos(grid, (u8)x, 0, (u8)z);
            // cube.pos.y = 0.0f;
            V3_veci(cube.scale, size, size, size);
            V4_colori(cube.color, 1.0f, 1.0f, 1.0f, 0.0f);

            PushCubeWF(push_buffer, cube);
        }
    }
}

internal void draw_piece(canvas_state* state, render_push_buffer* push_buffer) {
    RC_cube piece = {0};
    piece.pos     = grid_to_world_pos(state->grid, state->piece.x, state->piece.y, state->piece.z);
    V3_veci(piece.scale, PIECE_SIZE, PIECE_SIZE, PIECE_SIZE);
    PushCube(push_buffer, piece);
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
    state->grid.cube_size = 20.f;
    state->grid.rows      = 2;
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

    state->p_anim.t1 += (f32)time_elapsed;
    if ((state->p_anim.t1 - state->p_anim.t) >= 0) {
        if (input->keyboard.D.ended_down) {
            state->p_anim.t = (f32)state->p_anim.t1 + PIECE_MOVE_TIME;
            if (state->piece.x < (state->grid.cols - 1)) {
                state->piece.x += 1;
            }
        }
        if (input->keyboard.A.ended_down) {
            state->p_anim.t = (f32)state->p_anim.t1 + PIECE_MOVE_TIME;
            if (state->piece.x >= 1) {
                state->piece.x -= 1;
            }
        }
        if (input->keyboard.W.ended_down) {
            state->p_anim.t = (f32)state->p_anim.t1 + PIECE_MOVE_TIME;
            if (state->piece.z < (state->grid.rows - 1)) {
                state->piece.z += 1;
            }
        }
        if (input->keyboard.S.ended_down) {
            state->p_anim.t = (f32)state->p_anim.t1 + PIECE_MOVE_TIME;
            if (state->piece.z >= 1) {
                state->piece.z -= 1;
            }
        }
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

    // Update clear color
    f32 red_shift      = ((f32)sin(state->dt) * 0.5f + 0.5f);
    f32 green_shift    = 0.0f;
    f32 blue_shift     = ((f32)cos(state->dt) * 0.5f + 0.5f);
    v4  changing_color = {red_shift, green_shift, blue_shift, 1.0f};

    // v4 color = {0.1f, 0.1f, 0.1f, 1.0f};
    v4 corn_blue = {0.494f, 0.620f, 0.969f, 1.0};

    // Draw clear
    RC_clear2d clear = {0};
    clear.color      = corn_blue;
    PushClear(push_buffer, clear);


    f32 t = ((f32)cos(state->dt) * 0.5f + 0.5f);

    V3_vecd(pos, 0.0f, 0.0f, 0.0f);
    draw_grid2d(push_buffer, state->grid);

    V3_vecd(vp, 0.0f, 70.0f, 150.0f);
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
