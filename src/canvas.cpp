
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

    f64 dt = time_elapsed * 0.0005f;

    // Clearing the opengl buffer
    f32 red_shift   = ((f32)sin(dt) * 0.5f + 0.5f);
    f32 green_shift = ((f32)cos(dt) * 0.5f + 0.5f);
    f32 blue_shift  = 0.0f;
    v4  color       = {red_shift, green_shift, blue_shift, 1.0f};
    // v4 color = {0.1f, 0.1f, 0.1f, 1.0f};
    v4 corn_blue = {0.494f, 0.620f, 0.969f, 1.0};
    memory->GLClear(memory->gl_state, corn_blue);

    // Drawing a cube
    // v4  color       = {0.1f, 0.1f, 0.1f, 1.0f};
    v4  cube_color  = {blue_shift, red_shift, green_shift, 1.0f};
    f32 lerp_offset = ((f32)sin(dt) * 0.5f + 0.5f);
    memory->GLDrawCube(memory->gl_state, lerp_offset, {0}, cube_color);

    for (f32 t = 0.0f; t < 1.1f; t += 0.1f) {
        memory->GLDrawCubeWireframe(memory->gl_state, t, {0}, {0.5f, 0.5f, 0.5f, 1.0f});
    }
}
