// i_input.c - Doom input API using TUIX (FINAL FIXED)

#include "doomkeys.h"
#include "doomtype.h"
#include "d_event.h"
#include "i_input.h"

#include "i_input_tuix.h"

static int initialized = 0;

// ============================================================
// 🔥 SET1 SCANCODE → DOOM KEY
// ============================================================

static int MapKey(int sc)
{
    switch (sc)
    {
        // =========================
        // ESC / BASIC
        // =========================
        case 0x01: return KEY_ESCAPE;
        case 0x1C: return KEY_ENTER;
        case 0x0F: return KEY_TAB;
        case 0x0E: return KEY_BACKSPACE;
        case 0x39: return ' ';

        // =========================
        // MODIFIERS
        // =========================
        case 0x2A:
        case 0x36: return KEY_RSHIFT;

        case 0x1D:
        case 0x11D: return KEY_RCTRL;

        case 0x38:
        case 0x138: return KEY_RALT;

        case 0x3A: return KEY_CAPSLOCK;
        case 0x45: return KEY_NUMLOCK;
        case 0x46: return KEY_SCRLCK;

        // =========================
        // FUNCTION KEYS
        // =========================
        case 0x3B: return KEY_F1;
        case 0x3C: return KEY_F2;
        case 0x3D: return KEY_F3;
        case 0x3E: return KEY_F4;
        case 0x3F: return KEY_F5;
        case 0x40: return KEY_F6;
        case 0x41: return KEY_F7;
        case 0x42: return KEY_F8;
        case 0x43: return KEY_F9;
        case 0x44: return KEY_F10;
        case 0x57: return KEY_F11;
        case 0x58: return KEY_F12;

        // =========================
        // ARROWS (E0)
        // =========================
        case 0x148: return KEY_UPARROW;
        case 0x150: return KEY_DOWNARROW;
        case 0x14B: return KEY_LEFTARROW;
        case 0x14D: return KEY_RIGHTARROW;

        // =========================
        // NAVIGATION (E0)
        // =========================
        case 0x147: return KEY_HOME;
        case 0x14F: return KEY_END;
        case 0x149: return KEY_PGUP;
        case 0x151: return KEY_PGDN;
        case 0x152: return KEY_INS;
        case 0x153: return KEY_DEL;

        case 0x137: return KEY_PRTSCR;
        case 0x145: return KEY_PAUSE;

        // =========================
        // LETTERS
        // =========================
        case 0x1E: return 'a';
        case 0x30: return 'b';
        case 0x2E: return 'c';
        case 0x20: return 'd';
        case 0x12: return 'e';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x17: return 'i';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x32: return 'm';
        case 0x31: return 'n';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x10: return 'q';
        case 0x13: return 'r';
        case 0x1F: return 's';
        case 0x14: return 't';
        case 0x16: return 'u';
        case 0x2F: return 'v';
        case 0x11: return 'w';
        case 0x2D: return 'x';
        case 0x15: return 'y';
        case 0x2C: return 'z';

        // =========================
        // NUMBERS
        // =========================
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';

        // =========================
        // SYMBOLS
        // =========================
        case 0x0C: return KEY_MINUS;
        case 0x0D: return KEY_EQUALS;

        case 0x1A: return '[';
        case 0x1B: return ']';
        case 0x2B: return '\\';

        case 0x27: return ';';
        case 0x28: return '\'';
        case 0x29: return '`';

        case 0x33: return ',';
        case 0x34: return '.';
        case 0x35: return '/';

        case 0x56: return KEY_NONUSBACKSLASH;

        // =========================
        // NUMPAD (без NumLock)
        // =========================
        case 0x52: return KEYP_0;
        case 0x4F: return KEYP_1;
        case 0x50: return KEYP_2;
        case 0x51: return KEYP_3;
        case 0x4B: return KEYP_4;
        case 0x4C: return KEYP_5;
        case 0x4D: return KEYP_6;
        case 0x47: return KEYP_7;
        case 0x48: return KEYP_8;
        case 0x49: return KEYP_9;

        case 0x53: return KEYP_PERIOD;

        // =========================
        // NUMPAD (операції)
        // =========================
        case 0x135: return KEYP_DIVIDE;    // /
        case 0x37:  return KEYP_MULTIPLY;  // *
        case 0x4A:  return KEYP_MINUS;
        case 0x4E:  return KEYP_PLUS;
        case 0x11C: return KEYP_ENTER;

        // =========================
        // FALLBACK
        // =========================
    }

    return 0;
}

// ============================================================

void I_InitInput(void)
{
    if (initialized)
        return;

    listen_input();
    initialized = 1;
}

void I_ShutdownInput(void)
{
    if (!initialized)
        return;

    stop_input_listening();
    initialized = 0;
}

// ============================================================
// 🔥 MAIN LOOP
// ============================================================

void I_StartTic(void)
{
    while (1)
    {
        TuixInputSnapshot s = get_input_snapshot();

        if (!s.keyboard->has_event && !s.mouse->has_event)
            break;

        // =========================
        // KEYBOARD
        // =========================
        if (s.keyboard->has_event)
        {
            event_t ev;

            ev.type = s.keyboard->pressed ? ev_keydown : ev_keyup;

            int key = MapKey(s.keyboard->scancode);

            // uppercase (doom expects it)
            if (key >= 'a' && key <= 'z')
                key -= 32;

            ev.data1 = key;
            ev.data2 = 0;
            ev.data3 = 0;

            if (key != 0)
                D_PostEvent(&ev);
        }

        // =========================
        // MOUSE
        // =========================
        if (s.mouse->has_event)
        {
            event_t ev;

            ev.type  = ev_mouse;
            ev.data1 = s.mouse->buttons_held;
            ev.data2 = s.mouse->col;
            ev.data3 = s.mouse->row;

            D_PostEvent(&ev);
        }
    }
}

// ============================================================
// STUBS
// ============================================================

void I_ReadMouse(void) {}
void I_BindInputVariables(void) {}
void I_HandleKeyboardEvent(void *ev) {}
void I_HandleMouseEvent(void *ev) {}

void I_StartTextInput(int x1,int y1,int x2,int y2)
{
    (void)x1; (void)y1; (void)x2; (void)y2;
}

void I_StopTextInput(void) {}