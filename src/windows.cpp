#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")

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

#include "windows_structs.hpp"

#define MonitorRefreshRate 60
#define GameUpdateHz       (MonitorRefreshRate / 1)
#define FramesOfDelay      10

// Globals -------------------------------------------------- //
global bool              GlobalRunning;
global WinPlatBitMap     GlobalBitMap;
global WinPlatDimensions ScreenDim = {};
// Globals -------------------------------------------------- //


// XInput Shenanigans --------------------------------------------------------------------------- //

#define XINPUT_GET(name) DWORD WINAPI name(DWORD UserIndex, XINPUT_STATE* State)
typedef XINPUT_GET(xinput_get_state);
XINPUT_GET(xInputGetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
global xinput_get_state* XInputGetState_ = xInputGetStateStub;
#define XInputGetState XInputGetState_

#define XINPUT_SET(name) DWORD WINAPI name(DWORD UserIndex, XINPUT_VIBRATION* State)
typedef XINPUT_SET(xinput_set_state);
XINPUT_SET(xInputSetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
global xinput_set_state* XInputSetState_ = xInputSetStateStub;
#define XInputSetState XInputSetState_

internal void WinPlatLoadXInput() {

    HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");

    if (XInputLibrary) {
        XInputGetState = (xinput_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (xinput_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
    } else {
        OutputDebugStringA("Couldn't Load XInput\n");
    }
}
// ---------------------------------------------------------------------------------------------- //

// Loading game code ---------------------------------------------------------------------------- //

internal FILETIME WinPlatGetLastWriteTime(char* filename) {

    FILETIME last_write_time = {};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    }

    return last_write_time;
}

CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRenderStub) {}
internal WinPlatGameCode WinPlatLoadGameCode(char* source_dll_name) {

    WinPlatGameCode result   = {};
    result.update_and_render = CanvasUpdateAndRenderStub;

    char* temp_dll_name = Text("canvas_temp.dll");
    CopyFile(source_dll_name, temp_dll_name, FALSE);

    result.game_lib = LoadLibraryA(temp_dll_name);

    if (result.game_lib) {
        result.last_write_time = WinPlatGetLastWriteTime(source_dll_name);
        result.update_and_render =
            (canvas_update_and_render*)GetProcAddress(result.game_lib, "CanvasUpdateAndRender");

        result.is_valid = result.update_and_render;
    }

    if (!result.is_valid) {
        result.update_and_render = CanvasUpdateAndRenderStub;
    }

    return result;
}

internal void WinPlatFreeGameCode(WinPlatGameCode* game_code) {

    if (game_code->game_lib) {
        FreeLibrary(game_code->game_lib);
    }

    game_code->is_valid          = false;
    game_code->update_and_render = CanvasUpdateAndRenderStub;
}
// ---------------------------------------------------------------------------------------------- //

DBG_PLAT_READ_ENTIRE_FILE(DBG_PlatReadEntireFile) {
    DBG_FileStruct result = {};

    HANDLE file_handle =
        CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(file_handle, &FileSize)) {
            result.memory =
                VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (result.memory) {
                u32   FileSize32 = SafeTruncateU64(FileSize.QuadPart);
                DWORD BytesToRead;
                if (ReadFile(file_handle, result.memory, FileSize32, &BytesToRead, 0) &&
                    (FileSize32 == BytesToRead)) {
                    result.size = FileSize32;
                } else {
                    if (result.memory) {
                        VirtualFree(result.memory, 0, MEM_RELEASE);
                    }
                }
            }
        }
        CloseHandle(file_handle);
    } else {
    }

    return result;
}



DBG_PLAT_FREE_FILE_MEMORY(DBG_PlatFreeFilememory) {
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

DBG_PLAT_WRITE_ENTIRE_FILE(DBG_PlatWriteEntireFile) {

    bool result = false;

    HANDLE file_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        if (memory) {
            u32   memorySize32 = SafeTruncateU64(memory_size);
            DWORD bytes_to_write;

            if (WriteFile(file_handle, memory, memorySize32, &bytes_to_write, 0)) {
                result = true;
            } else {
                OutputDebugStringA("Couldn't Write the File\n");
            }
        } else {
            OutputDebugStringA("The memory is Null\n");
        }
        CloseHandle(file_handle);
    } else {
        OutputDebugStringA("File, not opened\n");
    }

    return result;
}



