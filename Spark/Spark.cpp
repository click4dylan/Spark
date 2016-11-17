// Spark.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "clock.h"
#include "sparkglobals.h"
#include "UsefulFunctions.h"
#include "Spark.h"
#include "SparkOverlays.h"
#include "ConsoleCommands.h"
#include "Mouse.h"
#include "Hotkeys.h"
#include "ConfigFile.h"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <float.h>
#include <thread>
#include "api/baseapi.h"
globalsdefs globals;

#define PROFILE 0

int _tmain(int argc, const wchar_t *argv[])
{
	globals.UseRemainderTimer = 1;
	if (argc == 2)
	{
		if (! _tcscmp(argv[1], _T("-alternativetimer")))
			globals.UseRemainderTimer = 0;
	}
	Initialize();
	std::thread AsyncSlow(AsyncSlowLoop);
	AsyncSlow.detach();
	std::thread AsyncVariable(AsyncVariableLoop);
	AsyncVariable.detach();
	std::thread AsyncFast(AsyncFastLoop);
	AsyncFast.detach();

	if (globals.UseRemainderTimer)
	{
		double curtime = Plat_FloatTime(), lasttime = Plat_FloatTime();
		float mainloop_frametime = 0, time_remainder = 0;
		while (!globals.quit)
		{
			curtime = Plat_FloatTime();
			mainloop_frametime += (float)(curtime - lasttime);
			lasttime = curtime;
			if (ShouldRunFrame(mainloop_frametime))
			{
				globals.realtime += mainloop_frametime; //Increment time since program started
				time_remainder += mainloop_frametime;
				globals.frametime = mainloop_frametime; //Set time between frames so we can calculate FPS
				int numframes = 0; //how many times we will run a frame this frame
				if (time_remainder >= globals.INTERVAL_PER_FRAME)
				{
					numframes = (int)(time_remainder / globals.INTERVAL_PER_FRAME);
					time_remainder -= numframes * globals.INTERVAL_PER_FRAME;
				}
				globals.MIN_INTERVAL_PER_FRAME = globals.INTERVAL_PER_FRAME - time_remainder;
				for (int frame = 0; frame < numframes; frame++)
				{
#if PROFILE
					static int numframes;
					static double totaltime;
					double curtime1 = Plat_FloatTime();
#endif
					RunFrame(); //Run a frame
#if PROFILE
					double curtime2 = Plat_FloatTime();
					numframes++;
					totaltime += curtime2 - curtime1;
					if (numframes == 10000) {
					printf("10000 frames = %f secs\n", totaltime);
					numframes = 0;
					totaltime = 0;
					}
#endif
				}
				mainloop_frametime = 0; //Reset for next frame
			}
			else
			{
				//Running too fast
				Sleep(1);
			}
		}
	}
	else
	{
		while (!globals.quit)
		{
			if (!globals.paused)
			{
				RunFrame(); //Run a frame
			}
			Sleep(globals.TimerResolutionMS);
		}
	}
	return 0;
}

inline bool ShouldRunFrame(float dt)
{
	return (!globals.UseRemainderTimer && globals.recording) || !globals.paused && globals.GameIsActiveWindow && dt >= globals.MIN_INTERVAL_PER_FRAME;
}

void SetTitleBarText()
{
	char szNewTitle[MAX_PATH];
	sprintf(szNewTitle, "Spark " SPARK_VERSION "    SCRIPT : %s    ", globals.WeaponName);
	if (globals.paused)
	{
		strcat(szNewTitle, "*PAUSED*");
	}
	else if (globals.UseRemainderTimer)
	{
		double ft = fmax(0.000001, globals.frametime);
		char fpsstr[32];
		sprintf(fpsstr, "FPS: %.1f", 1 / ft);
		strcat(szNewTitle, fpsstr);
	}
	SetConsoleTitleA(szNewTitle);
}

