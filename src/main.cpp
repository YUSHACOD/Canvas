#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include <cstdint>
#include <windows.h>

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

internal CanvasDimensions CanvasGetDimensions(HWND WindowHandle) {
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);

    int Width = ClientRect.right - ClientRect.left;
    int Height = ClientRect.bottom - ClientRect.top;

    return CanvasDimensions{Width, Height};
}

internal void CanvasDraw(int XOff, int YOff) {

    uint8 *Row = (uint8 *)GlobalCanvas.Memory;

    for (int Y = 0; Y < GlobalCanvas.Height; Y += 1) {

        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < GlobalCanvas.Width; X += 1) {

            // Blue
            uint8 blue = (uint8)(X + XOff);
            uint8 green = (uint8)(Y + YOff);
            uint8 red = 0;
            uint8 pad = 0;

            *Pixel = ((uint32)pad << 24) | ((uint32)red << 16) |
                     ((uint32)green << 8) | ((uint32)blue);

            Pixel += 1;
        }

        Row += GlobalCanvas.Pitch;
    }
}

internal void CanvasCreateDibSection(int Width, int Height) {

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

    GlobalCanvas.Size =
        GlobalCanvas.Width * GlobalCanvas.Height * GlobalCanvas.BytesPerPixel;

    GlobalCanvas.Memory =
        VirtualAlloc(0, GlobalCanvas.Size, MEM_COMMIT, PAGE_READWRITE);

    GlobalCanvas.Pitch = GlobalCanvas.Width * GlobalCanvas.BytesPerPixel;
    CanvasDraw(0, 0);
}

internal void CanvasDisplayBitmap(HDC DeviceCtx, int WindowWidth,
                                  int WindowHeight) {

    int DestX = 10;
    int DestY = 10;
    int DestWidth = WindowWidth - 20;
    int DestHeight = WindowHeight - 20;

    StretchDIBits(
        DeviceCtx,                                     //
        DestX, DestY, DestWidth, DestHeight,           // Destination Dimensions
        0, 0, GlobalCanvas.Width, GlobalCanvas.Height, // Source Dimensions
        GlobalCanvas.Memory, &GlobalCanvas.Info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CanvasWindowCallBack(HWND WindowHandle, UINT Message, WPARAM wParam,
                             LPARAM lParam) {
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

int32 WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine,
              int ShowCmd) {

    CanvasCreateDibSection(1280, 720);

    LPCSTR WindowClassName = "CanvasWindowClass";

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = CanvasWindowCallBack;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = WindowClassName;

    if (RegisterClassA(&WindowClass)) {

        HWND WindowHandle = CreateWindowExA(
            0, WindowClassName,

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

                CanvasDraw(XOff, YOff);

                HDC DeviceCtx = GetDC(WindowHandle);

                CanvasDimensions Dimensions = CanvasGetDimensions(WindowHandle);
                CanvasDisplayBitmap(DeviceCtx, Dimensions.Width,
                                    Dimensions.Height);

                ReleaseDC(WindowHandle, DeviceCtx);

                XOff += 1;
                YOff += 1;
            }

        } else {
            OutputDebugStringA("Failed at creation of window handle.");
        }
    } else {
        OutputDebugStringA("Registering the window class failed.");
    }

    return 0;
}
