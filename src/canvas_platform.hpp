#ifndef CANVAS_H
#define CANVAS_H
//  Game code interfaces : ----------------------------------------------------------- (section)  //

#include <base/include.cpp>
#include "windows_debug.hpp"
#include "renderer.hpp"


typedef struct {
    void* memory;
    u32   width;
    u32   height;
    u32   size;
    u32   pitch;
    u32   bytes_per_pixel;
} canvas_bitmap;

typedef struct {
    u32  flips;
    bool down;
} canvas_button_state;

#define Held(button) ((button).down)
#define Pushed(button) ((button).down && (button).flips > 0)
#define Released(button) (!(button).down && (button).flips > 0)


typedef struct {
    f32 min;
    f32 max;
    f32 start;
    f32 end;
} canvas_analog_state;

typedef struct {
    canvas_analog_state LeftStickX;
    canvas_analog_state LeftStickY;

    canvas_analog_state RightStickX;
    canvas_analog_state RightStickY;

    canvas_analog_state LeftTrigger;
    canvas_analog_state RightTrigger;

    union {
        canvas_button_state Buttons[14];

        struct {
            // XYAB 4
            canvas_button_state X;
            canvas_button_state Y;
            canvas_button_state A;
            canvas_button_state B;

            // Dpad 4
            canvas_button_state Up;
            canvas_button_state Down;
            canvas_button_state Left;
            canvas_button_state Right;

            // L/R Shoulders 2
            canvas_button_state LS;
            canvas_button_state RS;

            // L/R Thumb 2
            canvas_button_state LT;
            canvas_button_state RT;

            // Start/Stop 2
            canvas_button_state Start;
            canvas_button_state Stop;
        };
    };
} canvas_controller_input;

