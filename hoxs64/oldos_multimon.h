#ifndef __OLDOS_MULTIMON__
#define __OLDOS_MULTIMON__

#include <windows.h>
#include <windowsx.h>


#if !defined(HMONITOR_DECLARED) && (WINVER < 0x0500)
    #define HMONITOR_DECLARED
    DECLARE_HANDLE(HMONITOR);
#endif

#if (WINVER < 0x0500)

//--------------------------------------------------------------------------------------
// Multimon handling to support OSes with or without multimon API support.  
// Purposely avoiding the use of multimon.h so DXUT.lib doesn't require 
// COMPILE_MULTIMON_STUBS and cause complication with MFC or other users of multimon.h
//--------------------------------------------------------------------------------------

#define MONITORINFOF_PRIMARY        0x00000001
#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002
typedef struct tagMONITORINFO
{
    DWORD   cbSize;
    RECT    rcMonitor;
    RECT    rcWork;
    DWORD   dwFlags;
} MONITORINFO, *LPMONITORINFO;
typedef struct tagMONITORINFOEXA : public tagMONITORINFO
{
    CHAR        szDevice[CCHDEVICENAME];
} MONITORINFOEXA, *LPMONITORINFOEXA;
typedef struct tagMONITORINFOEXW : public tagMONITORINFO
{
    WCHAR       szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;

#ifdef UNICODE
typedef MONITORINFOEXW MONITORINFOEX;
typedef LPMONITORINFOEXW LPMONITORINFOEX;
#else
typedef MONITORINFOEXA MONITORINFOEX;
typedef LPMONITORINFOEXA LPMONITORINFOEX;
#endif // UNICODE

#endif


//--------------------------------------------------------------------------------------
// Multimon API handling for OSes with or without multimon API support
//--------------------------------------------------------------------------------------
#define DXUT_PRIMARY_MONITOR ((HMONITOR)0x12340042)
typedef HMONITOR     (WINAPI* LPMONITORFROMWINDOW)(HWND, DWORD);
typedef BOOL         (WINAPI* LPGETMONITORINFO)(HMONITOR, LPMONITORINFO);
typedef HMONITOR     (WINAPI* LPMONITORFROMRECT)(LPCRECT, DWORD);

typedef HRESULT (WINAPI *LPDWMISCOMPOSITIONENABLED)(__out BOOL *);
typedef HRESULT (WINAPI *LPDWMENABLECOMPOSITION)(UINT);
typedef HRESULT (WINAPI *LPDWMGETWINDOWATTRIBUTE)(HWND, DWORD, PVOID, DWORD);

typedef enum _DWMWINDOWATTRIBUTE { 
  DWMWA_NCRENDERING_ENABLED          = 1,
  DWMWA_NCRENDERING_POLICY,
  DWMWA_TRANSITIONS_FORCEDISABLED,
  DWMWA_ALLOW_NCPAINT,
  DWMWA_CAPTION_BUTTON_BOUNDS,
  DWMWA_NONCLIENT_RTL_LAYOUT,
  DWMWA_FORCE_ICONIC_REPRESENTATION,
  DWMWA_FLIP3D_POLICY,
  DWMWA_EXTENDED_FRAME_BOUNDS,
  DWMWA_HAS_ICONIC_BITMAP,
  DWMWA_DISALLOW_PEEK,
  DWMWA_EXCLUDED_FROM_PEEK,
  DWMWA_CLOAK,
  DWMWA_CLOAKED,
  DWMWA_FREEZE_REPRESENTATION,
  DWMWA_LAST
} DWMWINDOWATTRIBUTE;

#endif