internal WinPlatDimensions WinPlatGetDimensions(HWND window_handle) {

    RECT client_rect;
    GetClientRect(window_handle, &client_rect);

    u32 width  = client_rect.right - client_rect.left;
    u32 height = client_rect.bottom - client_rect.top;

    return WinPlatDimensions{width, height};
}



internal void WinPlatCreateDibSection(u32 width, u32 height) {

    if (GlobalBitMap.memory) {
        VirtualFree(GlobalBitMap.memory, 0, MEM_RELEASE);
    }

    GlobalBitMap.width  = width;
    GlobalBitMap.height = height;

    GlobalBitMap.info.bmiHeader.biSize        = sizeof(GlobalBitMap.info.bmiHeader);
    GlobalBitMap.info.bmiHeader.biWidth       = GlobalBitMap.width;
    GlobalBitMap.info.bmiHeader.biHeight      = -(i32)GlobalBitMap.height;
    GlobalBitMap.info.bmiHeader.biPlanes      = 1;
    GlobalBitMap.info.bmiHeader.biBitCount    = 32;
    GlobalBitMap.info.bmiHeader.biCompression = BI_RGB;

    GlobalBitMap.bytes_per_pixel = 4;

    GlobalBitMap.size = GlobalBitMap.width * GlobalBitMap.height * GlobalBitMap.bytes_per_pixel;

    GlobalBitMap.memory =
        VirtualAlloc(0, GlobalBitMap.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    GlobalBitMap.pitch = GlobalBitMap.width * GlobalBitMap.bytes_per_pixel;
}



internal void WinPlatDisplayBitmap(HDC device_ctx, u32 window_width, u32 window_height) {

    u32 dest_width  = window_width;
    u32 dest_height = window_height;

    f32 screen_aspect_ratio = (f32)ScreenDim.width / (f32)ScreenDim.height;
    f32 window_aspect_ratio = (f32)window_width / (f32)window_height;

    u32 dest_y = 0;
    u32 dest_x = 0;

    if (screen_aspect_ratio >= window_aspect_ratio) {

        dest_y      = dest_height;
        dest_height = (u32)((f32)window_width / screen_aspect_ratio);
        dest_y      = (dest_y - dest_height) / 2;
    } else {

        dest_x     = dest_width;
        dest_width = (u32)((f32)window_height * screen_aspect_ratio);
        dest_x     = (dest_x - dest_width) / 2;
    }


    StretchDIBits(device_ctx, //
                  dest_x,
                  dest_y,
                  dest_width,
                  dest_height, // Destination Dimensions
                  0,
                  0,
                  GlobalBitMap.width,
                  GlobalBitMap.height, // Source Dimensions
                  GlobalBitMap.memory,
                  &GlobalBitMap.info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}


internal void
WinPlatProcessXInputButton(CanvasButtonState* prev, CanvasButtonState* next, bool is_set) {
    next->ended_down = is_set;
    next->transition += (prev->ended_down ^ next->ended_down) ? 1 : 0;
}

internal void
WinPlatProcessXInputAnalog(CanvasAnalogState* prev, CanvasAnalogState* next, f32 val) {
    next->end = next->min = next->max = val;
    next->start                       = prev->end;
}


internal void
WinPlatProcessXInput(CanvasInput* inputs, CanvasInput* old_input, CanvasInput* new_input) {

    u32 max_controller_count = XUSER_MAX_COUNT;

    if (max_controller_count > ArrayLen(inputs[0].gamepads)) {
        max_controller_count = ArrayLen(inputs[0].gamepads);
    }

    for (DWORD idx = 0; idx < max_controller_count; idx += 1) {

        XINPUT_STATE controller_state;
        if (XInputGetState(idx, &controller_state) == ERROR_SUCCESS) {

            // Unpacking of Gamepad Inputs ------------------------------------------ //
            XINPUT_GAMEPAD* pad = &controller_state.Gamepad;

            bool Up    = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down  = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left  = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

            bool Start = (pad->wButtons & XINPUT_GAMEPAD_START);
            bool Stop  = (pad->wButtons & XINPUT_GAMEPAD_BACK);

            bool LT = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            bool RT = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

            bool LS = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RS = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

            bool A = (pad->wButtons & XINPUT_GAMEPAD_A);
            bool B = (pad->wButtons & XINPUT_GAMEPAD_B);
            bool X = (pad->wButtons & XINPUT_GAMEPAD_X);
            bool Y = (pad->wButtons & XINPUT_GAMEPAD_Y);


            f32 LStickX = (f32)pad->sThumbLX;
            f32 LStickY = (f32)pad->sThumbLY;
            f32 RStickX = (f32)pad->sThumbRX;
            f32 RStickY = (f32)pad->sThumbRY;

            // Normalization of sticks
            LStickX = (LStickX < 0) ? (LStickX / 32768.0f) : (LStickX / 32767.0f);
            LStickY = (LStickY < 0) ? (LStickY / 32768.0f) : (LStickY / 32767.0f);
            RStickX = (RStickX < 0) ? (RStickX / 32768.0f) : (RStickX / 32767.0f);
            RStickY = (RStickY < 0) ? (RStickY / 32768.0f) : (RStickY / 32767.0f);

            f32 LeftTrigger  = (f32)pad->bLeftTrigger / 255.0f;
            f32 RightTrigger = (f32)pad->bRightTrigger / 255.0f;
            // ---------------------------------------------------------------------- //


            // Process Input for use ------------------------------------------------ //
            CanvasControllerInput* old_controller = &(old_input->gamepads[idx]);
            CanvasControllerInput* new_controller = &(new_input->gamepads[idx]);

            // Digital ------------------------------------------------------------------- //
            WinPlatProcessXInputButton(&old_controller->Up, &new_controller->Up, Up);
            WinPlatProcessXInputButton(&old_controller->Down, &new_controller->Down, Down);
            WinPlatProcessXInputButton(&old_controller->Left, &new_controller->Left, Left);
            WinPlatProcessXInputButton(&old_controller->Right, &new_controller->Right, Right);

            WinPlatProcessXInputButton(&old_controller->Start, &new_controller->Start, Start);
            WinPlatProcessXInputButton(&old_controller->Stop, &new_controller->Stop, Stop);

            WinPlatProcessXInputButton(&old_controller->LT, &new_controller->LT, LT);
            WinPlatProcessXInputButton(&old_controller->RT, &new_controller->RT, RT);

            WinPlatProcessXInputButton(&old_controller->LS, &new_controller->LS, LS);
            WinPlatProcessXInputButton(&old_controller->RS, &new_controller->RS, RS);

            WinPlatProcessXInputButton(&old_controller->A, &new_controller->A, A);
            WinPlatProcessXInputButton(&old_controller->B, &new_controller->B, B);
            WinPlatProcessXInputButton(&old_controller->X, &new_controller->X, X);
            WinPlatProcessXInputButton(&old_controller->Y, &new_controller->Y, Y);
            // --------------------------------------------------------------------------- //

            // Analog -------------------------------------------------------------------- //
            WinPlatProcessXInputAnalog(
                &old_controller->LeftTrigger, &new_controller->LeftTrigger, LeftTrigger);

            WinPlatProcessXInputAnalog(
                &old_controller->RightTrigger, &new_controller->RightTrigger, RightTrigger);


            WinPlatProcessXInputAnalog(
                &old_controller->LeftStickY, &new_controller->LeftStickY, LStickY);
            WinPlatProcessXInputAnalog(
                &old_controller->LeftStickX, &new_controller->LeftStickX, LStickX);

            WinPlatProcessXInputAnalog(
                &old_controller->RightStickY, &new_controller->RightStickY, RStickY);
            WinPlatProcessXInputAnalog(
                &old_controller->RightStickX, &new_controller->RightStickX, RStickX);
            // --------------------------------------------------------------------------- //



            // XINPUT_VIBRATION Vibration;
            // Vibration.wLeftMotorSpeed = 60000;
            // Vibration.wRightMotorSpeed = 60000;
            // XInputSetState(ControllerIdx, &Vibration);
        } else {

            // OutputDebugStringA("Couldn't Get GamePad State\n");
        }
    }
}

internal void WinPlatInitOpengl(HWND window_handle) {

    PIXELFORMATDESCRIPTOR pixel_format_desc = {};
    pixel_format_desc.nSize                 = sizeof(PIXELFORMATDESCRIPTOR);
    pixel_format_desc.nVersion              = 1;
    pixel_format_desc.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixel_format_desc.iPixelType = PFD_TYPE_RGBA;
    pixel_format_desc.cColorBits = 24;
    pixel_format_desc.cRedBits   = 8;
    pixel_format_desc.cGreenBits = 8;
    pixel_format_desc.cBlueBits  = 8;
    pixel_format_desc.cAlphaBits = 8;

    HDC device_ctx    = GetDC(window_handle);
    i32 pixel_fmt_idx = ChoosePixelFormat(device_ctx, &pixel_format_desc);

    PIXELFORMATDESCRIPTOR pixel_fmt_desc_final = {};
    DescribePixelFormat(
        device_ctx, pixel_fmt_idx, sizeof(PIXELFORMATDESCRIPTOR), &pixel_fmt_desc_final);

    SetPixelFormat(device_ctx, pixel_fmt_idx, &pixel_fmt_desc_final);

    HGLRC rendering_context = wglCreateContext(device_ctx);
    if (rendering_context) {
        if (wglMakeCurrent(device_ctx, rendering_context)) {

        } else {
            // TODO: what to do when it fails
        }
    }
}


internal void WinPlatDeInitOpengl(HWND window_handle) {
    HGLRC rendering_context = wglGetCurrentContext();
    if (rendering_context) {
        HDC device_ctx = wglGetCurrentDC();

        wglMakeCurrent(NULL, NULL);

        ReleaseDC(window_handle, device_ctx);

        wglDeleteContext(rendering_context);
    }
}

internal void WinPlatProcessWindowMessages(CanvasKeyboardInput* keyboard) {

    MSG  message;
    bool peeking = true;
    u64  count   = 0;
    while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE) && peeking) {
        count += 1;

        WPARAM wParam = message.wParam;
        LPARAM lParam = message.lParam;

        switch (message.message) {

            case WM_QUIT: {
                GlobalRunning = false;
                peeking       = false;
            } break;

                // case WM_SIZE: {
                //
                // } break;

            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {

                u32  VKCode   = (u32)wParam;
                bool was_down = ((lParam & (1 << 30)) != 0);
                bool is_down  = ((lParam & (1 << 31)) == 0);

                if (is_down != was_down) {
                    // Key state changed
                    keyboard->Buttons[VKCode].ended_down = is_down;
                    keyboard->Buttons[VKCode].transition++;
                }

                // Optional: Handle special cases
                if (VKCode == VK_ESCAPE && is_down) {
                    GlobalRunning = false;
                }
            } break;

            default: {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }

    char buffer[256];
    sprintf(buffer, "Message count: %lld", count);
    OutputDebugStringA(buffer);
}



internal LRESULT WinPlatWindowCallBack(HWND   window_handle,
                                       UINT   message,
                                       WPARAM wParam,
                                       LPARAM lParam) {
    LRESULT Result = 0;

    switch (message) {
        case WM_SIZE: {
        } break;

        case WM_DESTROY: {
            GlobalRunning = false;
#if OPENGL
            WinPlatDeInitOpengl(window_handle);
#else
#endif
        } break;

        case WM_CREATE: {
#if OPENGL
            WinPlatInitOpengl(window_handle);
#else
#endif
        } break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP: {
            Assert(!"Wrong Channel to get Keyboard input, should be from Message dispatches in "
                    "Main Loop");
        } break;

        case WM_CLOSE: {
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP: {
        } break;

        case WM_PAINT: {

            PAINTSTRUCT paint;
            HDC         device_ctx = BeginPaint(window_handle, &paint);

            { // Flushing the window with BLACKNESS
                i64 x      = paint.rcPaint.left;
                i64 y      = paint.rcPaint.top;
                i64 width  = paint.rcPaint.right - x;
                i64 height = paint.rcPaint.bottom - y;

                PatBlt(device_ctx, (i32)x, (i32)y, (i32)width, (i32)height, BLACKNESS);
            }

#if OPENGL
#else
            WinPlatDimensions dimensions = WinPlatGetDimensions(window_handle);
            WinPlatDisplayBitmap(device_ctx, dimensions.width, dimensions.height);
#endif

            EndPaint(window_handle, &paint);
        } break;

        default: {
            Result = DefWindowProcA(window_handle, message, wParam, lParam);
        } break;
    }

    return Result;
}



inline internal i64 WinPlatGetTime() {
    LARGE_INTEGER time_counter = {};
    QueryPerformanceCounter(&time_counter);
    return time_counter.QuadPart;
}


i32 WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd) {

    LARGE_INTEGER freq_struct_result = {};
    QueryPerformanceFrequency(&freq_struct_result);
    i64 perf_counter_frequency = freq_struct_result.QuadPart;

    UINT scedular_granularity = 1;
    bool is_time_proper       = (timeBeginPeriod(scedular_granularity) == TIMERR_NOERROR);

    ScreenDim.width  = 1920;
    ScreenDim.height = 1080;

    WinPlatLoadXInput();

#if OPENGL
#else
    WinPlatCreateDibSection(ScreenDim.width, ScreenDim.height);
#endif

    LPCSTR window_class_name = "WinPlatWindowClass";

    WNDCLASSA window_class     = {};
    window_class.style         = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = WinPlatWindowCallBack;
    window_class.hInstance     = instance;
    window_class.lpszClassName = window_class_name;

    // Refesh Rate
    f32 max_time_per_frame = 1.0f / (f32)MonitorRefreshRate;

    if (RegisterClassA(&window_class)) {
        HWND window_handle = CreateWindowExA(0,
                                             window_class_name,
                                             "Canvas",                         // Title/Caption
                                             WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Style
                                             // Position and Size
                                             CW_USEDEFAULT,
                                             CW_USEDEFAULT,
                                             CW_USEDEFAULT,
                                             CW_USEDEFAULT,
                                             0,
                                             0,
                                             instance,
                                             0);

        if (window_handle) {


            // Game Arena Allocations ---------------------------------------------------------- //
            CanvasMemory memory = {};

            memory.is_valid   = false;
            memory.perma_size = MegaBytes(64);

#ifdef DEBUG
            LPVOID base_address = (LPVOID)TeraBytes(2);
#else
            LPVOID base_address = 0;
#endif

            memory.trans_size = GigaBytes(1);

            memory.perma_store = VirtualAlloc(base_address,
                                              memory.perma_size + memory.trans_size,
                                              MEM_RESERVE | MEM_COMMIT,
                                              PAGE_READWRITE);
            memory.trans_store = (u8*)memory.perma_store + memory.perma_size;

            memory.DBG_PlatReadEntireFile  = DBG_PlatReadEntireFile;
            memory.DBG_PlatFreeFileMemory  = DBG_PlatFreeFilememory;
            memory.DBG_PlatWriteEntireFile = DBG_PlatWriteEntireFile;
            // --------------------------------------------------------------------------------- //


            GlobalRunning = (memory.perma_store && memory.trans_store);

#ifdef DEBUG
            // Perf Metrics ------------------------------------------- //
            i64 last_counter     = WinPlatGetTime();
            u64 last_cycle_count = __rdtsc();
            // -------------------------------------------------------- //
#endif

            CanvasInput  inputs[2] = {};
            CanvasInput* old_input = &inputs[0];
            CanvasInput* new_input = &inputs[1];

            char*           source_dll_name = Text("canvas.dll");
            WinPlatGameCode game_code       = WinPlatLoadGameCode(source_dll_name);

            while (GlobalRunning) {

				FILETIME new_write_time = WinPlatGetLastWriteTime(source_dll_name);
				if (CompareFileTime(&new_write_time, &game_code.last_write_time) != 0) {
					WinPlatFreeGameCode(&game_code);
					game_code = WinPlatLoadGameCode(source_dll_name);
				}

                for (u32 idx = 0; idx < ArrayLen(old_input->keyboard.Buttons); idx += 1) {
                    new_input->keyboard.Buttons[idx].ended_down =
                        old_input->keyboard.Buttons[idx].ended_down;
                }

                WinPlatProcessWindowMessages(&new_input->keyboard);



                WinPlatProcessXInput(inputs, old_input, new_input);

#if OPENGL
#else

                CanvasBitMap bitmap    = {};
                bitmap.memory          = GlobalBitMap.memory;
                bitmap.width           = GlobalBitMap.width;
                bitmap.height          = GlobalBitMap.height;
                bitmap.size            = GlobalBitMap.size;
                bitmap.pitch           = GlobalBitMap.pitch;
                bitmap.bytes_per_pixel = GlobalBitMap.bytes_per_pixel;

                game_code.update_and_render(&memory, &bitmap, new_input, &GlobalRunning);

                // Drawing the Bitmap -------------------------------------------- //
                HDC               device_ctx  = GetDC(window_handle);
                WinPlatDimensions dimenstions = WinPlatGetDimensions(window_handle);

                WinPlatDisplayBitmap(device_ctx, dimenstions.width, dimenstions.height);
                ZeroMemory(bitmap.memory, bitmap.size);

                ReleaseDC(window_handle, device_ctx);
                // --------------------------------------------------------------- //

#endif

                // Profiling Stuff ----------------------------------------------- //
                u64 end_cycle_count = __rdtsc();

                i64 end_counter = WinPlatGetTime();

                f64 mega_cylces_elapsed =
                    (((f64)end_cycle_count - (f64)last_cycle_count) / (1000.0f * 1000.0f));

                i64 counter_elapsed = end_counter - last_counter;

                f32 time_elapsed_for_frame = (f32)counter_elapsed / (f32)perf_counter_frequency;

                if (time_elapsed_for_frame < max_time_per_frame) {
                    if (is_time_proper) {
                        DWORD SleepTime =
                            (DWORD)(1000.0f * (max_time_per_frame - time_elapsed_for_frame));
                        Sleep(SleepTime);
                    }
                    while (time_elapsed_for_frame < max_time_per_frame) {
                        counter_elapsed        = WinPlatGetTime() - last_counter;
                        time_elapsed_for_frame = (f32)counter_elapsed / (f32)perf_counter_frequency;
                    }
                } else {
                }


                i64 temp         = last_counter;
                last_counter     = WinPlatGetTime();
                last_cycle_count = end_cycle_count;

#ifdef DEBUG
                counter_elapsed   = WinPlatGetTime() - temp;
                f64  ms_per_frame = (1000.0f * (f64)counter_elapsed) / (f64)perf_counter_frequency;
                f64  fps          = 1000.0f / ms_per_frame;
                char buffer[256];

                sprintf(buffer,
                        "%.03fms, %.03ffps, %.03fMC/F \n",
                        ms_per_frame,
                        fps,
                        mega_cylces_elapsed);
                // OutputDebugStringA(Buffer);
#endif
                // --------------------------------------------------------------- //

                CanvasInput* t = old_input;
                old_input      = new_input;
                new_input      = t;
            }
        } else {
            OutputDebugStringA("Failed at creation of window handle.\n");
        }
    } else {
        OutputDebugStringA("Registering the window class failed.\n");
    }


    return 0;
}