void Initialize()
{
	StoreOriginalTimerResolution();
#if DEBUG
	printf("Original Timer Resolution: %f ms\n", GetTimerResolution());
	printf("Max Timer Resolution: %f ms\n", GetMaxTimerResolution());
#endif
	SetTimerResolution(fmax(1.0f, GetMaxTimerResolution())); //Faster than 1ms increases DPC latency too much
#if DEBUG
	printf("Timer Resolution is now set to %f ms\n", GetTimerResolution());
#endif
	InitTime();

	globals.AllowRecording = 1;

	hinput = GetStdHandle(STD_INPUT_HANDLE);
	houtput = GetStdHandle(STD_OUTPUT_HANDLE);
	// Force character width and character height
	SetConsoleCXCY(houtput, 81, 23);
	PrintStartupText();

	RegisterConsoleCommands();

	//Initialize global variables
	globals.WindowDivider = 1.0f;
	globals.INTERVAL_PER_FRAME = DEFAULT_INTERVAL_PER_FRAME;
	globals.resetcursor = 1;
	globals.done_resetting_cursor = 1;
	globals.TimerResolutionMS = 30;
	globals.frametime = 1.0f;
	strcpy(globals.WeaponName, "None");
	strcpy(globals.TargetWindowTitle, "any");
	SetTitleBarText();

	if (!ReadConfigFile())
	{
		WriteConfigFile();
	}
	//InitializeRawInput();
}

//Code that doesn't need to run fast gets run in here
void AsyncSlowLoop()
{
	while (!globals.quit)
	{
		//Read and process console commands
		ProcessConsoleInput();
		if (!globals.paused)
		{
			GetDesktopResolution(globals.ScreenRes.x, globals.ScreenRes.y);
			globals.ScreenRes_AspectRatio = (float)globals.ScreenRes.x / (float)globals.ScreenRes.y;
			GetOverlayDivider();
			GetResolutionDivider();
			layhWnd = GetForegroundWindow();
			GetWindowTextA(layhWnd, globals.ActiveWindowTitle, sizeof(globals.ActiveWindowTitle));
			//Todo: make this more elegant
			if (!strcmp(globals.ActiveWindowTitle, globals.TargetWindowTitle)) {
				globals.GameIsActiveWindow = 1;
			} else if(!strcmp(globals.TargetWindowTitle, "any")) {
				globals.GameIsActiveWindow = 1;
			} else if (!strcmp(globals.ActiveWindowTitle, "Untitled - Paint")) {
				globals.GameIsActiveWindow = 1;
			} else {
				globals.GameIsActiveWindow = 0;
			}
			GetClientRect(layhWnd, &clientrect);
			GetClientRect(layhWnd, &clientrect_unmodified);
			//globals.WindowAspectRatio = (float)clientrect.right / (float)clientrect.bottom;
			globals.ScreenRes_Sum = (float)(globals.ScreenRes.x + globals.ScreenRes.y);
			globals.ScreenRes_Difference = (float)(globals.ScreenRes.x - globals.ScreenRes.y);
			GetWindowRect(layhWnd, &windowrect);
			if (clientrect.bottom != windowrect.bottom)
			{
				//User has bordered window enabled! Compensate for it.
				globals.IsBorderedWindow = 1;
				globals.WindowBorder.x = (LONG)(((windowrect.right - windowrect.left) - (clientrect_unmodified.right - clientrect_unmodified.left)) * 0.5);
				globals.WindowBorder.y = (LONG)(((windowrect.bottom - windowrect.top) - (clientrect_unmodified.bottom - clientrect_unmodified.top)) / 1.136363636363636f); //- 3; //Don't ask about this weird number, not sure why it's required yet
				GetWindowDivider();
			}
			else if (globals.IsBorderedWindow)
			{
				globals.IsBorderedWindow = 0;
				globals.WindowBorder.x = globals.WindowBorder.y = 0;
				globals.WindowDivider = 1.0f;
			}

			//if (!globals.paused && !globals.recording && !globals.HoldingLMB && globals.enable_center_crosshair)
			//{
				//DrawCenterCrosshair(100);
			//}
			if (globals.UseRemainderTimer)
				SetTitleBarText();
		}
		Sleep(50);
	}
}

//TODO: Variable time thread
void AsyncVariableLoop()
{
	while (!globals.quit)
	{
		if (!globals.done_resetting_cursor && globals.resetcursor && !globals.paused && !globals.recording && !globals.HoldingLMB)
		{
			ResetCursorPos((5.0f / 1000.0f)); //Change to the value of Sleep
		}
		Sleep(5);
	}
}

