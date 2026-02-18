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
#include <math.h>


#include "windows_structs.hpp"
#include "windows_opengl.cpp"


// Globals -------------------------------------------------------------------------------------- //
global WinPlatMainContext MainCtx;
// Globals -------------------------------------------------------------------------------------- //


// Loading XInput ------------------------------------------------------------------------------- //
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
// Loading XInput ------------------------------------------------------------------------------- //


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
// Loading game code ---------------------------------------------------------------------------- //


// Debug File IO -------------------------------------------------------------------------------- //
DBG_PLAT_READ_ENTIRE_FILE(DBG_PlatReadEntireFile) {
    DBG_FileStruct result = {};

    HANDLE file_handle =
        CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(file_handle, &FileSize)) {
            result.memory =
                VirtualAlloc(0, FileSize.QuadPart + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (result.memory) {
                u32   FileSize32 = SafeTruncateU64(FileSize.QuadPart);
                DWORD BytesToRead;
                if (ReadFile(file_handle, result.memory, FileSize32, &BytesToRead, 0) &&
                    (FileSize32 == BytesToRead)) {
                    result.size                         = FileSize32;
                    ((char*)result.memory)[result.size] = '\0';
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
// Debug File IO -------------------------------------------------------------------------------- //


// WinPlat Helpers ------------------------------------------------------------------------------ //
inline internal i64 WinPlatGetTime() {
    LARGE_INTEGER time_counter = {};
    QueryPerformanceCounter(&time_counter);
    return time_counter.QuadPart;
}

internal WinPlatDimensions WinPlatGetDimensions(HWND window_handle) {

    RECT client_rect;
    GetClientRect(window_handle, &client_rect);

    u32 width  = client_rect.right - client_rect.left;
    u32 height = client_rect.bottom - client_rect.top;

    return WinPlatDimensions{width, height};
}
// WinPlat Helpers ------------------------------------------------------------------------------ //


// Software Renderer Helpers -------------------------------------------------------------------- //
internal void WinPlatCreateDibSection(u32 width, u32 height) {

    if (MainCtx.bitmap.memory) {
        VirtualFree(MainCtx.bitmap.memory, 0, MEM_RELEASE);
    }

    MainCtx.bitmap.width  = width;
    MainCtx.bitmap.height = height;

    MainCtx.bitmap.info.bmiHeader.biSize        = sizeof(MainCtx.bitmap.info.bmiHeader);
    MainCtx.bitmap.info.bmiHeader.biWidth       = MainCtx.bitmap.width;
    MainCtx.bitmap.info.bmiHeader.biHeight      = -(i32)MainCtx.bitmap.height;
    MainCtx.bitmap.info.bmiHeader.biPlanes      = 1;
    MainCtx.bitmap.info.bmiHeader.biBitCount    = 32;
    MainCtx.bitmap.info.bmiHeader.biCompression = BI_RGB;

    MainCtx.bitmap.bytes_per_pixel = 4;

    MainCtx.bitmap.size =
        MainCtx.bitmap.width * MainCtx.bitmap.height * MainCtx.bitmap.bytes_per_pixel;

    MainCtx.bitmap.memory =
        VirtualAlloc(0, MainCtx.bitmap.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    MainCtx.bitmap.pitch = MainCtx.bitmap.width * MainCtx.bitmap.bytes_per_pixel;
}



internal void WinPlatDisplayBitmap(HDC device_ctx, u32 window_width, u32 window_height) {

    u32 dest_width  = window_width;
    u32 dest_height = window_height;


    f32 window_aspect_ratio = (f32)window_width / (f32)window_height;

    u32 dest_y = 0;
    u32 dest_x = 0;

    if (MainCtx.aspect_ratio >= window_aspect_ratio) {

        dest_y      = dest_height;
        dest_height = (u32)((f32)window_width / MainCtx.aspect_ratio);
        dest_y      = (dest_y - dest_height) / 2;
    } else {

        dest_x     = dest_width;
        dest_width = (u32)((f32)window_height * MainCtx.aspect_ratio);
        dest_x     = (dest_x - dest_width) / 2;
    }


    StretchDIBits(device_ctx, //
                  dest_x,
                  dest_y,
                  dest_width,
                  dest_height, // Destination Dimensions
                  0,
                  0,
                  MainCtx.bitmap.width,
                  MainCtx.bitmap.height, // Source Dimensions
                  MainCtx.bitmap.memory,
                  &MainCtx.bitmap.info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}
// Software Renderer Helpers -------------------------------------------------------------------- //


// XInput Processing ---------------------------------------------------------------------------- //
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
// XInput Processing ---------------------------------------------------------------------------- //


// Windows Message Processing ------------------------------------------------------------------- //
internal void WinPlatProcessWindowMessages(CanvasKeyboardInput* keyboard) {

    MSG message;
    while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE) && MainCtx.is_running) {

        WPARAM wParam = message.wParam;
        LPARAM lParam = message.lParam;

        switch (message.message) {

            case WM_QUIT: {
                MainCtx.is_running = false;
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
                if (VKCode == VK_ESCAPE && is_down) {
                    MainCtx.is_running = false;
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

    LRESULT Result = 0;

    switch (message) {

        case WM_CREATE: {
#if OPENGL
            GLInit(window_handle);
#else
#endif
        } break;

        case WM_DESTROY: {
            MainCtx.is_running = false;
#if OPENGL
            GLDeInit(window_handle);
#else
#endif
        } break;

        case WM_SIZE: {
#if OPENGL
            if (MainCtx.gl_state != 0) {
                WinPlatDimensions dim = WinPlatGetDimensions(window_handle);
                GLFixProjection(
                    MainCtx.gl_state, dim, MainCtx.screen_dimensions, MainCtx.aspect_ratio);
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

        case WM_CLOSE: {
            MainCtx.is_running = false;
        } break;

        case WM_ACTIVATEAPP: {
        } break;

        case WM_PAINT: {

            PAINTSTRUCT paint;
            BeginPaint(window_handle, &paint);
#if OPENGL
#else
            { // Flushing the window with BLACKNESS
                i64 x      = paint.rcPaint.left;
                i64 y      = paint.rcPaint.top;
                i64 width  = paint.rcPaint.right - x;
                i64 height = paint.rcPaint.bottom - y;

                PatBlt(MainCtx.device_ctx, (i32)x, (i32)y, (i32)width, (i32)height, BLACKNESS);
            }
#endif
            EndPaint(window_handle, &paint);
        } break;

        default: {
            Result = DefWindowProcA(window_handle, message, wParam, lParam);
        } break;
    }

    return Result;
}
// Windows Message Processing ------------------------------------------------------------------- //



// Main Windows Entry Point --------------------------------------------------------------------- //
i32 WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd) {

    LARGE_INTEGER freq_struct_result = {};
    QueryPerformanceFrequency(&freq_struct_result);
    i64 perf_counter_frequency = freq_struct_result.QuadPart;

    UINT scedular_granularity = 1;
    bool is_time_proper       = (timeBeginPeriod(scedular_granularity) == TIMERR_NOERROR);

    MainCtx.screen_dimensions.width  = 1920;
    MainCtx.screen_dimensions.height = 1080;
    MainCtx.aspect_ratio =
        (f32)MainCtx.screen_dimensions.width / (f32)MainCtx.screen_dimensions.height;

    DEVMODEA device_mode = {};
    EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &device_mode);
    MainCtx.refresh_rate = (u64)device_mode.dmDisplayFrequency;

    WinPlatLoadXInput();

#if OPENGL
#else
    WinPlatCreateDibSection(MainCtx.screen_dimensions.width, MainCtx.screen_dimensions.height);
#endif

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    LPCSTR window_class_name = "WinPlatWindowClass";

    WNDCLASSA window_class     = {};
    window_class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc   = WinPlatWindowCallBack;
    window_class.hInstance     = instance;
    window_class.lpszClassName = window_class_name;

    // Refesh Rate
    f64 max_time_per_frame = 1.0f / (f64)MainCtx.refresh_rate;

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

            MainCtx.device_ctx = GetDC(window_handle);

            // Game Arena Allocations
            CanvasMemory memory = {};
            memory.is_valid     = false;
            memory.perma_size   = MegaBytes(64);
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

            MainCtx.is_running = (memory.perma_store && memory.trans_store);


            // Timing Init
            i64 last_counter     = WinPlatGetTime();
            u64 last_cycle_count = __rdtsc();
            f64 time_elapsed     = 0.0f;

            // Input Init
            CanvasInput  inputs[2] = {};
            CanvasInput* old_input = &inputs[0];
            CanvasInput* new_input = &inputs[1];

            // Gamecode Init
            char*           source_dll_name = Text("canvas.dll");
            WinPlatGameCode game_code       = WinPlatLoadGameCode(source_dll_name);

            // Opengl Pipeline Setup
#if OPENGL
            GLPipelineState gl_state = {};
            gl_state.vao_len         = 1;

            DBG_FileStruct vertex_source = DBG_PlatReadEntireFile(Text("../../src/vertex.glsl"));
            DBG_FileStruct fragment_source =
                DBG_PlatReadEntireFile(Text("../../src/fragment.glsl"));

            GLPipeLineSetup(&gl_state,
                            (char*)vertex_source.memory,
                            (char*)fragment_source.memory,
                            MainCtx.aspect_ratio);

            MainCtx.gl_state = &gl_state;
            glEnable(GL_SCISSOR_TEST);

            WinPlatDimensions dim = WinPlatGetDimensions(window_handle);
            GLFixProjection(&gl_state, dim, MainCtx.screen_dimensions, MainCtx.aspect_ratio);
            GLfloat input_pos[4] = {0};
#else
            CanvasBitMap bitmap    = {};
            bitmap.memory          = MainCtx.bitmap.memory;
            bitmap.width           = MainCtx.bitmap.width;
            bitmap.height          = MainCtx.bitmap.height;
            bitmap.size            = MainCtx.bitmap.size;
            bitmap.pitch           = MainCtx.bitmap.pitch;
            bitmap.bytes_per_pixel = MainCtx.bitmap.bytes_per_pixel;
#endif

            // Main Loop ------------------------------------------------------------------------ //
            while (MainCtx.is_running) {

                // Input Processing
                for (u32 idx = 0; idx < ArrayLen(old_input->keyboard.Buttons); idx += 1) {
                    new_input->keyboard.Buttons[idx].ended_down =
                        old_input->keyboard.Buttons[idx].ended_down;
                }
                WinPlatProcessWindowMessages(&new_input->keyboard);
                WinPlatProcessXInput(inputs, old_input, new_input);

#if OPENGL
                // Clear the buffer
                glDisable(GL_SCISSOR_TEST);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable(GL_SCISSOR_TEST);

                f64 dt = time_elapsed * 0.001f;

                // GLfloat color[] = {
                //     (f32)sin(dt) * 0.5f + 0.5f, 0.0f, (f32)cos(dt) * 0.5f + 0.5f, 1.0f};

                GLfloat color[] = {1.0f, 1.0f, 1.0f, 1.0f};
                glClearBufferfv(GL_COLOR, 0, color);

                glUseProgram(gl_state.program_handle);

                GLfloat offset[] = {-0.125, -0.125, 0.0f, 0.0f};
                glVertexAttrib4fv(0, offset);

                GLfloat color_attrib[] = {1.0f, 1.0f, 1.0f, 1.0f};
                glVertexAttrib4fv(1, color_attrib);

                GLfloat rot2D[] = {(f32)cos(dt) * 0.5f, (f32)sin(dt) * 0.5f, 0.0f, 0.0f};
                glVertexAttrib4fv(2, rot2D);

                input_pos[0] += (new_input->keyboard.D.ended_down) ? 0.01f : 0.0f;
                input_pos[0] -= (new_input->keyboard.A.ended_down) ? 0.01f : 0.0f;

                input_pos[1] += (new_input->keyboard.W.ended_down) ? 0.01f : 0.0f;
                input_pos[1] -= (new_input->keyboard.S.ended_down) ? 0.01f : 0.0f;

                input_pos[2] += (new_input->keyboard.E.ended_down) ? 0.01f : 0.0f;
                input_pos[2] -= (new_input->keyboard.Q.ended_down) ? 0.01f : 0.0f;
                glVertexAttrib4fv(3, input_pos);

                f32 t = new_input->gamepads[0].RightTrigger.end;
                glVertexAttrib1f(4, t);

                glPointSize(10.0f);
                // glDrawArrays(GL_TRIANGLES, 0, 6);
                // glDrawArrays(GL_POINTS, 6, 1);
                glDrawArrays(GL_LINES, 7, 24);

                SwapBuffers(MainCtx.device_ctx);
#else
#ifdef DEBUG
                FILETIME new_write_time = WinPlatGetLastWriteTime(source_dll_name);
                if (CompareFileTime(&new_write_time, &game_code.last_write_time) != 0) {
                    WinPlatFreeGameCode(&game_code);
                    game_code = WinPlatLoadGameCode(source_dll_name);
                }
#endif

                ZeroMemory(bitmap.memory, bitmap.size);

                game_code.update_and_render(&memory, &bitmap, new_input, &MainCtx.is_running);

                // Drawing the Bitmap
                WinPlatDimensions dimenstions = WinPlatGetDimensions(window_handle);

                WinPlatDisplayBitmap(MainCtx.device_ctx, dimenstions.width, dimenstions.height);
#endif

                // Timing Stuff ----------------------------------------------------------------- //
                u64 end_cycle_count = __rdtsc();
                i64 end_counter     = WinPlatGetTime();

                f64 mega_cylces_elapsed =
                    (((f64)end_cycle_count - (f64)last_cycle_count) / (1000.0f * 1000.0f));

                i64 counter_elapsed = end_counter - last_counter;

                f64 time_elapsed_for_frame = (f64)counter_elapsed / (f64)perf_counter_frequency;

                if (time_elapsed_for_frame <= max_time_per_frame) {
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
                counter_elapsed  = WinPlatGetTime() - temp;
                f64 ms_per_frame = (1000.0f * (f64)counter_elapsed) / (f64)perf_counter_frequency;
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
                // Timing Stuff ----------------------------------------------------------------- //

                // Double buffering input state
                Swap(CanvasInput*, old_input, new_input);
            }
            // Main Loop ------------------------------------------------------------------------ //

            // Cleanup
#if OPENGL
            GlPipelineDelete(&gl_state);
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
// Main Windows Entry Point --------------------------------------------------------------------- //
