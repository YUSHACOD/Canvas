#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")


/*
 * TODO: Seperating the Platform layer
 *
 * Next ->
 * - helpers to create projection / model-view matrix
 * - Bug: input clear when window is not focused
 * - Buffering projection, model-view mats
 * - world limit definition
 * - debug camera (that means first have to implement quaternions and other rotation mechs)
 * - maze generation from cubes
 *
 *
 *
 * - Save Location
 * - Getting the Handle to our own executable???
 * - Threading
* - Raw Input
 * - Sleep / TimeBeginPeriod
 * - WM_SETCURSOR
 *
 * - QueryCancelAutoPlay
 * - WM_ACTIVATEAPP
 * - Blt speed improvements
 *
 */

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <xinput.h>

#include "windows.hpp"
#include "canvas_platform.hpp"
#include "renderer.hpp"
#include "opengl.hpp"

#include "opengl.cpp"


//  globals : ------------------------------------------------------------------------ (section)  //
#define MAX_CANVAS_PATH 1028
#define WINDOW_STYLE    (WS_TILEDWINDOW & ~WS_MINIMIZEBOX)

// Predefined
global char* GLBL_game_dll_name = Text("canvas.dll");

// Infered at runtime
global HDC                       GLBL_device_ctx;
global winplat_dimensions        GLBL_display_size;
global f32                       GLBL_aspect_ratio;
global winplat_off_screen_buffer GLBL_offscreen_buffer;
global bool                      GLBL_is_running;
global WINDOWPLACEMENT           GLBL_prev_wnd_placement;

// TODO: Special Path Handling Check
// Am I being piggy here?
global char GLBL_module_path[MAX_CANVAS_PATH];
global i32  GLBL_module_path_len;

internal void LoadModulePath(char* path) {
    for (i32 i = GLBL_module_path_len; i < MAX_CANVAS_PATH; i += 1) {
        if (path[i - GLBL_module_path_len] == 0) {
            break;
        }
        GLBL_module_path[i] = path[i - GLBL_module_path_len];
    }
}

internal void UnloadModulePath() { GLBL_module_path[GLBL_module_path_len] = '\0'; }
//  (section) ------------------------------------------------------------------------ : globals  //

//  xinput loading : ----------------------------------------------------------------- (section)  //
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
//  (section) ----------------------------------------------------------------- : xinput loading  //


//  game code loading : -------------------------------------------------------------- (section)  //
internal FILETIME WinPlatGetLastWriteTime(char* filename) {

    FILETIME last_write_time = {};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    }

    return last_write_time;
}

CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRenderStub) {}
internal winplat_game_code WinPlatLoadGameCode(char* source_dll_path) {

    winplat_game_code result = {};
    result.update_and_draw   = CanvasUpdateAndRenderStub;

#if DEBUG
    char temp_dll_path[MAX_CANVAS_PATH] = {0};
    i32  temp_idx;
    for (temp_idx = 0; temp_idx < GLBL_module_path_len; temp_idx += 1) {
        temp_dll_path[temp_idx] = GLBL_module_path[temp_idx];
    }
    char temp_suffix[] = "temp_canvas.dll";
    for (i32 i = 0; temp_suffix[i] != '\0'; i += 1) {
        temp_dll_path[temp_idx++] = temp_suffix[i];
    }

    CopyFileA(source_dll_path, temp_dll_path, FALSE);
    result.game_lib = LoadLibraryA(temp_dll_path);
#else
    result.game_lib = LoadLibraryA(source_dll_path);
#endif

    if (result.game_lib) {
        result.last_write_time = WinPlatGetLastWriteTime(source_dll_path);
        result.update_and_draw =
            (canvas_update_and_draw*)GetProcAddress(result.game_lib, "CanvasUpdateAndRender");

        result.is_valid = result.update_and_draw;
    }

    if (!result.is_valid) {
        result.update_and_draw = CanvasUpdateAndRenderStub;
    }

    return result;
}

