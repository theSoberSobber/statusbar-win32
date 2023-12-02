#include <windows.h>
#include <winuser.h>
#include <stdlib.h>
#include <stdio.h>
#include <shellapi.h>
#include <stdbool.h>

#define NAME "shellhook-win32"     /* Used for window name/class */

#ifdef DEBUG
#define dbg eprint
#else
#define dbg
#endif

/* Shell hook stuff */

typedef BOOL (*RegisterShellHookWindowProc) (HWND);
HWND dwmhwnd;
static UINT shellhookid;    /* Window Message id */

#include "debug.h"

void cleanup() {
    DeregisterShellHookWindow(dwmhwnd);
    DestroyWindow(dwmhwnd);
}

void die(const char *errstr, ...) {
    va_list ap;
    va_start(ap, errstr);
    dbg(errstr, ap);
    va_end(ap);
    cleanup();
    exit(EXIT_FAILURE);
}

LPSTR getclientclassname(HWND hwnd) {
    static TCHAR buf[128];
    GetClassName(hwnd, buf, sizeof buf);
    return buf;
}

LPSTR getclienttitle(HWND hwnd) {
    static TCHAR buf[128];
    GetWindowText(hwnd, buf, sizeof buf);
    return buf;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        break;
    case WM_CLOSE:
        cleanup();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        if (msg == shellhookid) {
            switch (wParam & 0x7fff) {
            case HSHELL_WINDOWCREATED:
                dbg("=> Window created: %s, %s || %d\n", getclienttitle((HWND)lParam), getclientclassname((HWND)lParam), (HWND)lParam);
                break;
            case HSHELL_WINDOWDESTROYED:
                dbg("=> Window Destroyed: %s, %s || %d\n", getclienttitle((HWND)lParam), getclientclassname((HWND)lParam), (HWND)lParam);
                break;
            case HSHELL_WINDOWACTIVATED:
                dbg("=> Window activated: %s, %s || %d\n", getclienttitle((HWND)lParam), getclientclassname((HWND)lParam), (HWND)lParam);
                break;
            }
        } else return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

void setup(HINSTANCE hInstance) {

    WNDCLASSEX winClass;

    winClass.cbSize = sizeof(WNDCLASSEX);
    winClass.style = 0;
    winClass.lpfnWndProc = WndProc;
    winClass.cbClsExtra = 0;
    winClass.cbWndExtra = 0;
    winClass.hInstance = hInstance;
    winClass.hIcon = NULL;
    winClass.hIconSm = NULL;
    winClass.hCursor = NULL;
    winClass.hbrBackground = NULL;
    winClass.lpszMenuName = NULL;
    winClass.lpszClassName = NAME;

    if (!RegisterClassEx(&winClass)) die("Error registering window class");

    dwmhwnd = CreateWindowEx(0, NAME, NAME, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    if (!dwmhwnd) die("Error creating window");

    RegisterShellHookWindow(dwmhwnd);
    /* Grab a dynamic id for the SHELLHOOK message to be used later */
    shellhookid = RegisterWindowMessage("SHELLHOOK");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#ifdef DEBUG
    extern char ** __argv;
    extern int __argc;

    // This attaches a console to the parent process if it has a console
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // reopen stout handle as console window output
        freopen("CONOUT$", "wb", stdout);
        // reopen stderr handle as console window output
        freopen("CONOUT$", "wb", stderr);
    }
#endif

    MSG msg;

    setup(hInstance);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    cleanup();

    return msg.wParam;
}