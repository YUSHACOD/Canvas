#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include <cstdint>
#include <windows.h>
#include <Xinput.h>


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


// Think about this, learn this --------------------------------------------- //

#define XINPUT_GET(name) DWORD WINAPI name(DWORD UserIndex, XINPUT_STATE *State)
typedef XINPUT_GET(xinput_get_state);
XINPUT_GET(xInputGetStateStub) { return 0; }
global xinput_get_state *XInputGetState_ = xInputGetStateStub;
#define XInputGetState XInputGetState_

#define XINPUT_SET(name) DWORD WINAPI name(DWORD UserIndex, XINPUT_VIBRATION *State)
typedef XINPUT_SET(xinput_set_state);
XINPUT_SET(xInputSetStateStub) { return 0; }
global xinput_set_state *XInputSetState_ = xInputSetStateStub;
#define XInputSetState XInputSetState_

internal void
CanvasLoadXInput() {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");

    if (XInputLibrary) {
        XInputGetState = (xinput_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (xinput_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

// -------------------------------------------------------------------------- //

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


global bool GlobalRunning;
global CanvasBitMap GlobalCanvas;


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

    GlobalCanvas.Memory = VirtualAlloc(0, GlobalCanvas.Size, MEM_COMMIT, PAGE_READWRITE);

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

        HWND WindowHandle =
            CreateWindowExA(0, WindowClassName,

                            "Canvas", // Title/Caption

                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Style

                            // Position and Size
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                            // Other stuff
                            0, 0, Instance, 0);

        if (WindowHandle) {

            int XOff = 0;
            int YOff = 0;

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

                        if (Up) {
                            YOff -= 1;
                        }

                        if (Down) {
                            YOff += 1;
                        }

                        if (Left) {
                            XOff -= 1;
                        }

                        if (Right) {
                            XOff += 1;
                        }

                        if (Back) {
                            GlobalRunning = false;
                        }

                        XINPUT_VIBRATION Vibration;
                        // Vibration.wLeftMotorSpeed = 60000;
                        // Vibration.wRightMotorSpeed = 60000;
                        XInputSetState(ControllerIdx, &Vibration);
                    } else {

                        // Controller not found
                    }
                }

                CanvasDraw(XOff, YOff);
                HDC DeviceCtx = GetDC(WindowHandle);

                CanvasDimensions Dimensions = CanvasGetDimensions(WindowHandle);
                CanvasDisplayBitmap(DeviceCtx, Dimensions.Width, Dimensions.Height);


                ReleaseDC(WindowHandle, DeviceCtx);
            }

        } else {
            OutputDebugStringA("Failed at creation of window handle.");
        }
    } else {
        OutputDebugStringA("Registering the window class failed.");
    }

    return 0;
}
