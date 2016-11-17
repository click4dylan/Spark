#include "stdafx.h"
#include "Spark.h"
#include "SparkOverlays.h"
#include <thread>
#pragma comment(lib,"winmm.lib")

RECT clientrect_unmodified;
RECT clientrect;
RECT windowrect;
HWND layhWnd;
HFONT BIG_FONT = CreateFontA(48, 0, 0, 0, 600, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, DEFAULT_PITCH, "Arial");
HFONT SMALL_FONT = CreateFontA(18, 0, 0, 0, 600, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, DEFAULT_PITCH, "MS Serif");
HFONT DEFAULT_FONT = CreateFontA(18, 0, 0, 0, 600, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, DEFAULT_PITCH, "Verdana");

void DrawTextOverlay(XY coords, char *text, COLORREF color, int centerscreen, HFONT font)
{
	if (centerscreen)
	{
		//Draw overlay in the center of the screen
		clientrect.left = (LONG)((clientrect.right * 0.5) - coords.x) + globals.WindowBorder.x; //+ 3;
		clientrect.top = (LONG)((clientrect.bottom * 0.5) - coords.y) + globals.WindowBorder.y; //+ 22;
	}
	else
	{
		//Draw overlay where the coords tells us to, minus any window border
		//ClientToScreen(layhWnd, reinterpret_cast<POINT*>(&clientrect.left)); // convert top-left
		clientrect.left = coords.x - windowrect.left;
		clientrect.top = coords.y - windowrect.top;
	}
	
	LPRECT prect = &clientrect;
	HDC layhdc = GetWindowDC(layhWnd);
	HFONT OLD_HFONT = (HFONT)SelectObject(layhdc, font);
	SetTextColor(layhdc, color);
	SetBkMode(layhdc, TRANSPARENT);
	DrawTextA(layhdc, (LPCSTR)text, -1, prect, 0);
	UpdateWindow(layhWnd);
	SelectObject(layhdc, OLD_HFONT);
	ReleaseDC(layhWnd, layhdc);
}

void TimedOverlayThread(double lifesecs, XY coords, char *text, COLORREF color, int centerscreen, HFONT font)
{
	int time = timeGetTime();
	int lifemsecs = (int)(lifesecs * 1000);
	int endtime = time + lifemsecs;
	int nexttime_draw = time;
	//int nexttime_renderoverlay = time;
	char *newtext = new char[strlen(text) + 1];
	strcpy(newtext, text);
	while (!globals.quit)
	{
		time = timeGetTime();
		if (time >= endtime)
			break;
		if (time >= nexttime_draw) {
			DrawTextOverlay(coords, newtext, color, centerscreen, font);
			nexttime_draw = time + 1 - max(0, time - nexttime_draw);
		}
		else
		{
			Sleep(1);
		}
	}
	delete[] newtext;
}

__forceinline void RenderTimedOverlay(double lifesecs, XY coords, char *text, COLORREF color, int centerscreen, HFONT font)
{
	std::thread Overlay(TimedOverlayThread, lifesecs, coords, text, color, centerscreen, font);
	Overlay.detach();
}

void DrawDebugOverlay()
{
	//TODO: clear console
	char debugstr[64];
	sprintf(debugstr, "Max Bullets: %i\n", globals.DebugMaxMouseSamples);
	printf("Max firing sample: %i\n", globals.DebugMaxMouseSamples);
	RenderTimedOverlay(0.5, XY(25, 25), debugstr, RGB(255, 0, 0), TRUE, SMALL_FONT);
	Sleep(120);
}

void DrawMouseSampleOverlay()
{
	char debugstr[64];
	sprintf(debugstr, "Sample: %i", globals.bulletsfired);
	RenderTimedOverlay(0.3, XY(64, 64), debugstr, RGB(0, 255, 0), TRUE, SMALL_FONT);
}

void DrawCenterCrosshair()/*int msecs)*/
{
	DrawTextOverlay({ 8, 8 }, "><", RGB(0, 255, 0), TRUE, SMALL_FONT);
	//RenderTimedOverlay((double)msecs * 0.001f, { 8, 8 }, "><", RGB(0, 245, 0), TRUE);
}

void DrawFiringOverlay(int addx, int addy)
{
	//Tiny optimization, enable again if these values are no longer set before running a frame in the future
	//if (globals.OverlayDivider != 0 && globals.ResolutionDivider != 0)
	//{
	int XDivided = (int)round(((float)addx / globals.OverlayDivider) / globals.ResolutionDivider * globals.WindowDivider);

	XY coords;
	coords.x = (globals.OLCursorPos.x - XDivided) - 6; //nudge the origin 4 pixels so that the origin is actually in the center of the X since the origin is the top left coordinate and the X will be down and to the right

	int YDivided = (int)round(((float)addy / globals.OverlayDivider) / globals.ResolutionDivider * globals.WindowDivider);

	coords.y = (globals.OLCursorPos.y - YDivided) - 8; //nudge the origin 7 pixels so that the origin is actually in the center of the X since the origin is the top left coordinate and the X will be down and to the right
	//float OverlayTime = globals.VariableTime ? 0.1f : globals.CycleTime;
	//RenderTimedOverlay(OverlayTime, coords, "X", RGB(255, 0, 0), FALSE);
	RenderTimedOverlay(globals.CycleTime, coords, "X", RGB(255, 0, 0), FALSE, DEFAULT_FONT);
	globals.OLCursorPos.x -= XDivided;
	globals.OLCursorPos.y -= YDivided;
	//}
}