internal void WinPlatFreeGameCode(winplat_game_code* game_code) {

    if (game_code->game_lib) {
        FreeLibrary(game_code->game_lib);
    }

    game_code->is_valid        = false;
    game_code->update_and_draw = CanvasUpdateAndRenderStub;
}
//  (section) -------------------------------------------------------------- : game code loading  //

//  winplat helpers : ---------------------------------------------------------------- (section)  //
inline internal i64 WinPlatGetTime() {
    LARGE_INTEGER time_counter = {};
    QueryPerformanceCounter(&time_counter);
    return time_counter.QuadPart;
}

inline internal void WinPlatTimeQuery(winplat_time_counter* q) {
    q->counter     = WinPlatGetTime();
    q->cycle_count = __rdtsc();
}

internal winplat_dimensions WinPlatGetDimensions(HWND window_handle) {

    RECT client_rect;
    GetClientRect(window_handle, &client_rect);

    u32 width  = client_rect.right - client_rect.left;
    u32 height = client_rect.bottom - client_rect.top;

    return winplat_dimensions{width, height};
}
//  (section) ---------------------------------------------------------------- : winplat helpers  //


//  renderer helpers : --------------------------------------------------------------- (section)  //
RNDR_ALLOCATE_PUSH_BUFFER(AllocatePushBuffer) {
    push_buffer->cube_buffer.cubes =
        (RC_cube*)VirtualAlloc(0,
                               sizeof(RC_cube) * push_buffer->cube_buffer.size,
                               MEM_RESERVE | MEM_COMMIT,
                               PAGE_READWRITE);

    push_buffer->cube_wf_buffer.cubes =
        (RC_cube_wf*)VirtualAlloc(0,
                                  sizeof(RC_cube_wf) * push_buffer->cube_wf_buffer.size,
                                  MEM_RESERVE | MEM_COMMIT,
                                  PAGE_READWRITE);
}

RNDR_CLEAR_PUSH_BUFFER(ClearPushBuffer) {
    push_buffer->cube_buffer.count    = 0;
    push_buffer->cube_wf_buffer.count = 0;
}
//  (section) --------------------------------------------------------------- : renderer helpers  //


