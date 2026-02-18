
#include "canvas.hpp"
#include "base/sugars.hpp"

#include "canvas_utils.cpp"
#include "canvas3D.cpp"

void CanvasWeirdRender(CanvasBitMap* bitmap, u32 x_off, u32 y_off) {

    u8* row = (u8*)bitmap->memory;

    for (u32 y = 0; y < bitmap->height; y += 1) {
        u32* pixel = (u32*)row;
        for (u32 x = 0; x < bitmap->width; x += 1) {

            // Blue
            // uint8 blue  = (uint8)(X + x_off);
            // uint8 green = (uint8)(Y + y_off);
            u8 blue  = 34;
            u8 green = 34;
            u8 red   = 34;
            u8 pad   = 0;

            DrawPixel(pixel, red, blue, green, pad);

            pixel += 1;
        }

        row += bitmap->pitch;
    }
}

void JOY(CanvasBitMap* bitmap, CanvasState* state) {

    // DrawSquareC(bitmap, state->x_off, state->y_off, c, 30);

    // DrawSquareC(bitmap, 0, 0, c, 20);
    // DrawSquareC(bitmap, -960, -540, c, 20);
    // DrawSquareC(bitmap, -960, 0, c, 20);
    // DrawSquareC(bitmap, 959, 0, c, 20);
    // DrawSquareC(bitmap, 0, -540, c, 20);
    // DrawSquareC(bitmap, 0, 539, c, 20);
    // DrawSquareC(bitmap, -960, 539, c, 20);
    // DrawSquareC(bitmap, 959, -540, c, 20);
    // DrawSquareC(bitmap, 959, 539, c, 20);


    CaseyCircleFill(bitmap, state->x_off, state->y_off, state->weight + 10, SILVER);
    BLine(bitmap, 0, 0, state->x_off, state->y_off, SILVER, 5);

    CaseyCircleFill(bitmap, state->x_off + state->jx, state->y_off + state->jy, state->weight, GOLD);
}


extern "C" CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRender) {

#ifdef DEBUG
    Assert(sizeof(CanvasState) <= memory->perma_size);
#endif

    CanvasState* state = (CanvasState*)memory->perma_store;

    if (!memory->is_valid) {

#if 0
        char*          FileName = Text(__FILE__);
        DBG_FileStruct File     = memory->DBG_PlatReadEntireFile(FileName);
        if (File.memory) {
            memory->DBG_PlatWriteEntireFile(Text("./rama.txt"), File.memory, (uint32)File.Size);
            memory->DBG_PlatFreeFilememory(File.memory);
        }
#endif

        memory->is_valid = true;
    }

    // input Handling ------------------------------------------ //
    CanvasControllerInput* input1 = &input->gamepads[0];

    state->jx = (i32)(200.0f * input1->LeftStickX.end);
    state->jy = (i32)(200.0f * input1->LeftStickY.end);

    state->x_off -= (input1->Left.ended_down) ? 2 : 0;
    state->x_off += (input1->Right.ended_down) ? 2 : 0;

    state->y_off += (input1->Up.ended_down) ? 2 : 0;
    state->y_off -= (input1->Down.ended_down) ? 2 : 0;

    state->weight += (u32)(5.0f * input1->RightTrigger.end);
    state->weight -= (u32)(5.0f * input1->LeftTrigger.end);

    if (input1->Stop.ended_down) {
        *running = false;
    }

    state->x_off -= (input->keyboard.A.ended_down) ? 10 : 0;
    state->x_off += (input->keyboard.D.ended_down) ? 10 : 0;

    state->y_off += (input->keyboard.W.ended_down) ? 10 : 0;
    state->y_off -= (input->keyboard.S.ended_down) ? 10 : 0;

    if (input->keyboard.Alt.ended_down && input->keyboard.B.ended_down) {
        state->weight += 5;
    }

    if (input->keyboard.Control.ended_down && input->keyboard.R.ended_down) {
        ZeroMem(state, sizeof(CanvasState));
    }



    // Todo: Allow sample offsets here for more robust platform options.
    CanvasWeirdRender(bitmap, state->x_off, state->y_off);
    // JOY(BitMap, State);
    Render3DScene(bitmap, input, state);
}
