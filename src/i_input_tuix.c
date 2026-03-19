// i_input_tuix.c - TUIX input backend (DOOM-CORRECT)

#include "i_input_tuix.h"
#include "tuix_types.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================
// LOGGING
// ============================================================

static FILE *logf = NULL;

static void log_init(void)
{
    if (logf) return;

    logf = fopen("input_backend_log.csv", "w");

    if (logf)
    {
        fprintf(logf, "time,type,pressed,scancode\n");
        fflush(logf);
    }
}

static void log_key(int pressed, int sc)
{
    if (!logf) return;

    double t = (double)clock() / CLOCKS_PER_SEC;

    fprintf(logf, "%.6f,keyboard,%d,%d\n", t, pressed, sc);
    fflush(logf);
}

// ============================================================

int tuix_input_keyboard_enabled = 1;
int tuix_input_mouse_enabled    = 1;

#define QUEUE_CAP 256

static TuixKeyboardKey kb_queue[QUEUE_CAP];
static int kb_head = 0, kb_tail = 0;

static TuixMouseKey mi_queue[QUEUE_CAP];
static int mi_head = 0, mi_tail = 0;

static volatile int input_running = 0;

// 🔥 scancode state (512 for E0 safety)
static unsigned char key_state[512] = {0};

#ifdef _WIN32
static HANDLE input_thread = NULL;
static CRITICAL_SECTION input_cs;
static int cs_initialized = 0;

#define LOCK()   EnterCriticalSection(&input_cs)
#define UNLOCK() LeaveCriticalSection(&input_cs)
#endif

static TuixMouseKey snap_mouse = {0};
static TuixKeyboardKey snap_keyboard = {0};
static TuixInputSnapshot snap = { .mouse = &snap_mouse, .keyboard = &snap_keyboard };

static double now_seconds(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}

// ============================================================
// PUSH KEYBOARD
// ============================================================

static void push_keyboard(int scancode, int pressed)
{
    int key_id = scancode & 0x1FF;

    if (key_id < 0 || key_id >= 512)
        return;

    if (key_state[key_id] == pressed)
        return;

    key_state[key_id] = pressed;

    TuixKeyboardKey ev;
    memset(&ev, 0, sizeof(ev));

    ev.scancode  = scancode;
    ev.code      = scancode; // 🔥 більше не ASCII
    ev.pressed   = pressed;
    ev.timestamp = now_seconds();
    ev.has_event = 1;

    log_key(pressed, scancode);

    int next = (kb_head + 1) % QUEUE_CAP;

    if (next != kb_tail)
    {
        kb_queue[kb_head] = ev;
        kb_head = next;
    }
}

// ============================================================
// PUSH MOUSE
// ============================================================

static void push_mouse(int buttons, int x, int y)
{
    TuixMouseKey ev;
    memset(&ev, 0, sizeof(ev));

    ev.buttons_held = buttons;
    ev.col = x;
    ev.row = y;
    ev.timestamp = now_seconds();
    ev.has_event = 1;

    int next = (mi_head + 1) % QUEUE_CAP;

    if (next != mi_tail)
    {
        mi_queue[mi_head] = ev;
        mi_head = next;
    }
}

// ============================================================
// WINDOWS INPUT THREAD
// ============================================================

#ifdef _WIN32

static DWORD WINAPI input_thread_fn(LPVOID arg)
{
    (void)arg;

    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD out_mode;
    if (GetConsoleMode(hOut, &out_mode))
    {
        out_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, out_mode);
    }

    DWORD orig_mode;
    GetConsoleMode(hIn, &orig_mode);

    DWORD mode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    SetConsoleMode(hIn, mode);

    INPUT_RECORD rec[64];
    DWORD n_read;

    while (input_running)
    {
        if (!ReadConsoleInputW(hIn, rec, 64, &n_read))
            continue;

        for (DWORD i = 0; i < n_read; i++)
        {
            if (rec[i].EventType == KEY_EVENT && tuix_input_keyboard_enabled)
            {
                KEY_EVENT_RECORD *kr = &rec[i].Event.KeyEvent;

                int pressed = kr->bKeyDown ? 1 : 0;

                int sc = kr->wVirtualScanCode;

                // 🔥 E0 extended keys
                if (kr->dwControlKeyState & ENHANCED_KEY)
                    sc |= 0x100;

                LOCK();
                push_keyboard(sc, pressed);
                UNLOCK();
            }
            else if (rec[i].EventType == MOUSE_EVENT && tuix_input_mouse_enabled)
            {
                MOUSE_EVENT_RECORD *mr = &rec[i].Event.MouseEvent;

                LOCK();
                push_mouse(
                    mr->dwButtonState,
                    mr->dwMousePosition.X,
                    mr->dwMousePosition.Y
                );
                UNLOCK();
            }
        }
    }

    SetConsoleMode(hIn, orig_mode);
    return 0;
}

#endif

// ============================================================
// PUBLIC API
// ============================================================

void listen_input(void)
{
    if (input_running)
        return;

    log_init();

#ifdef _WIN32
    if (!cs_initialized)
    {
        InitializeCriticalSection(&input_cs);
        cs_initialized = 1;
    }
#endif

    memset(key_state, 0, sizeof(key_state));

    kb_head = kb_tail = 0;
    mi_head = mi_tail = 0;

    input_running = 1;

#ifdef _WIN32
    input_thread = CreateThread(NULL, 0, input_thread_fn, NULL, 0, NULL);
#endif
}

void stop_input_listening(void)
{
    if (!input_running)
        return;

    input_running = 0;

#ifdef _WIN32
    WaitForSingleObject(input_thread, INFINITE);
    CloseHandle(input_thread);
    input_thread = NULL;
#endif

    if (logf)
    {
        fclose(logf);
        logf = NULL;
    }
}

TuixInputSnapshot get_input_snapshot(void)
{
    if (!input_running)
        listen_input();

    LOCK();

    if (kb_tail != kb_head)
    {
        snap_keyboard = kb_queue[kb_tail];
        kb_tail = (kb_tail + 1) % QUEUE_CAP;
    }
    else
    {
        snap_keyboard.has_event = 0;
    }

    if (mi_tail != mi_head)
    {
        snap_mouse = mi_queue[mi_tail];
        mi_tail = (mi_tail + 1) % QUEUE_CAP;
    }
    else
    {
        snap_mouse.has_event = 0;
    }

    UNLOCK();

    return snap;
}