//  bitmap : ------------------------------------------------------------------------- (section)  //
internal void WinPlatCreateDibSection(winplat_off_screen_buffer* bitmap,
                                      winplat_dimensions         display_dim) {

    if (bitmap->memory) {
        VirtualFree(bitmap->memory, 0, MEM_RELEASE);
    }

    bitmap->width  = display_dim.width;
    bitmap->height = display_dim.height;

    bitmap->info.bmiHeader.biSize        = sizeof(bitmap->info.bmiHeader);
    bitmap->info.bmiHeader.biWidth       = bitmap->width;
    bitmap->info.bmiHeader.biHeight      = -(i32)bitmap->height;
    bitmap->info.bmiHeader.biPlanes      = 1;
    bitmap->info.bmiHeader.biBitCount    = 32;
    bitmap->info.bmiHeader.biCompression = BI_RGB;

    bitmap->bytes_per_pixel = 4;

    bitmap->size = bitmap->width * bitmap->height * bitmap->bytes_per_pixel;

    bitmap->memory = VirtualAlloc(0, bitmap->size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    bitmap->pitch = bitmap->width * bitmap->bytes_per_pixel;
}


internal void WinPlatDisplayBitmap(HDC                        device_ctx,
                                   winplat_off_screen_buffer* bitmap,
                                   winplat_dimensions         window_size,
                                   winplat_dimensions         display_size,
                                   f32                        display_aspect_ratio) {

    u32 dest_width  = window_size.width;
    u32 dest_height = window_size.height;

    f32 window_aspect_ratio = (f32)window_size.width / (f32)window_size.height;

    u32 dest_y = 0;
    u32 dest_x = 0;

    if (display_aspect_ratio >= window_aspect_ratio) {

        dest_y      = dest_height;
        dest_height = (u32)((f32)window_size.width / display_aspect_ratio);
        dest_y      = (dest_y - dest_height) / 2;
    } else {

        dest_x     = dest_width;
        dest_width = (u32)((f32)window_size.height * display_aspect_ratio);
        dest_x     = (dest_x - dest_width) / 2;
    }


    StretchDIBits(device_ctx, //
                  dest_x,
                  dest_y,
                  dest_width,
                  dest_height, // Destination Dimensions
                  0,
                  0,
                  bitmap->width,
                  bitmap->height, // Source Dimensions
                  bitmap->memory,
                  &bitmap->info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}
//  (section) ------------------------------------------------------------------------- : bitmap  //


//  xinput processing : -------------------------------------------------------------- (section)  //
internal void
WinPlatProcessXInputButton(canvas_button_state* prev, canvas_button_state* next, bool is_set) {
    next->ended_down = is_set;
    next->transition += (prev->ended_down ^ next->ended_down) ? 1 : 0;
}

internal void
WinPlatProcessXInputAnalog(canvas_analog_state* prev, canvas_analog_state* next, f32 val) {
    next->end = next->min = next->max = val;
    next->start                       = prev->end;
}


internal void
WinPlatProcessXInput(canvas_input* inputs, canvas_input* old_input, canvas_input* new_input) {

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
            canvas_controller_input* old_controller = &(old_input->gamepads[idx]);
            canvas_controller_input* new_controller = &(new_input->gamepads[idx]);

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
//  (section) -------------------------------------------------------------- : xinput processing  //


//  wm message processing : ---------------------------------------------------------- (section)  //

// Raymond Cheng toggle fullscreen function
internal void ToggleFullScreen(HWND window_handle) {

    DWORD window_style = GetWindowLong(window_handle, GWL_STYLE);

    if (window_style & WINDOW_STYLE) {

        MONITORINFO monitor_info = {sizeof(monitor_info)};

        if (GetWindowPlacement(window_handle, &GLBL_prev_wnd_placement) &&
            GetMonitorInfo(MonitorFromWindow(window_handle, MONITOR_DEFAULTTOPRIMARY),
                           &monitor_info)) {
            SetWindowLong(window_handle, GWL_STYLE, window_style & ~WINDOW_STYLE);
            SetWindowPos(window_handle,
                         HWND_TOP,
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(window_handle, GWL_STYLE, window_style | WINDOW_STYLE);
        SetWindowPlacement(window_handle, &GLBL_prev_wnd_placement);
        SetWindowPos(window_handle,
                     NULL,
                     0,
                     0,
                     0,
                     0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal void WinPlatProcessWindowMessages(canvas_keyboard_input* keyboard,
                                           HWND                   window_handle,
                                           bool*                  is_running) {

    MSG message;
    while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE) && *is_running) {

        WPARAM wParam = message.wParam;
        LPARAM lParam = message.lParam;

        switch (message.message) {

            case WM_QUIT: {
                *is_running = false;
            } break;


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
                if (keyboard->Escape.ended_down) {
                    *is_running = false;
                }

                if (keyboard->F11.ended_down) {
                    ToggleFullScreen(window_handle);
                }
            } break;

            default: {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }
}



internal LRESULT WinPlatWindowCallBack(HWND   window_handle,
                                       UINT   message,
                                       WPARAM wParam,
                                       LPARAM lParam) {

    LRESULT result = 0;

    switch (message) {

        case WM_CREATE: {
#if OPENGL
            GLInit(window_handle);
#endif
        } break;

        case WM_DESTROY: {
            GLBL_is_running = false;
        } break;

        case WM_CLOSE: {
            GLBL_is_running = false;
        } break;

        case WM_ACTIVATEAPP: {
        } break;

        case WM_SYSCOMMAND: {
            if (wParam == SC_MAXIMIZE) {
                ToggleFullScreen(window_handle);
            }
        } break;

        case WM_SIZE: {

#if OPENGL
            if (GLBL_opengl_state.is_valid) {
                winplat_dimensions dim = WinPlatGetDimensions(window_handle);
                GLFixProjection(&GLBL_opengl_state, dim, GLBL_display_size, GLBL_aspect_ratio);
            }
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


        case WM_PAINT: {
            PAINTSTRUCT paint;
            BeginPaint(window_handle, &paint);
            // Todo: Just render here, this flushing should not occur
            { // Flushing the window with BLACKNESS
                i64 x      = paint.rcPaint.left;
                i64 y      = paint.rcPaint.top;
                i64 width  = paint.rcPaint.right - x;
                i64 height = paint.rcPaint.bottom - y;

                PatBlt(GLBL_device_ctx, (i32)x, (i32)y, (i32)width, (i32)height, BLACKNESS);
            }
            EndPaint(window_handle, &paint);
        } break;


#if 0 // This is for having a no border window, there is a top bar with WS_POPUP and WS_THICKFRAME
        case WM_NCCALCSIZE: {
            // This is just for top-border bug for non border resizable window
            // Picked up from stackoverflow
            if (wParam) {
                /* Detect whether window is maximized or not. We don't need to change the resize
                 * border when win is maximized because all resize borders are gone automatically */
                WINDOWPLACEMENT wPos;
                // GetWindowPlacement fail if this member is not set correctly.
                wPos.length = sizeof(wPos);
                GetWindowPlacement(window_handle, &wPos);
                if (wPos.showCmd != SW_SHOWMAXIMIZED) {
                    RECT borderThickness;
                    SetRectEmpty(&borderThickness);
                    AdjustWindowRectEx(&borderThickness,
                                       GetWindowLongPtr(window_handle, GWL_STYLE) & ~WS_CAPTION,
                                       FALSE,
                                       NULL);
                    borderThickness.left *= -1;
                    borderThickness.top *= -1;
                    NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)(lParam);
                    sz->rgrc[0].top += 0;
                    sz->rgrc[0].left += borderThickness.left;
                    sz->rgrc[0].right -= borderThickness.right;
                    sz->rgrc[0].bottom -= borderThickness.bottom;
                    return 0;
                }
            }
        }
#endif


        default: {
            result = DefWindowProcA(window_handle, message, wParam, lParam);
        } break;
    }

    return result;
}
//  (section) ---------------------------------------------------------- : wm message processing  //



//  main entry point : --------------------------------------------------------------- (section)  //
i32 WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd) {


    GLBL_module_path_len = GetCurrentDirectoryA(MAX_CANVAS_PATH, GLBL_module_path);
    GLBL_module_path[GLBL_module_path_len++] = '\\';
    Assert(GLBL_module_path_len != 0);

    // Frequency
    LARGE_INTEGER freq_struct_result = {};
    QueryPerformanceFrequency(&freq_struct_result);
    i64 perf_counter_freq = freq_struct_result.QuadPart;

    // Time Validity
#define SCHEDULAR_GRANULARITY 1
    bool is_time_proper = (timeBeginPeriod(SCHEDULAR_GRANULARITY) == TIMERR_NOERROR);

    // Frame Timing
    DEVMODEA device_mode = {};
    EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &device_mode);
    u64 refresh_rate       = (u64)device_mode.dmDisplayFrequency;
    f64 max_time_per_frame = 1.0f / (f64)refresh_rate;

    WinPlatLoadXInput();

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    LPCSTR    window_class_name = "WinPlatWindowClass";
    WNDCLASSA window_class      = {};
    window_class.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc    = WinPlatWindowCallBack;
    window_class.hInstance      = instance;
    window_class.lpszClassName  = window_class_name;


    if (RegisterClassA(&window_class)) {

        HWND window_handle = CreateWindowExA(0,
                                             window_class_name,
                                             "Canvas",
                                             WINDOW_STYLE,
                                             CW_USEDEFAULT,
                                             CW_USEDEFAULT,
                                             1280,
                                             720,
                                             0,
                                             0,
                                             instance,
                                             0);

        if (window_handle) {

            HMONITOR    monitor = MonitorFromWindow(window_handle, MONITOR_DEFAULTTONEAREST);
            MONITORINFO info;
            info.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(monitor, &info);
            GLBL_display_size.width  = info.rcMonitor.right - info.rcMonitor.left;
            GLBL_display_size.height = info.rcMonitor.bottom - info.rcMonitor.top;
            GLBL_aspect_ratio        = (f32)GLBL_display_size.width / (f32)GLBL_display_size.height;

            GLBL_device_ctx = GetDC(window_handle);

            //  opengl pipeline setup : ---------------------------------------------- (section)  //
#if OPENGL

            GLPipeLineSetup(&GLBL_opengl_state, GLBL_aspect_ratio, 1);

            glEnable(GL_SCISSOR_TEST);

            winplat_dimensions dim = WinPlatGetDimensions(window_handle);
            GLFixProjection(&GLBL_opengl_state, dim, GLBL_display_size, GLBL_aspect_ratio);
#else
            WinPlatCreateDibSection(&Global_OffScreenBuffer, Global_DiplaySize);
            canvas_bitmap bitmap   = {};
            bitmap.memory          = GLBL_offscreen_buffer.memory;
            bitmap.width           = GLBL_offscreen_buffer.width;
            bitmap.height          = GLBL_offscreen_buffer.height;
            bitmap.size            = GLBL_offscreen_buffer.size;
            bitmap.pitch           = GLBL_offscreen_buffer.pitch;
            bitmap.bytes_per_pixel = GLBL_offscreen_buffer.bytes_per_pixel;
#endif


            //  game memory allocations : -------------------------------------------- (section)  //
            canvas_memory memory = {};

            memory.is_valid      = false;
            memory.perma_size    = MegaBytes(64);
#ifdef DEBUG
            LPVOID base_address = (LPVOID)TeraBytes(2);
#else
            LPVOID base_address = 0;
#endif
            memory.trans_size  = GigaBytes(1);
            memory.perma_store = VirtualAlloc(base_address,
                                              memory.perma_size + memory.trans_size,
                                              MEM_RESERVE | MEM_COMMIT,
                                              PAGE_READWRITE);
            memory.trans_store = (u8*)memory.perma_store + memory.perma_size;
#ifdef DEBUG
            memory.DBG_PlatReadEntireFile  = DBG_PlatReadEntireFile;
            memory.DBG_PlatFreeFileMemory  = DBG_PlatFreeFilememory;
            memory.DBG_PlatWriteEntireFile = DBG_PlatWriteEntireFile;

#endif
            render_push_buffer r_push_buffer  = {};
            r_push_buffer.cube_buffer.size    = 200;
            r_push_buffer.cube_wf_buffer.size = 200;
            AllocatePushBuffer(&r_push_buffer);

            // If arena is valid and nothing crashed until now then run
            GLBL_is_running = (memory.perma_store && memory.trans_store);

            //  input init : --------------------------------------------------------- (section)  //
            canvas_input  inputs[2] = {};
            canvas_input* old_input = &inputs[0];
            canvas_input* new_input = &inputs[1];

            //  gamecode init : ------------------------------------------------------ (section)  //
            winplat_game_code game_code;
            DeferLoop(LoadModulePath(GLBL_game_dll_name), UnloadModulePath()) {
                game_code = WinPlatLoadGameCode(GLBL_module_path);
            }

            bool window_shown = false;
            ShowCursor(false);


			//  timing init : -------------------------------------------------------- (section)  //
            winplat_time_counter last = {};
            WinPlatTimeQuery(&last);


            f64 time_elapsed = 0.0f;

            //  main loop : ---------------------------------------------------------- (section)  //
            while (GLBL_is_running) {

#ifdef DEBUG
                FILETIME new_write_time = WinPlatGetLastWriteTime(GLBL_game_dll_name);
                if (CompareFileTime(&new_write_time, &game_code.last_write_time) != 0) {
                    WinPlatFreeGameCode(&game_code);
                    game_code = WinPlatLoadGameCode(GLBL_game_dll_name);
                }
#endif

                // Input Processing
                for (u32 idx = 0; idx < ArrayLen(old_input->keyboard.Buttons); idx += 1) {
                    new_input->keyboard.Buttons[idx].ended_down =
                        old_input->keyboard.Buttons[idx].ended_down;
                }
                WinPlatProcessWindowMessages(&new_input->keyboard, window_handle, &GLBL_is_running);
                WinPlatProcessXInput(inputs, old_input, new_input);


				//  game layer call : ------------------------------------------------ (section)  //
                game_code.update_and_draw(
                    &memory, &r_push_buffer, new_input, time_elapsed, &GLBL_is_running);


                Render(r_push_buffer);

                ClearPushBuffer(&r_push_buffer);

                // Double buffering input state
                Swap(canvas_input*, old_input, new_input);
                SwapBuffers(GLBL_device_ctx);

				//  timing : --------------------------------------------------------- (section)  //
                winplat_time_counter end = {};
                WinPlatTimeQuery(&end);


                f64 mega_cylces_elapsed =
                    (((f64)end.cycle_count - (f64)last.cycle_count) / (1000.0f * 1000.0f));

                i64 counter_elapsed = end.counter - last.counter;

                f64 time_elapsed_for_frame = (f64)counter_elapsed / (f64)perf_counter_freq;

#if 0
#define THIRTY_FPS (1.0f / 30.f)
                if (time_elapsed_for_frame > THIRTY_FPS) {
                    OutputDebugStringA("======== TOO SLOW ========");
                    Assert(0);
                }
#endif

                // TODO(Timing): this is a mess
                if (time_elapsed_for_frame <= max_time_per_frame) {
                    if (is_time_proper) {
                        DWORD SleepTime =
                            (DWORD)(1000.0f * (max_time_per_frame - time_elapsed_for_frame));
                        Sleep(SleepTime);
                    }
                    while (time_elapsed_for_frame < max_time_per_frame) {
                        counter_elapsed        = WinPlatGetTime() - last.counter;
                        time_elapsed_for_frame = (f32)counter_elapsed / (f32)perf_counter_freq;
                    }
                } else {
                }

                i64 temp         = last.counter;
                last.counter     = WinPlatGetTime();
                last.cycle_count = end.cycle_count;
#ifdef DEBUG
                counter_elapsed  = WinPlatGetTime() - temp;
                f64 ms_per_frame = (1000.0f * (f64)counter_elapsed) / (f64)perf_counter_freq;
                time_elapsed += ms_per_frame;
                f64  fps = 1000.0f / ms_per_frame;
                char buffer[256];

                sprintf(buffer,
                        "%.03fms, %.03ffps, %.03fMC/F \n",
                        ms_per_frame,
                        fps,
                        mega_cylces_elapsed);
                // OutputDebugStringA(buffer);
#endif

				// showing window after one frame is painted
                if (!window_shown) {
                    ShowWindow(window_handle, show_cmd);
                    window_shown = true;
                }
            }

            //  cleanup : ------------------------------------------------------------ (section)  //
#if OPENGL
            GLDeInit(window_handle);
            GlPipelineDelete(&GLBL_opengl_state);


#else
#endif
        } else {
            OutputDebugStringA("Failed at creation of window handle.\n");
        }
    } else {
        OutputDebugStringA("Registering the window class failed.\n");
    }

    return 0;
}
//  (section) --------------------------------------------------------------- : main entry point  //
