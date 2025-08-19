#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include <cstdint>
#include <windows.h>

typedef int32_t int32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define global static
#define internal static
#define local_persist static

global bool Running;
global BITMAPINFO BitMapInfo;
global void *BitMapMemory;
global int BitMapWidth;
global int BitMapHeight;
global int BitMapBytesPerPixel;

internal void CanvasDraw(int XOff, int YOff) {
    int Pitch = BitMapWidth * BitMapBytesPerPixel;
    uint8 *Row = (uint8 *)BitMapMemory;

    for (int i = 0; i < BitMapHeight; i += 1) {

        uint32 *Pixel = (uint32 *)Row;
        for (int j = 0; j < BitMapWidth; j += 1) {

            // Blue
            uint8 blue = (uint8)(j + XOff);
            uint8 green = (uint8)(i + YOff);
            uint8 red = 0;
            uint8 pad = 0;

            *Pixel = ((uint32)pad << 24) | ((uint32)red << 16) |
                     ((uint32)green << 8) | ((uint32)blue);

            Pixel += 1;
        }

        Row += Pitch;
    }
}

internal void CanvasResizeDibSection(int Width, int Height) {

    if (BitMapMemory) {
        VirtualFree(BitMapMemory, 0, MEM_RELEASE);
    }

    BitMapWidth = Width;
    BitMapHeight = Height;

    BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
    BitMapInfo.bmiHeader.biWidth = BitMapWidth;
    BitMapInfo.bmiHeader.biHeight = -BitMapHeight;
    BitMapInfo.bmiHeader.biPlanes = 1;
    BitMapInfo.bmiHeader.biBitCount = 32;
    BitMapInfo.bmiHeader.biCompression = BI_RGB;

    BitMapBytesPerPixel = 4;

    int BitMapSize = BitMapWidth * BitMapHeight * BitMapBytesPerPixel;
    BitMapMemory = VirtualAlloc(0, BitMapSize, MEM_COMMIT, PAGE_READWRITE);

    CanvasDraw(200, 0);
}

internal void CanvasUpdateWindow(HDC DeviceCtx, RECT WindowDimensions) {

    int WindowWidth = WindowDimensions.right - WindowDimensions.left;
    int WindowHeight = WindowDimensions.bottom - WindowDimensions.top;

    StretchDIBits(DeviceCtx,                       //
                  0, 0, WindowWidth, WindowHeight, // Destination Dimensions
                  0, 0, BitMapWidth, BitMapHeight, // Source Dimensions
                  BitMapMemory, &BitMapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT WindowProcedureA(HWND WindowHandle, UINT Message, WPARAM wParam,
                         LPARAM lParam) {
    LRESULT Result = 0;

    switch (Message) {

        case WM_SIZE: {
            RECT ClientRect;
            GetClientRect(WindowHandle, &ClientRect);

            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;

            CanvasResizeDibSection(Width, Height);
        } break;

        case WM_DESTROY: {
            Running = false;
        } break;

        case WM_CLOSE: {
            Running = false;
        } break;

        case WM_ACTIVATEAPP: {

        } break;

        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceCtx = BeginPaint(WindowHandle, &Paint);

            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            RECT ClientRect;
            GetClientRect(WindowHandle, &ClientRect);

            CanvasUpdateWindow(DeviceCtx, ClientRect);

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

    OutputDebugStringA("Range");

    LPCSTR WindowClassName = "CanvasWindowClassName";

    WNDCLASSA WindowClass = {
        CS_HREDRAW | CS_VREDRAW, WindowProcedureA, 0, 0, Instance, 0, 0, 0, 0,
        WindowClassName,
    };

    if (RegisterClassA(&WindowClass)) {

        HWND WindowHandle = CreateWindowExA(0, WindowClassName,

                                            // Title/Caption
                                            "Canvas",

                                            // Style
                                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,

                                            // Position and Size
                                            CW_USEDEFAULT, CW_USEDEFAULT,
                                            CW_USEDEFAULT, CW_USEDEFAULT,

                                            // Other stuff
                                            0, 0, Instance, 0);

        if (WindowHandle) {

            MSG Message;
            int XOff = 0;
            int YOff = 0;

            Running = true;
            while (Running) {
                while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
                    if (Message.message == WM_QUIT) {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                CanvasDraw(XOff, YOff);
                HDC DeviceCtx = GetDC(WindowHandle);

                RECT ClientRect;
                GetClientRect(WindowHandle, &ClientRect);

                CanvasUpdateWindow(DeviceCtx, ClientRect);
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
