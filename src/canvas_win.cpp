/*
 *  TODO(Ayush): Seperating the Platform layer
 *   - Save Location
 *   - Getting the Handle to our own executable???
 *   - Asset Loading Path
 *   - Threading
 *   - Raw Input
 *   - Sleep / TimeBeginPeriod
 *   - FullScreen support
 *   - WM_SETCURSOR
 *   - QueryCancelAutoPlay
 *   - WM_ACTIVATEAPP
 *   - Blt speed improvements
 *   - Hardware Accelearation (OpenGl or DirectX)
 *
 *
 *   Just a PARTIAL LIST!!!!
 */

#include "canvas.hpp"
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include <stdio.h>
#include <windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>

#include "canvas_sugars.hpp"

#include "canvas.cpp"

typedef struct {
    BITMAPINFO Info;
    void *Memory;
    int32 Width;
    int32 Height;
    int32 Size;
    int32 Pitch;
    int32 BytesPerPixel;
} WinCanvasBitMap;

typedef struct {
    int32 Width;
    int32 Height;
} WinCanvasDimensions;

typedef struct {
    int32 Channels;
    int32 SamplesPerSec;
    int32 ToneHz;
    int32 WavePeriod;
    int32 BytesPerSample;
    int32 BufferSize;
    uint32 RunningSampleIndex;
    int32 ToneVolume;
    int32 LatencySampleCount;
    real32 tSine;
} WinCanvasSound;


// Globals --------------------------------------------------//
global bool GlobalRunning;
global WinCanvasBitMap GlobalWinCanvas;
global LPDIRECTSOUNDBUFFER GlobalSoundBuffer;
// Globals --------------------------------------------------//


// Think about this, learn this --------------------------------------------- //

