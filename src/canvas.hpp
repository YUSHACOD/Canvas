#ifndef CANVAS_H
#define CANVAS_H
// The Game Interface --------------------------------------------------------------------------- //

#include "canvas_sugars.hpp"

typedef struct {
    void *Memory;
    int32 Width;
    int32 Height;
    int32 Size;
    int32 Pitch;
    int32 BytesPerPixel;
} CanvasBitMap;

typedef struct {
    int16 *SampleOut;
    int32 SamplesPerSecond;
    int32 SampleCount;
} CanvasSound;

typedef struct {
    int32 Transitions;
    bool EndedDown;
} CanvasButtonState;

typedef struct {
    real32 Min;
    real32 Max;
    real32 Start;
    real32 End;
} CanvasAnalogState;

typedef struct {
    CanvasAnalogState LeftStickX;
    CanvasAnalogState LeftStickY;

    CanvasAnalogState RightStickX;
    CanvasAnalogState RightStickY;

    CanvasAnalogState LeftTrigger;
    CanvasAnalogState RightTrigger;

    union {
        CanvasButtonState Buttons[14];

        struct {
            // XYAB 4
            CanvasButtonState X;
            CanvasButtonState Y;
            CanvasButtonState A;
            CanvasButtonState B;

            // Dpad 4
            CanvasButtonState Up;
            CanvasButtonState Down;
            CanvasButtonState Left;
            CanvasButtonState Right;

            // L/R Shoulders 2
            CanvasButtonState LS;
            CanvasButtonState RS;

            // L/R Thumb 2
            CanvasButtonState LT;
            CanvasButtonState RT;

            // Start/Stop 2
            CanvasButtonState Start;
            CanvasButtonState Stop;
        };
    };
} CanvasControllerInput;

typedef struct {
    CanvasControllerInput Controllers[4];
} CanvasInput;



internal void CanvasUpdateAndRender(CanvasBitMap *BitMap,
                                    CanvasSound *SoundBuffer,
                                    CanvasInput *Input,
                                    bool *Running);

// ---------------------------------------------------------------------------------------------- //
#endif
