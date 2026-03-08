
#include "base/sugars.hpp"

#include "canvas_platform.hpp"
#include "canvas_game.hpp"

#include "renderer.hpp"

#include <cstring>
#include <math.h>

// #include "canvas_utils.cpp"
#include "renderer.cpp"

inline void
camera_control(canvas_state* state, canvas_input* input, render_push_buffer* push_buffer) {
    push_buffer->camera.pos.x = state->x_off;
    push_buffer->camera.pos.y = state->y_off;
    push_buffer->camera.pos.z = state->z_off;
}


// Main Entry Point for the game code ---------------------------------------------------------- //
extern "C" CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRender) {

#ifdef DEBUG
    Assert(sizeof(canvas_state) <= memory->perma_size);
#endif

    canvas_state* state = (canvas_state*)memory->perma_store;
    if (!memory->is_valid) {
        memory->is_valid = true;
    }

    // Input Handling -------------------------------------------------
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
	
	camera_control(state, input, push_buffer);
    // Input Handling -------------------------------------------------



    // Test Updates and Draws ----------------------------------------------------------
    f64 dt = time_elapsed * 0.001f;

    // Update clear color
    f32 red_shift      = ((f32)sin(dt) * 0.5f + 0.5f);
    f32 green_shift    = ((f32)cos(dt) * 0.5f + 0.5f);
    f32 blue_shift     = 0.0f;
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
    cube.pos     = {0.0f, 0.0f, -100.0f};
    cube.scale   = {20.0f, 20.0f, 20.0f};
    cube.color   = corn_blue;
    PushCube(push_buffer, cube);

    // Update cube_wf's
    RC_cube_wf cube_w= {};
    cube_w.pos        = {0.0f, 0.0f, -50.0f};
    cube_w.scale      = {20.0f, 20.0f, 20.0f};
    cube_w.color      = {1.0f, 1.0f, 1.0f, 1.0f};
    PushCubeWF(push_buffer, cube_w);

    RC_cube_wf cube0= {};
    cube0.pos        = {0.0f, 0.0f, -150.0f};
    cube0.scale      = {20.0f, 20.0f, 20.0f};
    cube0.color      = {0.0f, 0.0f, 0.0f, 1.0f};
    PushCubeWF(push_buffer, cube0);

    RC_cube_wf cube1 = {};
    cube1.pos        = {-50.0f, 0.0f, -100.0f};
    cube1.scale      = {20.0f, 20.0f, 20.0f};
    cube1.color      = {1.0f, 0.0f, 0.0f, 1.0f};
    PushCubeWF(push_buffer, cube1);

    RC_cube_wf cube2= {};
    cube2.pos        = {50.0f, 0.0f, -100.0f};
    cube2.scale      = {20.0f, 20.0f, 20.0f};
    cube2.color      = {0.0f, 0.0f, 1.0f, 1.0f};
    PushCubeWF(push_buffer, cube2);

    RC_cube_wf cube3= {};
    cube3.pos        = {0.0f, -50.0f, -100.0f};
    cube3.scale      = {20.0f, 20.0f, 20.0f};
    cube3.color      = {1.0f, 1.0f, 0.0f, 1.0f};
    PushCubeWF(push_buffer, cube3);

    RC_cube_wf cube4= {};
    cube4.pos        = {0.0f, 50.0f, -100.0f};
    cube4.scale      = {20.0f, 20.0f, 20.0f};
    cube4.color      = {1.0f, 0.0f, 1.0f, 1.0f};
    PushCubeWF(push_buffer, cube4);
}
// Main Entry Point for the game code ---------------------------------------------------------- //
