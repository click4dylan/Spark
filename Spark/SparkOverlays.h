#ifndef sparkoverlays_h
#define sparkoverlays_h
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
extern RECT clientrect_unmodified;
extern RECT clientrect;
extern RECT windowrect;
extern HWND layhWnd;
extern HFONT BIG_FONT;
extern HFONT SMALL_FONT;
extern HFONT DEFAULT_FONT;
void DrawTextOverlay(XY coords, char *text, COLORREF color, int centerscreen, HFONT font);
void TimedOverlayThread(double lifesecs, XY coords, char *text, COLORREF color, int centerscreen, HFONT font);
__forceinline void RenderTimedOverlay(double lifesecs, XY coords, char *text, COLORREF color, int centerscreen, HFONT font);
void DrawDebugOverlay();
void DrawMouseSampleOverlay();
void DrawCenterCrosshair();// int msecs);
void DrawFiringOverlay(int addx, int addy);
#endif