
#include "canvas.hpp"
#include "sugars.hpp"

#include "canvas_utils.cpp"
#include "canvas3D.cpp"

void CanvasWeirdRender(CanvasBitMap* BitMap, u32 XOff, u32 YOff) {

    u8* Row = (u8*)BitMap->Memory;

    for (u32 Y = 0; Y < BitMap->Height; Y += 1) {
        u32* Pixel = (u32*)Row;
        for (u32 X = 0; X < BitMap->Width; X += 1) {

            // Blue
            // uint8 blue  = (uint8)(X + XOff);
            // uint8 green = (uint8)(Y + YOff);
            u8 blue  = 255;
            u8 green = 255;
            u8 red   = 255;
            u8 pad   = 0;

            DrawPixel(Pixel, red, blue, green, pad);

            Pixel += 1;
        }

        Row += BitMap->Pitch;
    }
}

void JOY(CanvasBitMap* bitmap, CanvasState* state) {

    // DrawSquareC(bitmap, state->XOff, state->YOff, c, 30);

    // DrawSquareC(bitmap, 0, 0, c, 20);
    // DrawSquareC(bitmap, -960, -540, c, 20);
    // DrawSquareC(bitmap, -960, 0, c, 20);
    // DrawSquareC(bitmap, 959, 0, c, 20);
    // DrawSquareC(bitmap, 0, -540, c, 20);
    // DrawSquareC(bitmap, 0, 539, c, 20);
    // DrawSquareC(bitmap, -960, 539, c, 20);
    // DrawSquareC(bitmap, 959, -540, c, 20);
    // DrawSquareC(bitmap, 959, 539, c, 20);


    CaseyCircleFill(bitmap, state->XOff, state->YOff, state->Weight + 10, SILVER);
    BLine(bitmap, 0, 0, state->XOff, state->YOff, SILVER, 5);

    CaseyCircleFill(bitmap, state->XOff + state->JX, state->YOff + state->JY, state->Weight, GOLD);
}


extern "C" CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRender) {

#ifdef DEBUG
    Assert(sizeof(CanvasState) <= Memory->PermaSize);
#endif

    CanvasState* State = (CanvasState*)Memory->PermaStore;

    if (!Memory->IsValid) {

#if 0
        char*          FileName = Text(__FILE__);
        DBG_FileStruct File     = Memory->DBG_PlatReadEntireFile(FileName);
        if (File.Memory) {
            Memory->DBG_PlatWriteEntireFile(Text("./rama.txt"), File.Memory, (uint32)File.Size);
            Memory->DBG_PlatFreeFileMemory(File.Memory);
        }
#endif

        Memory->IsValid = true;
    }

    // Input Handling ------------------------------------------ //
    CanvasControllerInput* Input1 = &Input->Controllers[0];

    State->JX = (i32)(200.0f * Input1->LeftStickX.End);
    State->JY = (i32)(200.0f * Input1->LeftStickY.End);

    State->XOff -= (Input1->Left.EndedDown) ? 2 : 0;
    State->XOff += (Input1->Right.EndedDown) ? 2 : 0;

    State->YOff += (Input1->Up.EndedDown) ? 2 : 0;
    State->YOff -= (Input1->Down.EndedDown) ? 2 : 0;

    State->Weight += (u32)(5.0f * Input1->RightTrigger.End);
    State->Weight -= (u32)(5.0f * Input1->LeftTrigger.End);

    if (Input1->Stop.EndedDown) {
        *Running = false;
    }

    State->XOff -= (Input->Keyboard.A.EndedDown) ? 10 : 0;
    State->XOff += (Input->Keyboard.D.EndedDown) ? 10 : 0;

    State->YOff += (Input->Keyboard.W.EndedDown) ? 10 : 0;
    State->YOff -= (Input->Keyboard.S.EndedDown) ? 10 : 0;

    if (Input->Keyboard.Alt.EndedDown && Input->Keyboard.B.EndedDown) {
        State->Weight += 5;
    }

    if (Input->Keyboard.Control.EndedDown && Input->Keyboard.R.EndedDown) {
        ZeroMemory(State, sizeof(CanvasState));
    }



    // Todo: Allow sample offsets here for more robust platform options.
    // CanvasWeirdRender(BitMap, State->XOff, State->YOff);
    // JOY(BitMap, State);
    Render3DScene(BitMap, Input, State);
}
