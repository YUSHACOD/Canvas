
#include "base/include.cpp"

#include "canvas_platform.hpp"
#include "canvas_game.hpp"

#include "renderer.hpp"

#include <cmath>
#include <cstring>
#include <math.h>

// #include "canvas_utils.cpp"
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

    NoOp;
}


internal void
draw_grid(render_push_buffer* push_buffer, v3 pos, u32 rows, u32 cols, f32 pad, f32 size) {

    f32 x_off = ((size + pad) * (cols - 1)) / 2;
    f32 y_off = ((size + pad) * (rows - 1)) / 2;
    f32 z_off = ((size + pad) * (rows - 1)) / 2;

    for (u32 y = 0; y < rows; y += 1) {
        for (u32 x = 0; x < cols; x += 1) {
            for (u32 z = 0; z < cols; z += 1) {


                RC_cube cube = {0};
                V3_veci(cube.pos,
                        (pad + size) * x - x_off,
                        (pad + size) * y - y_off,
                        (pad + size) * z - z_off);
                cube.rotation = euler_to_quat(DegToRad(0), DegToRad(145), DegToRad(45));

                V3_veci(cube.scale, size, size, size);
                // V4_colori(cube.color, 1.0f, 1.0f, 0.0f, 1.0f);
                PushCube(push_buffer, cube);
            }
        }
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

    //  input handling : ------------------------------------------------------------- (section)  //
    canvas_controller_input* input1 = &input->gamepads[0];

    state->jx = (200.0f * input1->LeftStickX.end);
    state->jy = (200.0f * input1->LeftStickY.end);

    f32 input_factor = 1.0f;
    state->x_off -= (input1->Left.ended_down) ? input_factor : 0.0f;
    state->x_off += (input1->Right.ended_down) ? input_factor : 0.0f;

    state->y_off += (input1->Up.ended_down) ? input_factor : 0.0f;
    state->y_off -= (input1->Down.ended_down) ? input_factor : 0.0f;

    state->weight += (5.0f * input1->RightTrigger.end);
    state->weight -= (5.0f * input1->LeftTrigger.end);

    if (input1->Stop.ended_down) {
        *running = false;
    }

    state->x_off -= (input->keyboard.A.ended_down) ? input_factor : 0.0f;
    state->x_off += (input->keyboard.D.ended_down) ? input_factor : 0.0f;

    state->y_off += (input->keyboard.Q.ended_down) ? input_factor : 0.0f;
    state->y_off -= (input->keyboard.E.ended_down) ? input_factor : 0.0f;

    state->z_off += (input->keyboard.S.ended_down) ? input_factor : 0.0f;
    state->z_off -= (input->keyboard.W.ended_down) ? input_factor : 0.0f;

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
    f64 dt = time_elapsed * 0.001f;

    // Update clear color
    f32 red_shift      = ((f32)sin(dt) * 0.5f + 0.5f);
    f32 green_shift    = 0.0f;
    f32 blue_shift     = ((f32)cos(dt) * 0.5f + 0.5f);
    v4  changing_color = {red_shift, green_shift, blue_shift, 1.0f};

    // v4 color = {0.1f, 0.1f, 0.1f, 1.0f};
    v4 corn_blue = {0.494f, 0.620f, 0.969f, 1.0};

    // Draw clear
    RC_clear2d clear = {};
    clear.color      = changing_color;
    PushClear(push_buffer, clear);

    // Update cube
    v4 cube_color = {1.0, 1.0, 1.0f, 1.0f};

    // Draw cube
    RC_cube cube = {};
    V3_veci(cube.pos, 0, 0, -50.0f);
    V3_veci(cube.scale, 20.0f, 20.0f, 20.0f);
    // cube.rotation =
    //     euler_to_quat(DegToRad(state->y_off), DegToRad(state->x_off), DegToRad(state->z_off));
    cube.color = corn_blue;
    // PushCube(push_buffer, cube);


    // Update cube_wf's
    if (0) {
        RC_cube_wf cube_w = {};
        V3_veci(cube_w.pos, 0.0f, 0.0f, -50.0f);
        V3_veci(cube_w.scale, 20.0f, 20.0f, 20.0f);
        V4_colori(cube_w.color, 1.0f, 1.0f, 1.0f, 1.0f);
        PushCubeWF(push_buffer, cube_w);

        RC_cube_wf cube0 = {};
        V3_veci(cube0.pos, 0.0f, 0.0f, -150.0f);
        V3_veci(cube0.scale, 20.0f, 20.0f, 20.0f);
        V4_colori(cube0.color, 0.0f, 0.0f, 0.0f, 1.0f);
        PushCubeWF(push_buffer, cube0);

        RC_cube_wf cube1 = {};
        V3_veci(cube1.pos, -50.0f, 0.0f, -100.0f);
        V3_veci(cube1.scale, 20.0f, 20.0f, 20.0f);
        V4_colori(cube1.color, 1.0f, 0.0f, 0.0f, 1.0f);
        PushCubeWF(push_buffer, cube1);

        RC_cube_wf cube2 = {};
        V3_veci(cube2.pos, 50.0f, 0.0f, -100.0f);
        V3_veci(cube2.scale, 20.0f, 20.0f, 20.0f);
        V4_colori(cube2.color, 0.0f, 0.0f, 1.0f, 1.0f);
        PushCubeWF(push_buffer, cube2);

        RC_cube_wf cube3 = {};
        V3_veci(cube3.pos, 0.0f, -50.0f, -100.0f);
        V3_veci(cube3.scale, 20.0f, 20.0f, 20.0f);
        V4_colori(cube3.color, 1.0f, 1.0f, 0.0f, 1.0f);
        PushCubeWF(push_buffer, cube3);

        RC_cube_wf cube4 = {};
        cube4.pos        = {0.0f, 50.0f, -100.0f};
        cube4.scale      = {20.0f, 20.0f, 20.0f};
        cube4.color      = {1.0f, 0.0f, 1.0f, 1.0f};
        PushCubeWF(push_buffer, cube4);
    }


    V3_vecd(pos, 0.0f, 0.0f, -50.0f);
    // f32 t = ((f32)cos(dt) * 0.5f + 0.5f);
    // draw_grid(push_buffer, pos, 5, 5, Lerp(t, 15.0f, 75.0f), 20.0f);
    draw_grid(push_buffer, pos, 5, 5, 30.0f, 20.0f);

    f32 r = 200.0f;
    V3_veci(pos, 0.f, 0.f, 0.f);
    V3_vecd(vantage_point,
            ((f32)cos(dt) * r) + cube.pos.x,
            0.0f + cube.pos.y,
            ((f32)sin(dt) * r) + cube.pos.z);
    set_camera_to_look_at(push_buffer, pos, vantage_point);
}
//  (section) ---------------------------------------------------------------- : main game entry  //
