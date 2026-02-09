#ifndef CANVAS_H
#define CANVAS_H
// The Game Interface --------------------------------------------------------------------------- //

#include "base/sugars.hpp"

#ifdef DEBUG
typedef struct {
    void* memory;
    u64   size;
} DBG_FileStruct;

#define DBG_PLAT_READ_ENTIRE_FILE(name) DBG_FileStruct name(char* file_name)
typedef DBG_PLAT_READ_ENTIRE_FILE(dbg_plat_read_entire_file);

#define DBG_PLAT_FREE_FILE_MEMORY(name) void name(void* memory)
typedef DBG_PLAT_FREE_FILE_MEMORY(dbg_plat_free_file_memory);

#define DBG_PLAT_WRITE_ENTIRE_FILE(name) bool name(char* file_name, void* memory, u32 memory_size)
typedef DBG_PLAT_WRITE_ENTIRE_FILE(dbg_plat_write_entire_file);

#endif

typedef struct {
    void* memory;
    u32   width;
    u32   height;
    u32   size;
    u32   pitch;
    u32   bytes_per_pixel;
} CanvasBitMap;

typedef struct {
    u32  transition;
    bool ended_down;
} CanvasButtonState;

typedef struct {
    f32 min;
    f32 max;
    f32 start;
    f32 end;
} CanvasAnalogState;

typedef struct {
    CanvasAnalogState LeftStickX;
    CanvasAnalogState LeftStickY;

    CanvasAnalogState RightStickX;
    CanvasAnalogState RightStickY;

    CanvasAnalogState LeftTrigger;
    CanvasAnalogState RightTrigger;

    union {
        CanvasButtonState Buttons[14];

        struct {
            // XYAB 4
            CanvasButtonState X;
            CanvasButtonState Y;
            CanvasButtonState A;
            CanvasButtonState B;

            // Dpad 4
            CanvasButtonState Up;
            CanvasButtonState Down;
            CanvasButtonState Left;
            CanvasButtonState Right;

            // L/R Shoulders 2
            CanvasButtonState LS;
            CanvasButtonState RS;

            // L/R Thumb 2
            CanvasButtonState LT;
            CanvasButtonState RT;

            // Start/Stop 2
            CanvasButtonState Start;
            CanvasButtonState Stop;
        };
    };
} CanvasControllerInput;

