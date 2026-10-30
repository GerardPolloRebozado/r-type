/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-r-type-1
** File description:
** windows.h without GDI/USER so it can coexist with raylib.
**
** raylib declares Rectangle (typedef), CloseWindow, ShowCursor, DrawText,
** LoadImage and PlaySound with signatures that collide with wingdi.h /
** winuser.h / mmsystem.h. The Editor translation units that need Win32 also
** include raylib, so include THIS header instead of <windows.h>.
**
** The few USER32 entry points the Editor needs are declared by hand below.
*/
#ifndef EDITOR_COMPAT_WINDOWSAPI_HPP_
#define EDITOR_COMPAT_WINDOWSAPI_HPP_

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NOUSER
#define NOUSER
#endif
#ifndef MMNOSOUND
#define MMNOSOUND
#endif

#include <windows.h>

// minwindef.h defines these unconditionally (even with NOMINMAX / LEAN).
#undef far
#undef near
#undef FAR
#undef NEAR
#undef min
#undef max

#ifndef WM_CLOSE
#define WM_CLOSE 0x0010
#endif

extern "C" {
typedef int(__stdcall* WNDENUMPROC)(HWND, LPARAM);
__declspec(dllimport) int __stdcall EnumWindows(WNDENUMPROC, LPARAM);
__declspec(dllimport) int __stdcall IsWindowVisible(HWND);
__declspec(dllimport) int __stdcall PostMessageA(HWND, UINT, WPARAM, LPARAM);
__declspec(dllimport) DWORD __stdcall GetWindowThreadProcessId(HWND, LPDWORD);
}
#endif // _WIN32

#endif // EDITOR_COMPAT_WINDOWSAPI_HPP_
