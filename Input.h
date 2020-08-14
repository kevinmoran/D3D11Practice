#pragma once

#include "types.h"

struct KeyState {
    bool8 isDown;
    bool8 wasDown;

    bool wentDown() { return isDown && !wasDown; }
    bool wentUp() { return !isDown && wasDown; }
};

inline void keysUpdateWasDownState(KeyState keys[], i32 count) {
    for(i32 i=0; i<count; ++i)
        keys[i].wasDown = keys[i].isDown;
}

enum Keys {
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_ENTER,
    KEY_SHIFT,
    KEY_CTRL,
    KEY_ALT,
    // KEY_PAUSE, // doesn't work, see note in WndProc
    KEY_CAPSLOCK,
    KEY_ESC,
    KEY_SPACE,
    KEY_PGUP,
    KEY_PGDN,
    KEY_HOME,
    KEY_END,
    KEY_LEFT,
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    // KEY_PRINT_SCREEN, // doesn't work, see note in WndProc
    KEY_INSERT,
    KEY_DELETE,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_NUMPAD_0,
    KEY_NUMPAD_1,
    KEY_NUMPAD_2,
    KEY_NUMPAD_3,
    KEY_NUMPAD_4,
    KEY_NUMPAD_5,
    KEY_NUMPAD_6,
    KEY_NUMPAD_7,
    KEY_NUMPAD_8,
    KEY_NUMPAD_9,
    KEY_NUMPAD_MULTIPLY,
    KEY_NUMPAD_ADD,
    KEY_NUMPAD_SUBTRACT,
    KEY_NUMPAD_DECIMAL,
    KEY_NUMPAD_DIVIDE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_SEMICOLON,
    KEY_PLUS,
    KEY_COMMA,
    KEY_MINUS,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_GRAVE_ACCENT,
    KEY_LEFT_BRACKET,
    KEY_BACKSLASH,
    KEY_RIGHT_BRACKET,
    KEY_APOSTROPHE,
    KEY_COUNT
};

// enum GameAction {
//     GameActionMoveCamFwd,
//     GameActionMoveCamBack,
//     GameActionMoveCamLeft,
//     GameActionMoveCamRight,
//     GameActionTurnCamLeft,
//     GameActionTurnCamRight,
//     GameActionLookUp,
//     GameActionLookDown,
//     GameActionRaiseCam,
//     GameActionLowerCam,
//     GameActionCount
// };