typedef struct {
    union {
        CanvasButtonState Buttons[256];

        struct {
            CanvasButtonState _Reserved0[8]; // 0x00-0x07

            CanvasButtonState Backspace; // 0x08 VK_BACK
            CanvasButtonState Tab;       // 0x09 VK_TAB

            CanvasButtonState _Reserved10[2]; // 0x0A-0x0B

            CanvasButtonState Clear; // 0x0C VK_CLEAR
            CanvasButtonState Enter; // 0x0D VK_RETURN

            CanvasButtonState _Reserved14[2]; // 0x0E-0x0F

            CanvasButtonState Shift;    // 0x10 VK_SHIFT
            CanvasButtonState Control;  // 0x11 VK_CONTROL
            CanvasButtonState Alt;      // 0x12 VK_MENU
            CanvasButtonState Pause;    // 0x13 VK_PAUSE
            CanvasButtonState CapsLock; // 0x14 VK_CAPITAL

            CanvasButtonState _Reserved21[6]; // 0x15-0x1A (IME keys)

            CanvasButtonState Escape; // 0x1B VK_ESCAPE

            CanvasButtonState _Reserved28[4]; // 0x1C-0x1F (IME keys)

            CanvasButtonState Space;       // 0x20 VK_SPACE
            CanvasButtonState PageUp;      // 0x21 VK_PRIOR
            CanvasButtonState PageDown;    // 0x22 VK_NEXT
            CanvasButtonState End;         // 0x23 VK_END
            CanvasButtonState Home;        // 0x24 VK_HOME
            CanvasButtonState Left;        // 0x25 VK_LEFT
            CanvasButtonState Up;          // 0x26 VK_UP
            CanvasButtonState Right;       // 0x27 VK_RIGHT
            CanvasButtonState Down;        // 0x28 VK_DOWN
            CanvasButtonState Select;      // 0x29 VK_SELECT
            CanvasButtonState Print;       // 0x2A VK_PRINT
            CanvasButtonState Execute;     // 0x2B VK_EXECUTE
            CanvasButtonState PrintScreen; // 0x2C VK_SNAPSHOT
            CanvasButtonState Insert;      // 0x2D VK_INSERT
            CanvasButtonState Delete;      // 0x2E VK_DELETE
            CanvasButtonState Help;        // 0x2F VK_HELP

            CanvasButtonState Num0; // 0x30 '0'
            CanvasButtonState Num1; // 0x31 '1'
            CanvasButtonState Num2; // 0x32 '2'
            CanvasButtonState Num3; // 0x33 '3'
            CanvasButtonState Num4; // 0x34 '4'
            CanvasButtonState Num5; // 0x35 '5'
            CanvasButtonState Num6; // 0x36 '6'
            CanvasButtonState Num7; // 0x37 '7'
            CanvasButtonState Num8; // 0x38 '8'
            CanvasButtonState Num9; // 0x39 '9'

            CanvasButtonState _Reserved58[7]; // 0x3A-0x40

            CanvasButtonState A; // 0x41 'A'
            CanvasButtonState B; // 0x42 'B'
            CanvasButtonState C; // 0x43 'C'
            CanvasButtonState D; // 0x44 'D'
            CanvasButtonState E; // 0x45 'E'
            CanvasButtonState F; // 0x46 'F'
            CanvasButtonState G; // 0x47 'G'
            CanvasButtonState H; // 0x48 'H'
            CanvasButtonState I; // 0x49 'I'
            CanvasButtonState J; // 0x4A 'J'
            CanvasButtonState K; // 0x4B 'K'
            CanvasButtonState L; // 0x4C 'L'
            CanvasButtonState M; // 0x4D 'M'
            CanvasButtonState N; // 0x4E 'N'
            CanvasButtonState O; // 0x4F 'O'
            CanvasButtonState P; // 0x50 'P'
            CanvasButtonState Q; // 0x51 'Q'
            CanvasButtonState R; // 0x52 'R'
            CanvasButtonState S; // 0x53 'S'
            CanvasButtonState T; // 0x54 'T'
            CanvasButtonState U; // 0x55 'U'
            CanvasButtonState V; // 0x56 'V'
            CanvasButtonState W; // 0x57 'W'
            CanvasButtonState X; // 0x58 'X'
            CanvasButtonState Y; // 0x59 'Y'
            CanvasButtonState Z; // 0x5A 'Z'

            CanvasButtonState LeftSuper;  // 0x5B VK_LWIN
            CanvasButtonState RightSuper; // 0x5C VK_RWIN
            CanvasButtonState Apps;       // 0x5D VK_APPS

            CanvasButtonState _Reserved94[2]; // 0x5E-0x5F

            CanvasButtonState Numpad0;         // 0x60 VK_NUMPAD0
            CanvasButtonState Numpad1;         // 0x61 VK_NUMPAD1
            CanvasButtonState Numpad2;         // 0x62 VK_NUMPAD2
            CanvasButtonState Numpad3;         // 0x63 VK_NUMPAD3
            CanvasButtonState Numpad4;         // 0x64 VK_NUMPAD4
            CanvasButtonState Numpad5;         // 0x65 VK_NUMPAD5
            CanvasButtonState Numpad6;         // 0x66 VK_NUMPAD6
            CanvasButtonState Numpad7;         // 0x67 VK_NUMPAD7
            CanvasButtonState Numpad8;         // 0x68 VK_NUMPAD8
            CanvasButtonState Numpad9;         // 0x69 VK_NUMPAD9
            CanvasButtonState NumpadMultiply;  // 0x6A VK_MULTIPLY
            CanvasButtonState NumpadAdd;       // 0x6B VK_ADD
            CanvasButtonState NumpadSeparator; // 0x6C VK_SEPARATOR
            CanvasButtonState NumpadSubtract;  // 0x6D VK_SUBTRACT
            CanvasButtonState NumpadDecimal;   // 0x6E VK_DECIMAL
            CanvasButtonState NumpadDivide;    // 0x6F VK_DIVIDE

            CanvasButtonState F1;  // 0x70 VK_F1
            CanvasButtonState F2;  // 0x71 VK_F2
            CanvasButtonState F3;  // 0x72 VK_F3
            CanvasButtonState F4;  // 0x73 VK_F4
            CanvasButtonState F5;  // 0x74 VK_F5
            CanvasButtonState F6;  // 0x75 VK_F6
            CanvasButtonState F7;  // 0x76 VK_F7
            CanvasButtonState F8;  // 0x77 VK_F8
            CanvasButtonState F9;  // 0x78 VK_F9
            CanvasButtonState F10; // 0x79 VK_F10
            CanvasButtonState F11; // 0x7A VK_F11
            CanvasButtonState F12; // 0x7B VK_F12
            CanvasButtonState F13; // 0x7C VK_F13
            CanvasButtonState F14; // 0x7D VK_F14
            CanvasButtonState F15; // 0x7E VK_F15
            CanvasButtonState F16; // 0x7F VK_F16
            CanvasButtonState F17; // 0x80 VK_F17
            CanvasButtonState F18; // 0x81 VK_F18
            CanvasButtonState F19; // 0x82 VK_F19
            CanvasButtonState F20; // 0x83 VK_F20
            CanvasButtonState F21; // 0x84 VK_F21
            CanvasButtonState F22; // 0x85 VK_F22
            CanvasButtonState F23; // 0x86 VK_F23
            CanvasButtonState F24; // 0x87 VK_F24

            CanvasButtonState _Reserved136[8]; // 0x88-0x8F

            CanvasButtonState NumLock;    // 0x90 VK_NUMLOCK
            CanvasButtonState ScrollLock; // 0x91 VK_SCROLL

            CanvasButtonState _Reserved146[14]; // 0x92-0x9F

            CanvasButtonState LeftShift;    // 0xA0 VK_LSHIFT
            CanvasButtonState RightShift;   // 0xA1 VK_RSHIFT
            CanvasButtonState LeftControl;  // 0xA2 VK_LCONTROL
            CanvasButtonState RightControl; // 0xA3 VK_RCONTROL
            CanvasButtonState LeftAlt;      // 0xA4 VK_LMENU
            CanvasButtonState RightAlt;     // 0xA5 VK_RMENU

            CanvasButtonState BrowserBack;      // 0xA6 VK_BROWSER_BACK
            CanvasButtonState BrowserForward;   // 0xA7 VK_BROWSER_FORWARD
            CanvasButtonState BrowserRefresh;   // 0xA8 VK_BROWSER_REFRESH
            CanvasButtonState BrowserStop;      // 0xA9 VK_BROWSER_STOP
            CanvasButtonState BrowserSearch;    // 0xAA VK_BROWSER_SEARCH
            CanvasButtonState BrowserFavorites; // 0xAB VK_BROWSER_FAVORITES
            CanvasButtonState BrowserHome;      // 0xAC VK_BROWSER_HOME

            CanvasButtonState VolumeMute;        // 0xAD VK_VOLUME_MUTE
            CanvasButtonState VolumeDown;        // 0xAE VK_VOLUME_DOWN
            CanvasButtonState VolumeUp;          // 0xAF VK_VOLUME_UP
            CanvasButtonState MediaNextTrack;    // 0xB0 VK_MEDIA_NEXT_TRACK
            CanvasButtonState MediaPrevTrack;    // 0xB1 VK_MEDIA_PREV_TRACK
            CanvasButtonState MediaStop;         // 0xB2 VK_MEDIA_STOP
            CanvasButtonState MediaPlayPause;    // 0xB3 VK_MEDIA_PLAY_PAUSE
            CanvasButtonState LaunchMail;        // 0xB4 VK_LAUNCH_MAIL
            CanvasButtonState LaunchMediaSelect; // 0xB5 VK_LAUNCH_MEDIA_SELECT
            CanvasButtonState LaunchApp1;        // 0xB6 VK_LAUNCH_APP1
            CanvasButtonState LaunchApp2;        // 0xB7 VK_LAUNCH_APP2

            CanvasButtonState _Reserved184[2]; // 0xB8-0xB9

            CanvasButtonState OEM1;      // 0xBA VK_OEM_1 (';:' on US keyboard)
            CanvasButtonState OEMPlus;   // 0xBB VK_OEM_PLUS ('=+')
            CanvasButtonState OEMComma;  // 0xBC VK_OEM_COMMA (',<')
            CanvasButtonState OEMMinus;  // 0xBD VK_OEM_MINUS ('-_')
            CanvasButtonState OEMPeriod; // 0xBE VK_OEM_PERIOD ('.>')
            CanvasButtonState OEM2;      // 0xBF VK_OEM_2 ('/?')
            CanvasButtonState OEM3;      // 0xC0 VK_OEM_3 ('`~')

            CanvasButtonState _Reserved193[26]; // 0xC1-0xDA

            CanvasButtonState OEM4; // 0xDB VK_OEM_4 ('[{')
            CanvasButtonState OEM5; // 0xDC VK_OEM_5 ('\|')
            CanvasButtonState OEM6; // 0xDD VK_OEM_6 (']}')
            CanvasButtonState OEM7; // 0xDE VK_OEM_7 (''"')
            CanvasButtonState OEM8; // 0xDF VK_OEM_8

            CanvasButtonState _Reserved224[2]; // 0xE0-0xE1

            CanvasButtonState OEM102; // 0xE2 VK_OEM_102 ('<>' or '\|' on RT 102-key kbd)

            CanvasButtonState _Reserved227[2]; // 0xE3-0xE4

            CanvasButtonState ProcessKey; // 0xE5 VK_PROCESSKEY

            CanvasButtonState _Reserved230; // 0xE6

            CanvasButtonState Packet; // 0xE7 VK_PACKET

            CanvasButtonState _Reserved232[8]; // 0xE8-0xEF

            CanvasButtonState _Reserved240[6]; // 0xF0-0xF5

            CanvasButtonState Attn;     // 0xF6 VK_ATTN
            CanvasButtonState CrSel;    // 0xF7 VK_CRSEL
            CanvasButtonState ExSel;    // 0xF8 VK_EXSEL
            CanvasButtonState EraseEOF; // 0xF9 VK_EREOF
            CanvasButtonState Play;     // 0xFA VK_PLAY
            CanvasButtonState Zoom;     // 0xFB VK_ZOOM
            CanvasButtonState NoName;   // 0xFC VK_NONAME
            CanvasButtonState PA1;      // 0xFD VK_PA1
            CanvasButtonState OEMClear; // 0xFE VK_OEM_CLEAR

            CanvasButtonState _Reserved255; // 0xFF
        };
    };
} CanvasKeyboardInput;

typedef struct {
    CanvasControllerInput gamepads[4];
    CanvasKeyboardInput   keyboard;
} CanvasInput;

typedef struct {
    bool is_valid;

    u64   perma_size;
    void* perma_store;

    u64   trans_size;
    void* trans_store;

    dbg_plat_read_entire_file*  DBG_PlatReadEntireFile;
    dbg_plat_free_file_memory*  DBG_PlatFreeFileMemory;
    dbg_plat_write_entire_file* DBG_PlatWriteEntireFile;
} CanvasMemory;



#define CANVAS_UPDATE_AND_RENDER(name)                                                             \
    void name(CanvasMemory* memory, CanvasBitMap* bitmap, CanvasInput* input, bool* running)
typedef CANVAS_UPDATE_AND_RENDER(canvas_update_and_render);
CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRenderStub);



typedef struct {
    i32 jx;
    i32 jy;

    u32 x_off;
    u32 y_off;

    u32 weight;

    f32 dx;
    f32 dy;
    f32 dz;

    f32 theta;
} CanvasState;

// ---------------------------------------------------------------------------------------------- //
#endif
