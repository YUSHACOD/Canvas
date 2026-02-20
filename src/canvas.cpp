
#include "base/sugars.hpp"
#include "canvas.hpp"

#include <math.h>

// #include "canvas_utils.cpp"

extern "C" CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRender) {

#ifdef DEBUG
    Assert(sizeof(canvas_state) <= memory->perma_size);
#endif

    canvas_state* state = (canvas_state*)memory->perma_store;
    if (!memory->is_valid) {
        memory->is_valid = true;
    }

    // Input Handling
    canvas_controller_input* input1 = &input->gamepads[0];

    state->jx = (200.0f * input1->LeftStickX.end);
    state->jy = (200.0f * input1->LeftStickY.end);

    f32 input_factor = 2.0f;
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

    state->y_off += (input->keyboard.W.ended_down) ? input_factor : 0.0f;
    state->y_off -= (input->keyboard.S.ended_down) ? input_factor : 0.0f;

    if (input->keyboard.Control.ended_down && input->keyboard.R.ended_down) {
        ZeroMem(state, sizeof(canvas_state));
    }

    f64 dt = time_elapsed * 0.001f;

    // Clearing the opengl buffer
    f32 red_shift   = ((f32)sin(dt) * 0.5f + 0.5f);
    f32 green_shift = ((f32)cos(dt) * 0.5f + 0.5f);
    f32 blue_shift  = 0.0f;
    v4  color       = {red_shift, green_shift, blue_shift, 1.0f};
    // v4 color = {0.1f, 0.1f, 0.1f, 1.0f};
    memory->GLClear(memory->gl_state, color);

    // Drawing a cube
    // v4  color       = {0.1f, 0.1f, 0.1f, 1.0f};
    v4 cube_color = {1.0f, 0.0f, 0.0f, 1.0f};
    memory->GLDrawCube(memory->gl_state, dt, {0}, cube_color);
}