#define XINPUT_GET(name) DWORD WINAPI name(DWORD UserIndex, XINPUT_STATE *State)
typedef XINPUT_GET(xinput_get_state);
XINPUT_GET(xInputGetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
global xinput_get_state *XInputGetState_ = xInputGetStateStub;
#define XInputGetState XInputGetState_

#define XINPUT_SET(name) DWORD WINAPI name(DWORD UserIndex, XINPUT_VIBRATION *State)
typedef XINPUT_SET(xinput_set_state);
XINPUT_SET(xInputSetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
global xinput_set_state *XInputSetState_ = xInputSetStateStub;
#define XInputSetState XInputSetState_

internal void
WinCanvasLoadXInput() {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");

    if (XInputLibrary) {
        XInputGetState = (xinput_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (xinput_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    } else {
        // logging
    }
}
// -------------------------------------------------------------------------- //

// Same shenanigans as above for DirectSound -------------------------------- //
#define DSOUND_CREATE(name)                                                                        \
    HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DSOUND_CREATE(dsound_create);

internal void
WinCanvasInitDSound(HWND WindowHandle, int32 BufferSize, int32 SamplesPerSec) {
    // Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if (DSoundLibrary) {

        // Create the Direct Sound object
        dsound_create *DirectSoundCreate =
            (dsound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate) {
            if (SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {

                WAVEFORMATEX BufferFormat = {};

                BufferFormat.wFormatTag = WAVE_FORMAT_PCM;
                BufferFormat.nChannels = 2;
                BufferFormat.nSamplesPerSec = SamplesPerSec;
                BufferFormat.wBitsPerSample = 16;
                BufferFormat.nBlockAlign =
                    (BufferFormat.nChannels * BufferFormat.wBitsPerSample) / 8;
                BufferFormat.nAvgBytesPerSec =
                    BufferFormat.nSamplesPerSec * BufferFormat.nBlockAlign;

                BufferFormat.cbSize = 0;
                if (SUCCEEDED(DirectSound->SetCooperativeLevel(WindowHandle, DSSCL_PRIORITY))) {
                    // "Create" a primary buffer
                    DSBUFFERDESC DSBufferDesc = {};

                    DSBufferDesc.dwSize = sizeof(DSBufferDesc);
                    DSBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

                    LPDIRECTSOUNDBUFFER DSPrimaryBuffer;
                    if (SUCCEEDED(
                            DirectSound->CreateSoundBuffer(&DSBufferDesc, &DSPrimaryBuffer, 0))) {


                        if (SUCCEEDED(DSPrimaryBuffer->SetFormat(&BufferFormat))) {
                        }
                    } else {
                        // logging
                    }

                } else {
                    // logging
                }


                // "Create" a secondary buffer
                DSBUFFERDESC DSBufferDesc = {};

                DSBufferDesc.dwSize = sizeof(DSBufferDesc);
                DSBufferDesc.dwBufferBytes = BufferSize;
                DSBufferDesc.lpwfxFormat = &BufferFormat;

                if (SUCCEEDED(
                        DirectSound->CreateSoundBuffer(&DSBufferDesc, &GlobalSoundBuffer, 0))) {

                    if (SUCCEEDED(GlobalSoundBuffer->SetFormat(&BufferFormat))) {
                    }
                } else {
                    // logging
                }
            } else {
                // logging
            }
        } else {
            // logging
        }

    } else {
        // logging
    }
}
// -------------------------------------------------------------------------- //

internal WinCanvasSound
WinCanvasInitSound() {
    WinCanvasSound Sound = {};

    Sound.Channels = 2;
    Sound.SamplesPerSec = 48000;
    Sound.ToneHz = 256;
    Sound.WavePeriod = Sound.SamplesPerSec / Sound.ToneHz;
    Sound.BytesPerSample = sizeof(int16) * Sound.Channels;
    Sound.BufferSize = Sound.SamplesPerSec * Sound.BytesPerSample;
    Sound.RunningSampleIndex = 0;
    Sound.LatencySampleCount = Sound.SamplesPerSec / 15;

    Sound.ToneVolume = 3000;

    return Sound;
}


internal void
WinCanvasSoundFill(WinCanvasSound *Sound, DWORD ByteToLock, DWORD BytesToWrite) {

    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;


    if (SUCCEEDED(GlobalSoundBuffer->Lock(ByteToLock, BytesToWrite, //
                                          &Region1, &Region1Size,   //
                                          &Region2, &Region2Size,   //
                                          0))) {

        int16 *SampleOut = (int16 *)Region1;
        DWORD Region1SampleCount = Region1Size / Sound->BytesPerSample;
        for (DWORD SampleIdx = 0; SampleIdx < Region1SampleCount; SampleIdx += 1) {
            // real32 t =
            //     ((real32)Sound->RunningSampleIndex / (real32)Sound->WavePeriod) * Pi32 * 2.0f;

            real32 SineValue = sinf(Sound->tSine);

            int16 SampleValue = (int16)(SineValue * (real32)Sound->ToneVolume);

            *SampleOut = SampleValue;
            SampleOut += 1;

            *SampleOut = SampleValue;
            SampleOut += 1;

            Sound->tSine += (1.0f * Pi32 * 2.0f) / (real32)Sound->WavePeriod;
            Sound->RunningSampleIndex += 1;
        }

        DWORD Region2SampleCount = Region2Size / Sound->BytesPerSample;
        SampleOut = (int16 *)Region2;
        for (DWORD SampleIdx = 0; SampleIdx < Region2SampleCount; SampleIdx += 1) {
            // real32 t =
            //     ((real32)Sound->RunningSampleIndex / (real32)Sound->WavePeriod) * Pi32 * 2.0f;

            real32 SineValue = sinf(Sound->tSine);

            int16 SampleValue = (int16)(SineValue * (real32)Sound->ToneVolume);

            *SampleOut = SampleValue;
            SampleOut += 1;

            *SampleOut = SampleValue;
            SampleOut += 1;

            Sound->tSine += (1.0f * Pi32 * 2.0f) / (real32)Sound->WavePeriod;
            Sound->RunningSampleIndex += 1;
        }

        GlobalSoundBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}


internal WinCanvasDimensions
WinCanvasGetDimensions(HWND WindowHandle) {
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);

    int Width = ClientRect.right - ClientRect.left;
    int Height = ClientRect.bottom - ClientRect.top;

    return WinCanvasDimensions{Width, Height};
}



internal void
WinCanvasCreateDibSection(int Width, int Height) {

    if (GlobalWinCanvas.Memory) {
        VirtualFree(GlobalWinCanvas.Memory, 0, MEM_RELEASE);
    }

    GlobalWinCanvas.Width = Width;
    GlobalWinCanvas.Height = Height;

    GlobalWinCanvas.Info.bmiHeader.biSize = sizeof(GlobalWinCanvas.Info.bmiHeader);
    GlobalWinCanvas.Info.bmiHeader.biWidth = GlobalWinCanvas.Width;
    GlobalWinCanvas.Info.bmiHeader.biHeight = -GlobalWinCanvas.Height; // Windows
                                                                       // Convention
                                                                       // Bullshit
    GlobalWinCanvas.Info.bmiHeader.biPlanes = 1;
    GlobalWinCanvas.Info.bmiHeader.biBitCount = 32;
    GlobalWinCanvas.Info.bmiHeader.biCompression = BI_RGB;

    GlobalWinCanvas.BytesPerPixel = 4;

    GlobalWinCanvas.Size =
        GlobalWinCanvas.Width * GlobalWinCanvas.Height * GlobalWinCanvas.BytesPerPixel;

    GlobalWinCanvas.Memory =
        VirtualAlloc(0, GlobalWinCanvas.Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    GlobalWinCanvas.Pitch = GlobalWinCanvas.Width * GlobalWinCanvas.BytesPerPixel;
}



internal void
WinCanvasDisplayBitmap(HDC DeviceCtx, int WindowWidth, int WindowHeight) {

    int DestWidth = WindowWidth;
    int DestHeight = WindowHeight;

    // float AspectRatio = (float)GlobalWinCanvas.Width /
    //    (float)GlobalWinCanvas.Height;
    //
    // if (WindowWidth > WindowHeight) { DestHeight = (int)((float)WindowWidth /
    // AspectRatio); } else { 	DestWidth = (int)((float)WindowHeight *
    // AspectRatio);
    // }

    int DestX = 0;
    int DestY = 0;

    StretchDIBits(DeviceCtx,                                           //
                  DestX, DestY, DestWidth, DestHeight,                 // Destination Dimensions
                  0, 0, GlobalWinCanvas.Width, GlobalWinCanvas.Height, // Source Dimensions
                  GlobalWinCanvas.Memory, &GlobalWinCanvas.Info, DIB_RGB_COLORS, SRCCOPY);
}



LRESULT
WinCanvasWindowCallBack(HWND WindowHandle, UINT Message, WPARAM wParam, LPARAM lParam) {
    LRESULT Result = 0;

    switch (Message) {

        case WM_SIZE: {

        } break;

        case WM_DESTROY: {
            GlobalRunning = false;
        } break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP: {
            uint32 VKCode = wParam;
            bool WasDown = ((lParam & (1 << 30)) != 0);
            bool IsDown = ((lParam & (1 << 31)) == 0);

            if (VKCode == VK_ESCAPE) {
                GlobalRunning = false;
            }

            if (VKCode == 'W') {
            }
            if (VKCode == 'A') {
            }
            if (VKCode == 'S') {
            }
            if (VKCode == 'D') {
            }

            if (VKCode == VK_SPACE) {
            }
        } break;

        case WM_CLOSE: {
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP: {

        } break;

        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceCtx = BeginPaint(WindowHandle, &Paint);

            { // Flushing the window with BLACKNESS
                int X = Paint.rcPaint.left;
                int Y = Paint.rcPaint.top;
                int Width = Paint.rcPaint.right - X;
                int Height = Paint.rcPaint.bottom - Y;
                PatBlt(DeviceCtx, X, Y, Width, Height, BLACKNESS);
            }

            WinCanvasDimensions Dimensions = WinCanvasGetDimensions(WindowHandle);
            WinCanvasDisplayBitmap(DeviceCtx, Dimensions.Width, Dimensions.Height);

            EndPaint(WindowHandle, &Paint);
        } break;

        default: {
            Result = DefWindowProcA(WindowHandle, Message, wParam, lParam);
        } break;
    }

    return Result;
}



int32
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd) {

    LARGE_INTEGER FreqStructResult = {};
    QueryPerformanceFrequency(&FreqStructResult);
    int64 PerfCounterFrequency = FreqStructResult.QuadPart;

    WinCanvasLoadXInput();
    WinCanvasCreateDibSection(1280, 720);

    LPCSTR WindowClassName = "WinCanvasWindowClass";

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WinCanvasWindowCallBack;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = WindowClassName;


    if (RegisterClassA(&WindowClass)) {

        HWND WindowHandle = CreateWindowExA(0, WindowClassName,
                                            "WinCanvas",                      // Title/Caption
                                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Style
                                            // Position and Size
                                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                            CW_USEDEFAULT, 0, 0, Instance, 0); // Other stuff

        if (WindowHandle) {

            // Graphic Test
            int32 XOff = 0;
            int32 YOff = 0;

            // Sound Test
            // ----------------------------------------------------- //
            WinCanvasSound Sound = WinCanvasInitSound();

            // Sound init after a window is created
            WinCanvasInitDSound(WindowHandle, Sound.BufferSize, Sound.SamplesPerSec);
            WinCanvasSoundFill(&Sound, 0, Sound.LatencySampleCount * Sound.BytesPerSample);

            GlobalSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
            // ----------------------------------------------------- //

            GlobalRunning = true;

            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);

            uint64 LastCycleCount = __rdtsc();
            while (GlobalRunning) {

                LARGE_INTEGER BeginCounter;
                QueryPerformanceCounter(&BeginCounter);


                MSG Message;
                while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
                    if (Message.message == WM_QUIT) {
                        GlobalRunning = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                // Gamepad
                for (DWORD ControllerIdx = 0; ControllerIdx < XUSER_MAX_COUNT; ControllerIdx += 1) {

                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIdx, &ControllerState) == ERROR_SUCCESS) {
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                        bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);

                        bool LeftThumb = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
                        bool RightThumb = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 LStickX = Pad->sThumbLX;
                        int16 LStickY = Pad->sThumbLY;

                        int16 RStickX = Pad->sThumbRX;
                        int16 RStickY = Pad->sThumbRY;

                        uint8 LeftTrigger = Pad->bLeftTrigger;
                        uint8 RightTrigger = Pad->bRightTrigger;

                        XOff += LStickX / 4096;
                        YOff -= LStickY / 4096;

                        if (Up) {
                            YOff -= 2;
                        }

                        if (Down) {
                            YOff += 2;
                        }

                        if (Left) {
                            XOff -= 2;
                        }

                        if (Right) {
                            XOff += 2;
                        }

                        if (Back) {
                            GlobalRunning = false;
                        }

                        if (RightShoulder) {
                            Sound.ToneHz += 1;
                        }

                        if (LeftShoulder) {
                            Sound.ToneHz -= 1;
                        }

                        Sound.WavePeriod = Sound.SamplesPerSec / (Sound.ToneHz + RightTrigger);
                        // Sound.ToneVolume = (int32)((3000.0f) * ((real32)LeftTrigger / 255.0f));

                        // XINPUT_VIBRATION Vibration;
                        // Vibration.wLeftMotorSpeed = 60000;
                        // Vibration.wRightMotorSpeed = 60000;
                        // XInputSetState(ControllerIdx, &Vibration);
                    } else {

                        // Controller not found
                    }
                }

                CanvasBitMap BitMap = {};
                BitMap.Memory = GlobalWinCanvas.Memory;
                BitMap.Width = GlobalWinCanvas.Width;
                BitMap.Height = GlobalWinCanvas.Height;
                BitMap.Size = GlobalWinCanvas.Size;
                BitMap.Pitch = GlobalWinCanvas.Pitch;
                BitMap.BytesPerPixel = GlobalWinCanvas.BytesPerPixel;

                CanvasUpdateAndRender(BitMap, XOff, YOff);

                // Sound Writting is pretty tough ---------------------------- //

                DWORD PlayCursor = 0;
                DWORD WriteCursor = 0;

                if (SUCCEEDED(GlobalSoundBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {

                    DWORD TargetCursor =
                        (PlayCursor + (Sound.LatencySampleCount * Sound.BytesPerSample)) %
                        Sound.BufferSize;

                    DWORD ByteToLock =
                        (Sound.RunningSampleIndex * Sound.BytesPerSample) % Sound.BufferSize;
                    DWORD BytesToWrite = 0;


                    if (ByteToLock > TargetCursor) {

                        BytesToWrite = Sound.BufferSize - ByteToLock;
                        BytesToWrite += TargetCursor;
                    } else {

                        BytesToWrite = TargetCursor - ByteToLock;
                    }

                    WinCanvasSoundFill(&Sound, ByteToLock, BytesToWrite);
                }
                // Sound Writting is pretty tough ---------------------------- //

                // --------------------------------------------------------------- //
                HDC DeviceCtx = GetDC(WindowHandle);
                WinCanvasDimensions Dimensions = WinCanvasGetDimensions(WindowHandle);
                WinCanvasDisplayBitmap(DeviceCtx, Dimensions.Width, Dimensions.Height);

                ReleaseDC(WindowHandle, DeviceCtx);
                // --------------------------------------------------------------- //

                uint64 EndCycleCount = __rdtsc();

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);

                int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                real64 MSPerFrame =
                    (1000.0f * (real64)CounterElapsed) / (real64)PerfCounterFrequency;
                real64 FPS = 1000.0f / MSPerFrame;
                real64 MegaCyclesElapsed =
                    (((real64)EndCycleCount - (real64)LastCycleCount) / (1000.0f * 1000.0f));
                char Buffer[256];
                sprintf(Buffer, "%.03fms, %.03ffps, %.03fMC/F \n", MSPerFrame, FPS,
                        MegaCyclesElapsed);
                OutputDebugStringA(Buffer);

                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
            }

        } else {
            OutputDebugStringA("Failed at creation of window handle.");
        }
    } else {
        OutputDebugStringA("Registering the window class failed.");
    }

    return 0;
}