//Time-critical tasks that are run asynchronously
void AsyncFastLoop()
{
	while (!globals.quit)
	{
		ListenForHotkeys();
		Sleep(1);
	}
}

__forceinline void RunFrame()
{
	if (!globals.recording)
	{
		//Only call FireBullet if there are more than 1 mouse samples so single shot weapons continue rendering the crosshair while firing
		if (globals.HoldingLMB && globals.NumMouseSamples > 1)
		{
			//We are spraying
			FireBullet();
			//globals.lasttime_pressedLMB = globals.realtime;
		}
		else
		{
			//Idle
			if (GET_CONVAL_INT(&enable_center_crosshair) && !globals.InChangeWeaponOverlay)
			{
				DrawCenterCrosshair();
			}
		}
		if (GET_CONVAL_INT(&debug_script))
		{
			HandleDebugKeys();
		}
	}
	else if (globals.HoldingLMB)
	{
		//Recording
		RecordMouse();
	}
}

void HandleDebugKeys()
{
	//If we press keypad + or keypad - then set the hard limit for the mouse samples. Spark will not move the mouse past this sample number
	if (GetAsyncKeyState(KPP))
	{
		globals.DebugMaxMouseSamples++;
		if (globals.DebugMaxMouseSamples > globals.MaxBullets)
		{
			globals.DebugMaxMouseSamples = globals.MaxBullets;
		}
		DrawDebugOverlay();
	}
	else if (GetAsyncKeyState(KPM))
	{
		globals.DebugMaxMouseSamples--;
		if (globals.DebugMaxMouseSamples < -1)
		{
			globals.DebugMaxMouseSamples = -1;
		}
		DrawDebugOverlay();
	}
}

inline void GetOverlayDivider()
{
	float mouse_sensitivity_capped = GET_CONVAL_FLOAT(&mouse_sensitivity);
	if (mouse_sensitivity_capped < 0.5f) {
		mouse_sensitivity_capped = 0.5f;
		printf("Warning: sensitivity < 0.5! Crosshair won't be accurate!\n");
	}
	globals.OverlayDivider = (6 + globals.ScreenRes_AspectRatio) / mouse_sensitivity_capped;  //24 max sensitivity / mousesensitivity
}

inline void GetResolutionDivider()
{
	//1920-1080 = 840 (recorded resolution)
	//800-600 = 200 (game resolution)
	//840 / 200 = 4.2
	//4.2 / 2 = 2.1
	
	//Avoid dividing by 0, only do this if script file is loaded
	if (globals.NumMouseSamples != 0)
	{
		float difference = globals.RecordedScreenRes_Difference / globals.ScreenRes_Difference;
		if (difference == 1.0 || difference <= globals.RecordedScreenRes_AspectRatio)  //screen resolution is the same as the recording resolution
		{
			globals.ResolutionDivider = difference;
		}
		else
		{
			//if difference = 1.2 then return 1.2
			//if difference = 1.333 then return 1.333
			//if difference = 4.2 then return 4.2 / 2
			//if difference = 2.625 then return 2.625 / 2
			globals.ResolutionDivider = (difference * 0.5f);
		}
	}
}

void GetWindowDivider()
{
	float pctdecreaserecordedtocurrent = (globals.RecordedScreenRes_Sum - globals.ScreenRes_Sum) / globals.RecordedScreenRes_Sum;
	float windowadd = (float)(clientrect_unmodified.right + clientrect_unmodified.bottom);
	float pctdecrease = (globals.ScreenRes_Sum - windowadd) / globals.ScreenRes_Sum;
	globals.WindowDivider = !globals.IsBorderedWindow ? 1.0f : (1.0f + (2.0f * pctdecreaserecordedtocurrent) - pctdecrease);
}

//Prints debug information to the console
void DEBUG_PRINT(const char* string, ...)
{
	if (GET_CONVAL_INT(&debug_script))
	{
		va_list args;
		va_start(args, string);
		printf(string, args);
	}
}