typedef struct {
    union {
        canvas_button_state Buttons[256];

        struct {
            canvas_button_state _Reserved0[8]; // 0x00-0x07

            canvas_button_state Backspace; // 0x08 VK_BACK
            canvas_button_state Tab;       // 0x09 VK_TAB

            canvas_button_state _Reserved10[2]; // 0x0A-0x0B

            canvas_button_state Clear; // 0x0C VK_CLEAR
            canvas_button_state Enter; // 0x0D VK_RETURN

            canvas_button_state _Reserved14[2]; // 0x0E-0x0F

            canvas_button_state Shift;    // 0x10 VK_SHIFT
            canvas_button_state Control;  // 0x11 VK_CONTROL
            canvas_button_state Alt;      // 0x12 VK_MENU
            canvas_button_state Pause;    // 0x13 VK_PAUSE
            canvas_button_state CapsLock; // 0x14 VK_CAPITAL

            canvas_button_state _Reserved21[6]; // 0x15-0x1A (IME keys)

            canvas_button_state Escape; // 0x1B VK_ESCAPE

            canvas_button_state _Reserved28[4]; // 0x1C-0x1F (IME keys)

            canvas_button_state Space;       // 0x20 VK_SPACE
            canvas_button_state PageUp;      // 0x21 VK_PRIOR
            canvas_button_state PageDown;    // 0x22 VK_NEXT
            canvas_button_state End;         // 0x23 VK_END
            canvas_button_state Home;        // 0x24 VK_HOME
            canvas_button_state Left;        // 0x25 VK_LEFT
            canvas_button_state Up;          // 0x26 VK_UP
            canvas_button_state Right;       // 0x27 VK_RIGHT
            canvas_button_state Down;        // 0x28 VK_DOWN
            canvas_button_state Select;      // 0x29 VK_SELECT
            canvas_button_state Print;       // 0x2A VK_PRINT
            canvas_button_state Execute;     // 0x2B VK_EXECUTE
            canvas_button_state PrintScreen; // 0x2C VK_SNAPSHOT
            canvas_button_state Insert;      // 0x2D VK_INSERT
            canvas_button_state Delete;      // 0x2E VK_DELETE
            canvas_button_state Help;        // 0x2F VK_HELP

            canvas_button_state Num0; // 0x30 '0'
            canvas_button_state Num1; // 0x31 '1'
            canvas_button_state Num2; // 0x32 '2'
            canvas_button_state Num3; // 0x33 '3'
            canvas_button_state Num4; // 0x34 '4'
            canvas_button_state Num5; // 0x35 '5'
            canvas_button_state Num6; // 0x36 '6'
            canvas_button_state Num7; // 0x37 '7'
            canvas_button_state Num8; // 0x38 '8'
            canvas_button_state Num9; // 0x39 '9'

            canvas_button_state _Reserved58[7]; // 0x3A-0x40

            canvas_button_state A; // 0x41 'A'
            canvas_button_state B; // 0x42 'B'
            canvas_button_state C; // 0x43 'C'
            canvas_button_state D; // 0x44 'D'
            canvas_button_state E; // 0x45 'E'
            canvas_button_state F; // 0x46 'F'
            canvas_button_state G; // 0x47 'G'
            canvas_button_state H; // 0x48 'H'
            canvas_button_state I; // 0x49 'I'
            canvas_button_state J; // 0x4A 'J'
            canvas_button_state K; // 0x4B 'K'
            canvas_button_state L; // 0x4C 'L'
            canvas_button_state M; // 0x4D 'M'
            canvas_button_state N; // 0x4E 'N'
            canvas_button_state O; // 0x4F 'O'
            canvas_button_state P; // 0x50 'P'
            canvas_button_state Q; // 0x51 'Q'
            canvas_button_state R; // 0x52 'R'
            canvas_button_state S; // 0x53 'S'
            canvas_button_state T; // 0x54 'T'
            canvas_button_state U; // 0x55 'U'
            canvas_button_state V; // 0x56 'V'
            canvas_button_state W; // 0x57 'W'
            canvas_button_state X; // 0x58 'X'
            canvas_button_state Y; // 0x59 'Y'
            canvas_button_state Z; // 0x5A 'Z'

            canvas_button_state LeftSuper;  // 0x5B VK_LWIN
            canvas_button_state RightSuper; // 0x5C VK_RWIN
            canvas_button_state Apps;       // 0x5D VK_APPS

            canvas_button_state _Reserved94[2]; // 0x5E-0x5F

            canvas_button_state Numpad0;         // 0x60 VK_NUMPAD0
            canvas_button_state Numpad1;         // 0x61 VK_NUMPAD1
            canvas_button_state Numpad2;         // 0x62 VK_NUMPAD2
            canvas_button_state Numpad3;         // 0x63 VK_NUMPAD3
            canvas_button_state Numpad4;         // 0x64 VK_NUMPAD4
            canvas_button_state Numpad5;         // 0x65 VK_NUMPAD5
            canvas_button_state Numpad6;         // 0x66 VK_NUMPAD6
            canvas_button_state Numpad7;         // 0x67 VK_NUMPAD7
            canvas_button_state Numpad8;         // 0x68 VK_NUMPAD8
            canvas_button_state Numpad9;         // 0x69 VK_NUMPAD9
            canvas_button_state NumpadMultiply;  // 0x6A VK_MULTIPLY
            canvas_button_state NumpadAdd;       // 0x6B VK_ADD
            canvas_button_state NumpadSeparator; // 0x6C VK_SEPARATOR
            canvas_button_state NumpadSubtract;  // 0x6D VK_SUBTRACT
            canvas_button_state NumpadDecimal;   // 0x6E VK_DECIMAL
            canvas_button_state NumpadDivide;    // 0x6F VK_DIVIDE

            canvas_button_state F1;  // 0x70 VK_F1
            canvas_button_state F2;  // 0x71 VK_F2
            canvas_button_state F3;  // 0x72 VK_F3
            canvas_button_state F4;  // 0x73 VK_F4
            canvas_button_state F5;  // 0x74 VK_F5
            canvas_button_state F6;  // 0x75 VK_F6
            canvas_button_state F7;  // 0x76 VK_F7
            canvas_button_state F8;  // 0x77 VK_F8
            canvas_button_state F9;  // 0x78 VK_F9
            canvas_button_state F10; // 0x79 VK_F10
            canvas_button_state F11; // 0x7A VK_F11
            canvas_button_state F12; // 0x7B VK_F12
            canvas_button_state F13; // 0x7C VK_F13
            canvas_button_state F14; // 0x7D VK_F14
            canvas_button_state F15; // 0x7E VK_F15
            canvas_button_state F16; // 0x7F VK_F16
            canvas_button_state F17; // 0x80 VK_F17
            canvas_button_state F18; // 0x81 VK_F18
            canvas_button_state F19; // 0x82 VK_F19
            canvas_button_state F20; // 0x83 VK_F20
            canvas_button_state F21; // 0x84 VK_F21
            canvas_button_state F22; // 0x85 VK_F22
            canvas_button_state F23; // 0x86 VK_F23
            canvas_button_state F24; // 0x87 VK_F24

            canvas_button_state _Reserved136[8]; // 0x88-0x8F

            canvas_button_state NumLock;    // 0x90 VK_NUMLOCK
            canvas_button_state ScrollLock; // 0x91 VK_SCROLL

            canvas_button_state _Reserved146[14]; // 0x92-0x9F

            canvas_button_state LeftShift;    // 0xA0 VK_LSHIFT
            canvas_button_state RightShift;   // 0xA1 VK_RSHIFT
            canvas_button_state LeftControl;  // 0xA2 VK_LCONTROL
            canvas_button_state RightControl; // 0xA3 VK_RCONTROL
            canvas_button_state LeftAlt;      // 0xA4 VK_LMENU
            canvas_button_state RightAlt;     // 0xA5 VK_RMENU

            canvas_button_state BrowserBack;      // 0xA6 VK_BROWSER_BACK
            canvas_button_state BrowserForward;   // 0xA7 VK_BROWSER_FORWARD
            canvas_button_state BrowserRefresh;   // 0xA8 VK_BROWSER_REFRESH
            canvas_button_state BrowserStop;      // 0xA9 VK_BROWSER_STOP
            canvas_button_state BrowserSearch;    // 0xAA VK_BROWSER_SEARCH
            canvas_button_state BrowserFavorites; // 0xAB VK_BROWSER_FAVORITES
            canvas_button_state BrowserHome;      // 0xAC VK_BROWSER_HOME

            canvas_button_state VolumeMute;        // 0xAD VK_VOLUME_MUTE
            canvas_button_state VolumeDown;        // 0xAE VK_VOLUME_DOWN
            canvas_button_state VolumeUp;          // 0xAF VK_VOLUME_UP
            canvas_button_state MediaNextTrack;    // 0xB0 VK_MEDIA_NEXT_TRACK
            canvas_button_state MediaPrevTrack;    // 0xB1 VK_MEDIA_PREV_TRACK
            canvas_button_state MediaStop;         // 0xB2 VK_MEDIA_STOP
            canvas_button_state MediaPlayPause;    // 0xB3 VK_MEDIA_PLAY_PAUSE
            canvas_button_state LaunchMail;        // 0xB4 VK_LAUNCH_MAIL
            canvas_button_state LaunchMediaSelect; // 0xB5 VK_LAUNCH_MEDIA_SELECT
            canvas_button_state LaunchApp1;        // 0xB6 VK_LAUNCH_APP1
            canvas_button_state LaunchApp2;        // 0xB7 VK_LAUNCH_APP2

            canvas_button_state _Reserved184[2]; // 0xB8-0xB9

            canvas_button_state OEM1;      // 0xBA VK_OEM_1 (';:' on US keyboard)
            canvas_button_state OEMPlus;   // 0xBB VK_OEM_PLUS ('=+')
            canvas_button_state OEMComma;  // 0xBC VK_OEM_COMMA (',<')
            canvas_button_state OEMMinus;  // 0xBD VK_OEM_MINUS ('-_')
            canvas_button_state OEMPeriod; // 0xBE VK_OEM_PERIOD ('.>')
            canvas_button_state OEM2;      // 0xBF VK_OEM_2 ('/?')
            canvas_button_state OEM3;      // 0xC0 VK_OEM_3 ('`~')

            canvas_button_state _Reserved193[26]; // 0xC1-0xDA

            canvas_button_state OEM4; // 0xDB VK_OEM_4 ('[{')
            canvas_button_state OEM5; // 0xDC VK_OEM_5 ('\|')
            canvas_button_state OEM6; // 0xDD VK_OEM_6 (']}')
            canvas_button_state OEM7; // 0xDE VK_OEM_7 (''"')
            canvas_button_state OEM8; // 0xDF VK_OEM_8

            canvas_button_state _Reserved224[2]; // 0xE0-0xE1

            canvas_button_state OEM102; // 0xE2 VK_OEM_102 ('<>' or '\|' on RT 102-key kbd)

            canvas_button_state _Reserved227[2]; // 0xE3-0xE4

            canvas_button_state ProcessKey; // 0xE5 VK_PROCESSKEY

            canvas_button_state _Reserved230; // 0xE6

            canvas_button_state Packet; // 0xE7 VK_PACKET

            canvas_button_state _Reserved232[8]; // 0xE8-0xEF

            canvas_button_state _Reserved240[6]; // 0xF0-0xF5

            canvas_button_state Attn;     // 0xF6 VK_ATTN
            canvas_button_state CrSel;    // 0xF7 VK_CRSEL
            canvas_button_state ExSel;    // 0xF8 VK_EXSEL
            canvas_button_state EraseEOF; // 0xF9 VK_EREOF
            canvas_button_state Play;     // 0xFA VK_PLAY
            canvas_button_state Zoom;     // 0xFB VK_ZOOM
            canvas_button_state NoName;   // 0xFC VK_NONAME
            canvas_button_state PA1;      // 0xFD VK_PA1
            canvas_button_state OEMClear; // 0xFE VK_OEM_CLEAR

            canvas_button_state _Reserved255; // 0xFF
        };
    };
} canvas_keyboard_input;

typedef struct {
    canvas_controller_input gamepads[4];
    canvas_keyboard_input   keyboard;
} canvas_input;

typedef struct {
    bool is_valid;

    u64   perma_size;
    void* perma_store;

    u64   trans_size;
    void* trans_store;

#ifdef DEBUG
    dbg_plat_read_entire_file*  DBG_PlatReadEntireFile;
    dbg_plat_free_file_memory*  DBG_PlatFreeFileMemory;
    dbg_plat_write_entire_file* DBG_PlatWriteEntireFile;
#endif

} canvas_memory;



#define CANVAS_UPDATE_AND_RENDER(name)                                                             \
    void name(canvas_memory*      memory,                                                          \
              render_push_buffer* push_buffer,                                                     \
              canvas_input*       input,                                                           \
              f64                 dt,                                                              \
              bool                is_first_time,                                                   \
              bool*               running)
typedef CANVAS_UPDATE_AND_RENDER(canvas_update_and_draw);
CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRenderStub);

//  (section) ----------------------------------------------------------- : Game code interfaces  //
#endif
