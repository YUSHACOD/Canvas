#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")

/*
 *  TODO: Seperating the Platform layer
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

#include <stdio.h>

#include "canvas_sugars.hpp"
#include "canvas.hpp"

#include "windows_structs.hpp"

#define MonitorRefreshRate 144
#define GameUpdateHz       (MonitorRefreshRate / 1)
#define FramesOfDelay      10

// Globals --------------------------------------------------//
global bool              GlobalRunning;
global WinPlatBitMap     GlobalBitMap;
global WinPlatDimensions ScreenDim = {};
// Globals --------------------------------------------------//


// XInput Shenanigans ------------------------------------------------------- //
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

internal void WinPlatLoadXInput() {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");

    if (XInputLibrary) {
        XInputGetState = (xinput_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (xinput_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    } else {
        OutputDebugStringA("Couldn't Load XInput\n");
    }
}
// -------------------------------------------------------------------------- //



internal void
WinPlatProcessXInputButton(CanvasButtonState *Old, CanvasButtonState *New, bool IsSet) {
    New->EndedDown = IsSet;
    New->Transitions += (Old->EndedDown ^ New->EndedDown) ? 1 : 0;
}

internal void
WinPlatProcessXInputAnalog(CanvasAnalogState *Old, CanvasAnalogState *New, real32 Val) {
    New->End = New->Min = New->Max = Val;
    New->Start                     = Old->End;
}



internal WinPlatDimensions WinPlatGetDimensions(HWND WindowHandle) {
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);

    uint32 Width  = ClientRect.right - ClientRect.left;
    uint32 Height = ClientRect.bottom - ClientRect.top;

    return WinPlatDimensions{Width, Height};
}



internal void WinPlatCreateDibSection(uint32 Width, uint32 Height) {
    if (GlobalBitMap.Memory) {
        VirtualFree(GlobalBitMap.Memory, 0, MEM_RELEASE);
    }

    GlobalBitMap.Width  = Width;
    GlobalBitMap.Height = Height;

    GlobalBitMap.Info.bmiHeader.biSize   = sizeof(GlobalBitMap.Info.bmiHeader);
    GlobalBitMap.Info.bmiHeader.biWidth  = GlobalBitMap.Width;
    GlobalBitMap.Info.bmiHeader.biHeight = -(int32)GlobalBitMap.Height; // Windows
                                                                        // Convention
                                                                        // Bullshit
    GlobalBitMap.Info.bmiHeader.biPlanes      = 1;
    GlobalBitMap.Info.bmiHeader.biBitCount    = 32;
    GlobalBitMap.Info.bmiHeader.biCompression = BI_RGB;

    GlobalBitMap.BytesPerPixel = 4;

    GlobalBitMap.Size = GlobalBitMap.Width * GlobalBitMap.Height * GlobalBitMap.BytesPerPixel;

    GlobalBitMap.Memory =
        VirtualAlloc(0, GlobalBitMap.Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    GlobalBitMap.Pitch = GlobalBitMap.Width * GlobalBitMap.BytesPerPixel;
}



internal void WinPlatDisplayBitmap(HDC DeviceCtx, uint32 WindowWidth, uint32 WindowHeight) {
    uint32 DestWidth  = WindowWidth;
    uint32 DestHeight = WindowHeight;

    real32 AspectRatioScr = (real32)ScreenDim.Width / (real32)ScreenDim.Height;
    real32 AspectRatioWin = (real32)WindowWidth / (real32)WindowHeight;

    uint32 DestY = 0;
    uint32 DestX = 0;

    if (AspectRatioScr >= AspectRatioWin) {
        DestY      = DestHeight;
        DestHeight = (uint32)((real32)WindowWidth / AspectRatioScr);
        DestY      = (DestY - DestHeight) / 2;
    } else {
        DestX     = DestWidth;
        DestWidth = (uint32)((real32)WindowHeight * AspectRatioScr);
        DestX     = (DestX - DestWidth) / 2;
    }


    StretchDIBits(DeviceCtx, //
                  DestX,
                  DestY,
                  DestWidth,
                  DestHeight, // Destination Dimensions
                  0,
                  0,
                  GlobalBitMap.Width,
                  GlobalBitMap.Height, // Source Dimensions
                  GlobalBitMap.Memory,
                  &GlobalBitMap.Info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

DEBUGFileStruct DEBUGPlatformReadEntireFile(char *FileName) {
    DEBUGFileStruct Result = {};

    HANDLE FileHandle =
        CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize)) {
            Result.Memory =
                VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (Result.Memory) {
                uint32 FileSize32 = SafeTruncateU64(FileSize.QuadPart);
                DWORD  BytesToRead;
                if (ReadFile(FileHandle, Result.Memory, FileSize32, &BytesToRead, 0) &&
                    (FileSize32 == BytesToRead)) {
                    Result.Size = FileSize32;
                } else {
                    if (Result.Memory) {
                        VirtualFree(Result.Memory, 0, MEM_RELEASE);
                    }
                }
            }
        }
        CloseHandle(FileHandle);
    } else {
    }

    return Result;
}



void DEBUGPlatformFreeFileMemory(void *Memory) {
    if (Memory) {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}



bool DEBUGPlatformWriteEntireFile(char *FileName, void *Memory, uint32 MemorySize) {
    bool Result = false;

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        if (Memory) {
            uint32 MemmorySize32 = SafeTruncateU64(MemorySize);
            DWORD  BytesToWrite;

            if (WriteFile(FileHandle, Memory, MemmorySize32, &BytesToWrite, 0)) {
                Result = true;
            } else {
                OutputDebugStringA("Couldn't Write the File\n");
            }
        } else {
            OutputDebugStringA("The Memory is Null\n");
        }
        CloseHandle(FileHandle);
    } else {
        OutputDebugStringA("File, not opened\n");
    }

    return Result;
}



internal void
WinPlatProcessXInput(CanvasInput *Inputs, CanvasInput *OldInput, CanvasInput *NewInput) {
    uint32 MaxControllerCount = XUSER_MAX_COUNT;
    if (MaxControllerCount > ArrayLen(Inputs[0].Controllers)) {
        MaxControllerCount = ArrayLen(Inputs[0].Controllers);
    }

    for (DWORD ControllerIdx = 0; ControllerIdx < MaxControllerCount; ControllerIdx += 1) {
        XINPUT_STATE ControllerState;
        if (XInputGetState(ControllerIdx, &ControllerState) == ERROR_SUCCESS) {
            // Unpacking of Gamepad Inputs ------------------------------------------ //
            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

            bool Up    = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down  = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left  = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

            bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
            bool Stop  = (Pad->wButtons & XINPUT_GAMEPAD_BACK);

            bool LT = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            bool RT = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

            bool LS = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RS = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

            bool A = (Pad->wButtons & XINPUT_GAMEPAD_A);
            bool B = (Pad->wButtons & XINPUT_GAMEPAD_B);
            bool X = (Pad->wButtons & XINPUT_GAMEPAD_X);
            bool Y = (Pad->wButtons & XINPUT_GAMEPAD_Y);


            real32 LStickX = (real32)Pad->sThumbLX;
            real32 LStickY = (real32)Pad->sThumbLY;
            real32 RStickX = (real32)Pad->sThumbRX;
            real32 RStickY = (real32)Pad->sThumbRY;

            // Normalization of sticks
            LStickX = (LStickX < 0) ? (LStickX / 32768.0f) : (LStickX / 32767.0f);
            LStickY = (LStickY < 0) ? (LStickY / 32768.0f) : (LStickY / 32767.0f);
            RStickX = (RStickX < 0) ? (RStickX / 32768.0f) : (RStickX / 32767.0f);
            RStickY = (RStickY < 0) ? (RStickY / 32768.0f) : (RStickY / 32767.0f);

            real32 LeftTrigger  = (real32)Pad->bLeftTrigger / 255.0f;
            real32 RightTrigger = (real32)Pad->bRightTrigger / 255.0f;
            // ---------------------------------------------------------------------- //


            // Process Input for use ------------------------------------------------ //
            CanvasControllerInput *OldController = &(OldInput->Controllers[ControllerIdx]);
            CanvasControllerInput *NewController = &(NewInput->Controllers[ControllerIdx]);

            // Digital ------------------------------------------------------------------- //
            WinPlatProcessXInputButton(&OldController->Up, &NewController->Up, Up);
            WinPlatProcessXInputButton(&OldController->Down, &NewController->Down, Down);
            WinPlatProcessXInputButton(&OldController->Left, &NewController->Left, Left);
            WinPlatProcessXInputButton(&OldController->Right, &NewController->Right, Right);

            WinPlatProcessXInputButton(&OldController->Start, &NewController->Start, Start);
            WinPlatProcessXInputButton(&OldController->Stop, &NewController->Stop, Stop);

            WinPlatProcessXInputButton(&OldController->LT, &NewController->LT, LT);
            WinPlatProcessXInputButton(&OldController->RT, &NewController->RT, RT);

            WinPlatProcessXInputButton(&OldController->LS, &NewController->LS, LS);
            WinPlatProcessXInputButton(&OldController->RS, &NewController->RS, RS);

            WinPlatProcessXInputButton(&OldController->A, &NewController->A, A);
            WinPlatProcessXInputButton(&OldController->B, &NewController->B, B);
            WinPlatProcessXInputButton(&OldController->X, &NewController->X, X);
            WinPlatProcessXInputButton(&OldController->Y, &NewController->Y, Y);
            // --------------------------------------------------------------------------- //

            // Analog -------------------------------------------------------------------- //
            WinPlatProcessXInputAnalog(
                &OldController->LeftTrigger, &NewController->LeftTrigger, LeftTrigger);

            WinPlatProcessXInputAnalog(
                &OldController->RightTrigger, &NewController->RightTrigger, RightTrigger);


            WinPlatProcessXInputAnalog(
                &OldController->LeftStickY, &NewController->LeftStickY, LStickY);
            WinPlatProcessXInputAnalog(
                &OldController->LeftStickX, &NewController->LeftStickX, LStickX);

            WinPlatProcessXInputAnalog(
                &OldController->RightStickY, &NewController->RightStickY, RStickY);
            WinPlatProcessXInputAnalog(
                &OldController->RightStickX, &NewController->RightStickX, RStickX); // ----------------------------------------------------------------------
                                                                                    // //



            // XINPUT_VIBRATION Vibration;
            // Vibration.wLeftMotorSpeed = 60000;
            // Vibration.wRightMotorSpeed = 60000;
            // XInputSetState(ControllerIdx, &Vibration);
        } else {
            // OutputDebugStringA("Couldn't Get GamePad State\n");
        }
    }
}



internal void WinPlatProcessWindowMessages(void) {

    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
        WPARAM wParam = Message.wParam;
        // LPARAM lParam = Message.lParam;
        switch (Message.message) {
            case WM_QUIT: {
                GlobalRunning = false;
            } break;

                // case WM_SIZE: {
                //
                // } break;

            case WM_DESTROY: {
                GlobalRunning = false;
            } break;

            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {
                uint32 VKCode = (uint32)wParam;
                // bool WasDown = ((lParam & (uint64)(1 << 30)) != 0);
                // bool IsDown = ((lParam & (uint64)(1 << 31)) == 0);

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

                // case WM_ACTIVATEAPP: {
                //
                // } break;

                // case WM_PAINT: {
                //     PAINTSTRUCT Paint;
                //     HDC DeviceCtx = BeginPaint(WindowHandle, &Paint);
                //
                //     { // Flushing the window with BLACKNESS
                //         int X = Paint.rcPaint.left;
                //         int Y = Paint.rcPaint.top;
                //         int Width = Paint.rcPaint.right - X;
                //         int Height = Paint.rcPaint.bottom - Y;
                //         PatBlt(DeviceCtx, X, Y, Width, Height, BLACKNESS);
                //     }
                //
                //     WinPlatDimensions Dimensions =
                //     WinPlatGetDimensions(WindowHandle);
                //     WinPlatDisplayBitmap(DeviceCtx, Dimensions.Width,
                //     Dimensions.Height);
                //
                //     EndPaint(WindowHandle, &Paint);
                // } break;

            default: {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}



internal LRESULT WinPlatWindowCallBack(HWND   WindowHandle,
                                       UINT   Message,
                                       WPARAM wParam,
                                       LPARAM lParam) {
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
            Assert(!"Wrong Channel to get Keyboard input, should be from Message dispatches in "
                    "Main Loop");
            // uint32 VKCode = wParam;
            // bool WasDown = ((lParam & (uint64)(1 << 30)) != 0);
            // bool IsDown = ((lParam & (uint64)(1 << 31)) == 0);
            //
            // if (VKCode == VK_ESCAPE) {
            //     GlobalRunning = false;
            // }
            //
            // if (VKCode == 'W') {
            // }
            // if (VKCode == 'A') {
            // }
            // if (VKCode == 'S') {
            // }
            // if (VKCode == 'D') {
            // }
            //
            // if (VKCode == VK_SPACE) {
            // }
        } break;

        case WM_CLOSE: {
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP: {
        } break;

        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC         DeviceCtx = BeginPaint(WindowHandle, &Paint);

            { // Flushing the window with BLACKNESS
                int64 X      = Paint.rcPaint.left;
                int64 Y      = Paint.rcPaint.top;
                int64 Width  = Paint.rcPaint.right - X;
                int64 Height = Paint.rcPaint.bottom - Y;
                PatBlt(DeviceCtx, (int32)X, (int32)Y, (int32)Width, (int32)Height, BLACKNESS);
            }

            WinPlatDimensions Dimensions = WinPlatGetDimensions(WindowHandle);
            WinPlatDisplayBitmap(DeviceCtx, Dimensions.Width, Dimensions.Height);

            EndPaint(WindowHandle, &Paint);
        } break;

        default: {
            Result = DefWindowProcA(WindowHandle, Message, wParam, lParam);
        } break;
    }

    return Result;
}



inline internal int64 WinPlatGetTime() {
    LARGE_INTEGER TimeCounter = {};
    QueryPerformanceCounter(&TimeCounter);
    return TimeCounter.QuadPart;
}


int32 WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd) {
    LARGE_INTEGER FreqStructResult = {};
    QueryPerformanceFrequency(&FreqStructResult);
    int64 PerfCounterFrequency = FreqStructResult.QuadPart;

    UINT SchedulerGranularity = 1;
    bool IsTimeProper         = (timeBeginPeriod(SchedulerGranularity) == TIMERR_NOERROR);

    ScreenDim.Width  = 1920;
    ScreenDim.Height = 1080;

    WinPlatLoadXInput();

    WinPlatCreateDibSection(ScreenDim.Width, ScreenDim.Height);

    LPCSTR WindowClassName = "WinPlatWindowClass";

    WNDCLASSA WindowClass     = {};
    WindowClass.style         = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc   = WinPlatWindowCallBack;
    WindowClass.hInstance     = Instance;
    WindowClass.lpszClassName = WindowClassName;

    // Refesh Rate
    real32 MaxTimePerFrame = 1.0f / (real32)MonitorRefreshRate;

    if (RegisterClassA(&WindowClass)) {
        HWND WindowHandle = CreateWindowExA(0,
                                            WindowClassName,
                                            "Canvas",                         // Title/Caption
                                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Style
                                            // Position and Size
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            0,
                                            0,
                                            Instance,
                                            0);

        if (WindowHandle) {
            // Game Arena Allocations ---------------------------------------------------------- //
            CanvasMemmory Memmory = {};

            Memmory.IsValid   = false;
            Memmory.PermaSize = MegaBytes(64);

#ifdef DEBUG
            LPVOID BaseAdress = (LPVOID)TeraBytes(2);
#else
            LPVOID BaseAdress = 0;
#endif

            Memmory.TransSize = GigaBytes(1);

            Memmory.PermaStore = VirtualAlloc(BaseAdress,
                                              Memmory.PermaSize + Memmory.TransSize,
                                              MEM_RESERVE | MEM_COMMIT,
                                              PAGE_READWRITE);
            Memmory.TransStore = (uint8 *)Memmory.PermaStore + Memmory.PermaSize;
            // --------------------------------------------------------------------------------- //


            GlobalRunning = (Memmory.PermaStore && Memmory.TransStore);

#ifdef DEBUG
            // Perf Metrics ------------------------------------------- //
            int64  LastCounter    = WinPlatGetTime();
            uint64 LastCycleCount = __rdtsc();
            // -------------------------------------------------------- //
#endif

            CanvasInput  Inputs[2] = {};
            CanvasInput *OldInput  = &Inputs[0];
            CanvasInput *NewInput  = &Inputs[1];


            while (GlobalRunning) {
                WinPlatProcessWindowMessages();

                WinPlatProcessXInput(Inputs, OldInput, NewInput);

                CanvasBitMap BitMap = {};

                BitMap.Memory        = GlobalBitMap.Memory;
                BitMap.Width         = GlobalBitMap.Width;
                BitMap.Height        = GlobalBitMap.Height;
                BitMap.Size          = GlobalBitMap.Size;
                BitMap.Pitch         = GlobalBitMap.Pitch;
                BitMap.BytesPerPixel = GlobalBitMap.BytesPerPixel;


                CanvasUpdateAndRender(&Memmory, &BitMap, NewInput, &GlobalRunning);


                // Drawing the Bitmap -------------------------------------------- //
                HDC               DeviceCtx  = GetDC(WindowHandle);
                WinPlatDimensions Dimensions = WinPlatGetDimensions(WindowHandle);


                WinPlatDisplayBitmap(DeviceCtx, Dimensions.Width, Dimensions.Height);
				ZeroMemory(BitMap.Memory, BitMap.Size);

                ReleaseDC(WindowHandle, DeviceCtx);
                // --------------------------------------------------------------- //


                // Profiling Stuff ----------------------------------------------- //
                uint64 EndCycleCount = __rdtsc();

                int64 EndCounter = WinPlatGetTime();

                real64 MegaCyclesElapsed =
                    (((real64)EndCycleCount - (real64)LastCycleCount) / (1000.0f * 1000.0f));

                int64 CounterElapsed = EndCounter - LastCounter;

                real32 TimeElapsedForFrame = (real32)CounterElapsed / (real32)PerfCounterFrequency;

                if (TimeElapsedForFrame < MaxTimePerFrame) {
                    if (IsTimeProper) {
                        DWORD SleepTime =
                            (DWORD)(1000.0f * (MaxTimePerFrame - TimeElapsedForFrame));
                        Sleep(SleepTime);
                    }
                    while (TimeElapsedForFrame < MaxTimePerFrame) {
                        CounterElapsed      = WinPlatGetTime() - LastCounter;
                        TimeElapsedForFrame = (real32)CounterElapsed / (real32)PerfCounterFrequency;
                    }
                } else {
                }


                int64 temp     = LastCounter;
                LastCounter    = WinPlatGetTime();
                LastCycleCount = EndCycleCount;

#ifdef DEBUG
                CounterElapsed = WinPlatGetTime() - temp;
                real64 MSPerFrame =
                    (1000.0f * (real64)CounterElapsed) / (real64)PerfCounterFrequency;
                real64 FPS = 1000.0f / MSPerFrame;
                char   Buffer[256];

                sprintf(
                    Buffer, "%.03fms, %.03ffps, %.03fMC/F \n", MSPerFrame, FPS, MegaCyclesElapsed);
                // OutputDebugStringA(Buffer);
#endif
                // --------------------------------------------------------------- //

                CanvasInput *T = OldInput;
                OldInput       = NewInput;
                NewInput       = T;
            }
        } else {
            OutputDebugStringA("Failed at creation of window handle.\n");
        }
    } else {
        OutputDebugStringA("Registering the window class failed.\n");
    }

    return 0;
}
