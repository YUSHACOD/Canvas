#ifndef CANVAS_H
#define CANVAS_H
// The Game Interface --------------------------------------------------------------------------- //

#include "canvas_sugars.hpp"

#ifdef DEBUG
typedef struct {
    void  *Memory;
    uint64 Size;
} DEBUGFileStruct;
DEBUGFileStruct DEBUGPlatformReadEntireFile(char *FileName);
void            DEBUGPlatformFreeFileMemory(void *Memory);
bool            DEBUGPlatformWriteEntireFile(char *FileName, void *Memory, uint32 MemorySize);
#endif

typedef struct {
    void  *Memory;
    uint32 Width;
    uint32 Height;
    uint32 Size;
    uint32 Pitch;
    uint32 BytesPerPixel;
} CanvasBitMap;

typedef struct {
    uint32 Transitions;
    bool   EndedDown;
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

typedef struct {
    bool IsValid;

    uint64 PermaSize;
    void  *PermaStore;

    uint64 TransSize;
    void  *TransStore;
} CanvasMemmory;



internal void CanvasUpdateAndRender(CanvasMemmory *Memmory,
                                    CanvasBitMap  *BitMap,
                                    CanvasInput   *Input,
                                    bool          *Running);



typedef struct {
    uint32 XOff;
    uint32 YOff;
} CanvasState;

// ---------------------------------------------------------------------------------------------- //
#endif
