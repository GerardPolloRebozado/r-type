/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Centralized ENet include with Windows workarounds.
**
** ENet pulls in <windows.h> (via winsock2.h) on Windows, which collides
** with raylib / raylib-cpp:
**   - wingdi.h declares Rectangle()  vs raylib's `typedef struct Rectangle`
**   - winuser.h declares CloseWindow/ShowCursor vs raylib's functions
**   - winuser.h #defines DrawText/DrawTextEx/LoadImage (to ...A/W variants),
**     hijacking raylib's ::DrawText / ::DrawTextEx / ::LoadImage calls
**   - mmsystem.h declares PlaySound vs raylib's ::PlaySound
**   - minwindef.h #defines near/far (empty) and min/max, breaking
**     raylib-cpp Matrix::Frustum/Perspective/Ortho parameter names
**
** Always include THIS header instead of <enet.h> / "enet.h" directly.
** Include it BEFORE any raylib header when a TU needs both.
*/
#ifndef ENGINE_COMPAT_ENET_WRAPPER_HPP_
#define ENGINE_COMPAT_ENET_WRAPPER_HPP_

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
#endif

#include <enet.h>

#ifdef _WIN32
// minwindef.h defines these unconditionally (even with NOMINMAX / LEAN).
#undef far
#undef near
#undef FAR
#undef NEAR
#undef min
#undef max
// In case lean macros were not active when windows.h was parsed
// (e.g. another header included it first), undo the collisions so that
// raylib headers parsed AFTER this point see clean names.
#ifdef Rectangle
#undef Rectangle
#endif
#ifdef CloseWindow
#undef CloseWindow
#endif
#ifdef ShowCursor
#undef ShowCursor
#endif
#ifdef DrawText
#undef DrawText
#endif
#ifdef DrawTextEx
#undef DrawTextEx
#endif
#ifdef LoadImage
#undef LoadImage
#endif
#ifdef PlaySound
#undef PlaySound
#endif
#endif

#endif // ENGINE_COMPAT_ENET_WRAPPER_HPP_
