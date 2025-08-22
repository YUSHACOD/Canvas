#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include <cstdint>
#include <windows.h>
#include <Xinput.h>
#include <dsound.h>


// Sugars ---------------------------------------------------- //
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define global static
#define internal static
#define local_persist static
// Sugars ---------------------------------------------------- //


typedef struct {
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Size;
    int Pitch;
    int BytesPerPixel;
} CanvasBitMap;

typedef struct {
    int Width;
    int Height;
} CanvasDimensions;


// Globals --------------------------------------------------//
global bool GlobalRunning;
global CanvasBitMap GlobalCanvas;
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
CanvasLoadXInput() {
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
CanvasInitDSound(HWND WindowHandle, int32 BufferSize, int32 SamplesPerSec) {
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



internal CanvasDimensions
CanvasGetDimensions(HWND WindowHandle) {
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);

    int Width = ClientRect.right - ClientRect.left;
    int Height = ClientRect.bottom - ClientRect.top;

    return CanvasDimensions{Width, Height};
}



internal void
CanvasDraw(int XOff, int YOff) {

    uint8 *Row = (uint8 *)GlobalCanvas.Memory;

    for (int Y = 0; Y < GlobalCanvas.Height; Y += 1) {

        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < GlobalCanvas.Width; X += 1) {

            // Blue
            uint8 blue = (uint8)(X + XOff);
            uint8 green = (uint8)(Y + YOff);
            uint8 red = 0;
            uint8 pad = 0;

            *Pixel =
                ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);

            Pixel += 1;
        }

        Row += GlobalCanvas.Pitch;
    }
}



