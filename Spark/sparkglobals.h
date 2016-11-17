#ifndef globals_h
#define globals_h
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include "ScriptFile.h"

#define DEFAULT_INTERVAL_PER_FRAME (1.0f / 1024.0f)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / INTERVAL_PER_FRAME ) )
#define TICKS_TO_TIME( t )		( INTERVAL_PER_FRAME *( t ) )
#define ROUND_TO_TICKS( t )		( INTERVAL_PER_FRAME * TIME_TO_TICKS( t ) )

#define LMB 0x1 //Left mouse button
#define KPP 0x6B //Numpad Plus key
#define KPM 0x6D //Numpad Minus key

#define MAX_MOUSE_SAMPLES 2049

typedef POINT PointAPI;

struct XY {
	int x, y;
	XY::XY() {
		x = x;
		y = y;
	}
	XY::XY(int ix, int iy) {
		x = ix;
		y = iy;
	}
};

struct MouseSprayPattern {
	int bullet; //Index of mouse sample
	POINT coords; //X Y coordinates for this mouse sample
	float timeuntilnext; //Time between shots, used for Variable Recording only
};

struct globalsdefs {
	char ActiveWindowTitle[256]; //Window title of the currently active window
	char TargetWindowTitle[256]; //Window title of the target process
	int GameIsActiveWindow; //Target process is currently the active window
	int quit; //quit the application
	POINT LastRecordingCursorPos; //Last recording cursor position
	POINT OLCursorPos; //Starting Cursor position for the Red X Overlay. Gets set on the first bullet fired
	POINT ScreenRes; //The player's current screen resolution
	POINT RecordedScreenRes; //The screen resolution used during recording of the script file
	POINT WindowBorder; //Window Border width and height
	int  IsBorderedWindow; //Is the current window a window with a border
	float ScreenRes_AspectRatio; //X / Y
	float ScreenRes_Sum; //X + Y
	float ScreenRes_Difference; //X - Y
	float RecordedScreenRes_AspectRatio; //The script file's defined resolution aspect ratio X / Y
	float RecordedScreenRes_Sum; //X + Y
	float RecordedScreenRes_Difference; //X - Y
	float WindowAspectRatio; //X / Y of the game window
	float lasttime_mousechanged_recording; //The last time the mouse cursor moved during a recording
	float time_since_started_recording; //Time since recording started
	float OverlayDivider; //Used for getting the proper xy coordinates for the red x bullet crosshair
	float ResolutionDivider; //Used for getting the proper xy coordinates for the player's current screen resolution
	float WindowDivider; //Used for getting the proper xy coordinates for a bordered window
	float record_mouse_sensitivity; //Mouse sensitivity that the recorded script file uses
	float nexttime_resetcursor; //The next time the program should move the cursor to a position between the current position and the starting position
	int done_resetting_cursor; //Finished resetting the mouse back to the original position when the player started left click
	POINT maximum_movement; //Maximum mouse movement for resetting the cursor pos
	POINT incremental_movement; //How much to currently move the cursor pos when resetting
	POINT movement_so_far; //How much we have already moved the cursor when resetting
	int resetcursor; //Enable cursor resetting after letting go of the fire button
	int recording; //Currently recording
	int PreStartRecord; //User is in record mode but has not confirmed to start recording yet
	int paused; //Program is paused
	int DebugMaxMouseSamples; //Max mouse samples to use when firing a weapon
	float ScriptVersion; //Script File Version
	float ConfigVersion; //Config File Version
	char WeaponName[64]; //Current Weapon Name
	int InChangeWeaponOverlay; //Are we currently showing the change weapon overlay or not
	char ChangeWeaponOverlayText[64]; //Currently Typed New Weapon Name in the overlay
	int ChangeWeaponOverlayTextLength; //Length of currently typed text in new weapon name overlay
	int NumMouseSamples; //Number of recorded mouse samples
	int MaxBullets; //Maximum mouse samples
	int bulletsfired; //Current number of bullets fired
	int ScriptType; //ScriptTypes
	int AutoClicker; //Script Type is an autoclicker script
	int VariableTime; //Script Type is a variable script
	float CycleTime; //Weapon Cycle Time
	float ClickUpTime; //Time to keep auto clicker button up
	float ClickDownTime; //Time to keep auto clicker button down
	float nexttime_ClickDown; //Next time to autoclick
	float nexttime_ClickUp; //Next time to let go of autoclick
	float TimeToIdle; //Time from letting go of the fire button until the gun is fully accurate again
	float ReloadTime; //Time it takes to reload the weapon
	float INTERVAL_PER_FRAME;
	float MIN_INTERVAL_PER_FRAME;
	float realtime; //Current time
	float frametime;
	float nexttime_firebullet; //Next time to fire a bullet
	float nexttime_recordmouse; //Next time to record a mouse sample
	int TimerResolutionMS;
	int UseRemainderTimer; //Use Source Engine based tick method
	int HoldingLMB; //Holding the left mouse button
	double AllowRecording; //License allows recording or not
	MouseSprayPattern MouseSprayPattern[MAX_MOUSE_SAMPLES]; //starts from 1
};

#endif