#include "stdafx.h"
#include "UsefulFunctions.h"
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(LONG& horizontal, LONG& vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}