internal void
CanvasCreateDibSection(int Width, int Height) {

    if (GlobalCanvas.Memory) {
        VirtualFree(GlobalCanvas.Memory, 0, MEM_RELEASE);
    }

    GlobalCanvas.Width = Width;
    GlobalCanvas.Height = Height;

    GlobalCanvas.Info.bmiHeader.biSize = sizeof(GlobalCanvas.Info.bmiHeader);
    GlobalCanvas.Info.bmiHeader.biWidth = GlobalCanvas.Width;
    GlobalCanvas.Info.bmiHeader.biHeight = -GlobalCanvas.Height; // Windows
                                                                 // Convention
                                                                 // Bullshit
    GlobalCanvas.Info.bmiHeader.biPlanes = 1;
    GlobalCanvas.Info.bmiHeader.biBitCount = 32;
    GlobalCanvas.Info.bmiHeader.biCompression = BI_RGB;

    GlobalCanvas.BytesPerPixel = 4;

    GlobalCanvas.Size = GlobalCanvas.Width * GlobalCanvas.Height * GlobalCanvas.BytesPerPixel;

    GlobalCanvas.Memory =
        VirtualAlloc(0, GlobalCanvas.Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    GlobalCanvas.Pitch = GlobalCanvas.Width * GlobalCanvas.BytesPerPixel;
    CanvasDraw(0, 0);
}



internal void
CanvasDisplayBitmap(HDC DeviceCtx, int WindowWidth, int WindowHeight) {

    int DestWidth = WindowWidth;
    int DestHeight = WindowHeight;

    // float AspectRatio = (float)GlobalCanvas.Width /
    //    (float)GlobalCanvas.Height;
    //
    // if (WindowWidth > WindowHeight) { DestHeight = (int)((float)WindowWidth /
    // AspectRatio); } else { 	DestWidth = (int)((float)WindowHeight *
    // AspectRatio);
    // }

    int DestX = 0;
    int DestY = 0;

    StretchDIBits(DeviceCtx,                                     //
                  DestX, DestY, DestWidth, DestHeight,           // Destination Dimensions
                  0, 0, GlobalCanvas.Width, GlobalCanvas.Height, // Source Dimensions
                  GlobalCanvas.Memory, &GlobalCanvas.Info, DIB_RGB_COLORS, SRCCOPY);
}



LRESULT
CanvasWindowCallBack(HWND WindowHandle, UINT Message, WPARAM wParam, LPARAM lParam) {
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

            CanvasDimensions Dimensions = CanvasGetDimensions(WindowHandle);
            CanvasDisplayBitmap(DeviceCtx, Dimensions.Width, Dimensions.Height);

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

    CanvasLoadXInput();
    CanvasCreateDibSection(1280, 720);

    LPCSTR WindowClassName = "CanvasWindowClass";

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = CanvasWindowCallBack;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = WindowClassName;

    if (RegisterClassA(&WindowClass)) {

        HWND WindowHandle = CreateWindowExA(0, WindowClassName,
                                            "Canvas",                         // Title/Caption
                                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Style
                                            // Position and Size
                                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                            CW_USEDEFAULT, 0, 0, Instance, 0); // Other stuff

        if (WindowHandle) {

            // Graphic Test
            int32 XOff = 0;
            int32 YOff = 0;

            // Sound Test
            int32 Channels = 2;
            int32 SamplesPerSec = 48000;
            int32 ToneHz = 256;
            int32 SquareWaveCounter = 0;
            int32 SquareWavePeriod = SamplesPerSec / ToneHz;
            int32 HalfSquareWavePeriod = SquareWavePeriod / 2;
            int32 BytesPerSample = sizeof(int16) * Channels;
            int32 GlobalSoundBufferSize = SamplesPerSec * BytesPerSample;
            uint32 RunningSampleIndex = 0;

            int32 ToneVolume = 16000;



            // Sound init after a window is created
            CanvasInitDSound(WindowHandle, SamplesPerSec, SamplesPerSec * BytesPerSample);

            GlobalSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GlobalRunning = true;
            while (GlobalRunning) {

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

                        XOff += LStickX >> 12;
                        YOff += LStickY >> 12;

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

                        // XINPUT_VIBRATION Vibration;
                        // Vibration.wLeftMotorSpeed = 60000;
                        // Vibration.wRightMotorSpeed = 60000;
                        // XInputSetState(ControllerIdx, &Vibration);
                    } else {

                        // Controller not found
                    }
                }

                CanvasDraw(XOff, YOff);

                // Sound Writting is pretty tough ---------------------------- //

                DWORD PlayCursor = 0;
                DWORD WriteCursor = 0;

                if (SUCCEEDED(GlobalSoundBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {

                    // DirectSound test
                    VOID *Region1;
                    DWORD Region1Size;
                    VOID *Region2;
                    DWORD Region2Size;

                    DWORD BytesToLock =
                        (RunningSampleIndex * BytesPerSample) % GlobalSoundBufferSize;
                    DWORD BytesToWrite = 0;
                    if (BytesToLock > PlayCursor) {
                        BytesToWrite = GlobalSoundBufferSize - BytesToLock;
                        BytesToWrite += PlayCursor;
                    } else {
                        BytesToWrite = PlayCursor + BytesToLock;
                    }

                    if (SUCCEEDED(GlobalSoundBuffer->Lock(BytesToLock, BytesToWrite, //
                                                          &Region1, &Region1Size,    //
                                                          &Region2, &Region2Size,    //
                                                          0))) {

                        int16 *SampleOut = (int16 *)Region1;
                        DWORD Region1SampleCount = Region1Size / BytesPerSample;
                        for (DWORD SampleIdx = 0; SampleIdx < Region1SampleCount; SampleIdx += 1) {

                            int16 SampleValue = ((RunningSampleIndex / HalfSquareWavePeriod) % 2)
                                                    ? ToneVolume
                                                    : -ToneVolume;

                            *SampleOut = SampleValue;
                            SampleOut += 1;

                            *SampleOut = SampleValue;
                            SampleOut += 1;

                            RunningSampleIndex += 1;
                        }

                        DWORD Region2SampleCount = Region2Size / BytesPerSample;
                        SampleOut = (int16 *)Region2;
                        for (DWORD SampleIdx = 0; SampleIdx < Region2SampleCount; SampleIdx += 1) {
                            int16 SampleValue = ((RunningSampleIndex / HalfSquareWavePeriod) % 2)
                                                    ? ToneVolume
                                                    : -ToneVolume;

                            *SampleOut = SampleValue;
                            SampleOut += 1;

                            *SampleOut = SampleValue;
                            SampleOut += 1;

                            RunningSampleIndex += 1;
                        }

                        GlobalSoundBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
                    }
                }
                // Sound Writting is pretty tough ---------------------------- //

                // --------------------------------------------------------------- //
                HDC DeviceCtx = GetDC(WindowHandle);
                CanvasDimensions Dimensions = CanvasGetDimensions(WindowHandle);
                CanvasDisplayBitmap(DeviceCtx, Dimensions.Width, Dimensions.Height);

                ReleaseDC(WindowHandle, DeviceCtx);
                // --------------------------------------------------------------- //
            }

        } else {
            OutputDebugStringA("Failed at creation of window handle.");
        }
    } else {
        OutputDebugStringA("Registering the window class failed.");
    }

    return 0;
}
