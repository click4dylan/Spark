#include "stdafx.h"
#include "Mouse.h"
#include "Spark.h"
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <float.h>
#include "ScriptFile.h"
#include "SparkOverlays.h"
#include <math.h>
#include "clock.h"
#include "ConsoleCommands.h"
#include "Hotkeys.h"
#define round(number) floor(number + 0.5f);
float lasttimeleftclickeddown = 0.0f;

//Called as soon as the player first left clicks
void CallOnStartLeftClickDown()
{
	globals.done_resetting_cursor = 1;
	//Tell the red X overlay where our starting cursor position was
	GetCursorPos(&globals.OLCursorPos);
	ClearResetCursorVars();
	if (globals.AutoClicker)
	{
		//Set next time to autoclick down
		globals.nexttime_ClickDown = globals.realtime;
	}

	//Set next time to fire a bullet to right now
	if (!globals.VariableTime)
		globals.nexttime_firebullet = globals.realtime + globals.CycleTime;
	else
		globals.nexttime_firebullet = globals.realtime + globals.MouseSprayPattern[1].timeuntilnext;

	globals.maximum_movement.x = 0;
	globals.maximum_movement.y = 0;

	//Tell other threads we are clicking the left mouse button
	globals.HoldingLMB = 1;
	CalledOnLeftClick_Down = 1;
	CalledOnLeftClick_Up = 0;
}

//Called as soon as the player first lets go of the left mouse button
void CallOnStartLeftClickUp()
{
	if (globals.MaxBullets > 1)
	{
		//Don't reset cursor for weapons that only have 1 bullet, for crosshair use
		InitializeResetCursorPos();
	}
	ClearBulletsShot();
	globals.HoldingLMB = 0;
	CalledOnLeftClick_Up = 1;
	CalledOnLeftClick_Down = 0;
}

void FireBullet()
{
	POINT MouseMoveAmt = { 0, 0 };
	int MoveMouseFlags = 0;
	float realtime = globals.realtime;
	if (!globals.UseRemainderTimer || realtime >= globals.nexttime_firebullet)
	{
		//Increment bullets fired counter
		globals.bulletsfired = min(globals.bulletsfired + 1, globals.MaxBullets);

		if (GET_CONVAL_INT(&debug_script))
		{
			if (GET_CONVAL_INT(&render_debug))
			{
				//Draw on screen the mouse sample we are currently at
				DrawMouseSampleOverlay();
			}
			//If there is a hard cap on the mouse samples, then don't compensate for recoil any more
			if (globals.DebugMaxMouseSamples > 0 && globals.bulletsfired == globals.DebugMaxMouseSamples )
				return; //FIXME: What to do about the autoclicker?

			PrintBulletsFired();
		}

		//Get the next bullet after this one so that we can subtract coordinates from it
		int nextbullet = min(globals.bulletsfired + 1, globals.MaxBullets);

		//Possible speed improvement: Move sensitivity divisor into global vars
		float sensitivitydivisor = GET_CONVAL_FLOAT(&mouse_sensitivity) / globals.record_mouse_sensitivity;

		MouseMoveAmt.x = (int)round((globals.MouseSprayPattern[nextbullet].coords.x - globals.MouseSprayPattern[globals.bulletsfired].coords.x) / sensitivitydivisor);
		MouseMoveAmt.y = (int)round((globals.MouseSprayPattern[nextbullet].coords.y - globals.MouseSprayPattern[globals.bulletsfired].coords.y) / sensitivitydivisor);
			
		if (GET_CONVAL_INT(&enable_bullet_crosshair))
		{
			//Only draw the firing overlay if there are more than 1 mouse samples, so scripts like scout don't show the red X
			DrawFiringOverlay(MouseMoveAmt.x, MouseMoveAmt.y);
		}

		//POINT CursorPos;
		//GetCursorPos(&CursorPos);
		//SetCursorPos(CursorPos.x + addx, CursorPos.y + addy);
		//mouse_event(MOUSEEVENTF_MOVE, MouseMoveAmt.x, MouseMoveAmt.y, 0, 0);
		//Move the mouse
		MoveMouseFlags = MOUSEEVENTF_MOVE;

		//Now increment the maximum tracked mouse movement so that cursor resetting knows how much to move back
		globals.maximum_movement.x += MouseMoveAmt.x;
		globals.maximum_movement.y += MouseMoveAmt.y;

		SetNextFireBulletTime();
	}

	if (globals.AutoClicker)
	{
		//Handle auto clicking scripts
		if (realtime >= globals.nexttime_ClickDown)
		{
			MoveMouseFlags |= MOUSEEVENTF_LEFTDOWN;
			float delta = fmax(0.0f, realtime - globals.nexttime_ClickUp);
			globals.nexttime_ClickUp = realtime + globals.ClickDownTime - delta;
			globals.nexttime_ClickDown = realtime + 60.0f;
		}
		else if (realtime >= globals.nexttime_ClickUp)
		{
			MoveMouseFlags |= MOUSEEVENTF_LEFTUP;
			float delta = fmax(0.0f, realtime - globals.nexttime_ClickUp);
			globals.nexttime_ClickDown = realtime + globals.ClickUpTime - delta;
			globals.nexttime_ClickUp = realtime + 60.0f;
		}
	}

	if (MoveMouseFlags)
	{
		//Click or move the mouse
		mouse_event(MoveMouseFlags, MouseMoveAmt.x, MouseMoveAmt.y, 0, 0);
	}
}

//Possibly useful for the future
/*
INPUT    Input = { 0 };
// left up
ZeroMemory(&Input, sizeof(INPUT));
Input.type = INPUT_MOUSE;
Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
SendInput(1, &Input, sizeof(INPUT));
*/

void SetNextFireBulletTime()
{
	float realtime = globals.realtime;
	float maxDelta;
	float delta;
	if (!globals.VariableTime)
	{
		maxDelta = fmin(globals.INTERVAL_PER_FRAME, globals.CycleTime);
		delta = realtime - globals.nexttime_firebullet;
		delta = fmax(0.0f, delta);
		delta = fmin(delta, maxDelta);
		globals.nexttime_firebullet = realtime + globals.CycleTime - delta;
	}
	else
	{
		float timeuntilnext = globals.MouseSprayPattern[globals.bulletsfired].timeuntilnext;
		maxDelta = fmin(globals.INTERVAL_PER_FRAME, timeuntilnext);
		delta = realtime - globals.nexttime_firebullet;
		delta = fmax(0.0f, delta);
		delta = fmin(delta, maxDelta);
		globals.nexttime_firebullet = realtime + timeuntilnext - delta;
		//Set next time
		globals.TimerResolutionMS = max(1, (int)(roundf(timeuntilnext * 1000)));
	}
}

static bool PosX, PosY;
//Sets up Spark for moving the mouse back to the initial position when the player left clicked
void InitializeResetCursorPos()
{
	globals.done_resetting_cursor = 0;
	//Reverse maximum movement since we need to invert the movement to move the cursor back to the original position
	globals.incremental_movement.x = -globals.maximum_movement.x / 8;
	globals.incremental_movement.y = -globals.maximum_movement.y / 8;
	globals.movement_so_far.x = 0;
	globals.movement_so_far.y = 0;
	PosX = globals.maximum_movement.x > 0;
	PosY = globals.maximum_movement.y > 0;
}

void ResetCursorPos(float absmaxspeed)
{
	float realtime = globals.realtime;
	if (realtime >= globals.nexttime_resetcursor)
	{
		int absmoveinc = abs(globals.movement_so_far.x) + abs(globals.incremental_movement.x);
		int absmove = abs(globals.maximum_movement.x);

		if (absmoveinc > absmove)
		{
			globals.incremental_movement.x = absmove - abs(globals.movement_so_far.x);
			if (PosX)
				globals.incremental_movement.x = -globals.incremental_movement.x;
			absmoveinc = abs(globals.movement_so_far.x) + abs(globals.incremental_movement.x);
		}

		absmoveinc = abs(globals.movement_so_far.y) + abs(globals.incremental_movement.y);
		absmove = abs(globals.maximum_movement.y);

		if (absmoveinc > absmove)
		{
			globals.incremental_movement.y = absmove - abs(globals.movement_so_far.y);
			if (PosY)
				globals.incremental_movement.y = -globals.incremental_movement.y;
		}

		/*
		//This code is terrible, but if for any case the above code stops working, this works. I haven't seen the above code fail at all.
		while (abs(globals.movement_so_far.x) + abs(globals.incremental_movement.x) > abs(globals.maximum_movement.x))
		{
		globals.incremental_movement.x -= PosX ? -1 : 1;
		}

		while (abs(globals.movement_so_far.y) + abs(globals.incremental_movement.y) > abs(globals.maximum_movement.y))
		{
		globals.incremental_movement.y -= PosY ? -1 : 1;
		}
		*/

		if (globals.incremental_movement.x == 0 && globals.incremental_movement.y == 0)
		{
			//All done!
			ClearResetCursorVars();
			globals.done_resetting_cursor = 1;
		}
		else
		{
			globals.movement_so_far.x += globals.incremental_movement.x;
			globals.movement_so_far.y += globals.incremental_movement.y;

			//POINT curpos;
			//GetCursorPos(&curpos);
			//int addx = curpos.x + globals.incremental_movement.x;
			//int addy = curpos.y + globals.incremental_movement.y;
			//SetCursorPos(addx, addy);
			mouse_event(MOUSEEVENTF_MOVE, globals.incremental_movement.x, globals.incremental_movement.y, 0, 0);
			float maxDelta = fmin(absmaxspeed, globals.nexttime_resetcursor);
			float delta = fmax(0.0f, realtime - globals.nexttime_resetcursor);
			delta = fmin(delta, maxDelta);
			globals.nexttime_resetcursor = realtime + GET_CONVAL_FLOAT(&mouse_reset_speed) - delta;
		}
	}
}

void ClearResetCursorVars()
{
	globals.incremental_movement.x = 0;
	globals.incremental_movement.y = 0;
	globals.movement_so_far.x = 0;
	globals.movement_so_far.y = 0;
}

//Records mouse samples
void RecordMouse()
{
	if (globals.VariableTime || globals.bulletsfired <= globals.MaxBullets)
	{
		POINT mouse;
		GetCursorPos(&mouse);
		if (globals.VariableTime)
		{
			//Variable Time Recording Method
			if (!RecordVariableMouseSample(mouse))
			{
				return; //Don't save this mouse sample
			}
		}
		else
		{
			//CycleTime Recording Method
			if (!RecordCycleTimeMouseSample(mouse))
			{
				return; //Don't save this mouse sample
			}
		}
		//Save Mouse Sample
		SaveMouseSample(mouse);
	}
	else
	{
		FinishRecordingCycleTimeScript();
	}
}

//Saves the recorded mouse sample into the buffer
void SaveMouseSample(POINT& mouse)
{
	globals.NumMouseSamples++;
	globals.MouseSprayPattern[globals.bulletsfired].bullet = globals.bulletsfired;
	globals.MouseSprayPattern[globals.bulletsfired].coords.x = mouse.x;
	globals.MouseSprayPattern[globals.bulletsfired].coords.y = mouse.y;
	globals.MouseSprayPattern[globals.bulletsfired].timeuntilnext = 0;
	DEBUG_PRINT("Recorded Sample %i: %li, %li\n", globals.NumMouseSamples, mouse.x, mouse.y);
}

//Records a variable mouse sample, returns 1 if it should save the sample, 0 if it should not
int RecordVariableMouseSample(POINT& mouse)
{
	if (bCoordsIdentical(mouse, globals.LastRecordingCursorPos))
	{
		//Don't record this mouse sample since the mouse position didn't change!
		return 0;
	}
	else
	{
		globals.bulletsfired++;
		if (globals.bulletsfired > MAX_MOUSE_SAMPLES)
		{
			printf("Warning: MAX_MOUSE_SAMPLES hit! Recording cannot continue.");
			FinishedRecordingVariableScript();
			return 0;
		}
		float curtime = (float)GetCurTime();
		//First bullet doesn't have a time yet
		if (globals.bulletsfired > 1)
		{
			//Set the time between the current sample and the last sample to the previous sample
			int lastindex = globals.bulletsfired - 1;
			globals.MouseSprayPattern[lastindex].timeuntilnext = curtime - globals.lasttime_mousechanged_recording;
		}
		globals.lasttime_mousechanged_recording = curtime;
		globals.LastRecordingCursorPos = mouse;
	}
	return 1;
}

//Records a cycle time mouse sample, returns 1 if it should save the sample, 0 if it should not
int RecordCycleTimeMouseSample(POINT& mouse)
{
	float curtime = (float)GetCurTime(); //Still unsure, should we access globals.realtime or do this?
	if (curtime < globals.nexttime_recordmouse)
	{
		return 0; //Don't record this mouse sample since it is not time yet
	}
	globals.bulletsfired++;
	float maxDelta = fmin(globals.INTERVAL_PER_FRAME, globals.CycleTime);
	float delta = curtime - globals.nexttime_recordmouse;
	delta = fmax(0.0f, delta);
	delta = fmin(delta, maxDelta);
	globals.nexttime_recordmouse = curtime + globals.CycleTime - delta;
	return 1;
}

//Stops and saves the recording cycle time script
void FinishRecordingCycleTimeScript()
{
	printf("Mouse recording complete, number of samples %i\n", globals.NumMouseSamples);
	ClearBulletsShot();
	globals.recording = 0;
	globals.lasttime_mousechanged_recording = 0;
	WriteWeaponConfig();
}

//Stops and saves the recording variable script
void FinishedRecordingVariableScript()
{
	globals.MaxBullets = globals.NumMouseSamples;
	printf("Mouse variable recording complete, number of samples: %i\n", globals.MaxBullets);
	ClearBulletsShot();
	globals.recording = 0;
	WriteWeaponConfig();
}

//Starts a re-recording session to quickly re-record the current script file without having to type out all the information again
void QuickReRecordScript()
{
	if (strlen(globals.WeaponName) > 0)
	{
		if (strcmp(globals.WeaponName, "None"))
		{
			printf("Now re-recording the current script. Press shift if you would like to do this or press space to cancel.\n");
			globals.PreStartRecord = 1;
		}
	}
}

void StartRecording()
{
	printf("Mouse recording started. Hold Left click to record or press space to cancel.\n");
	if (globals.VariableTime)
	{
		globals.MaxBullets = 0;
		printf("Press enter to complete and save the script\n");
	}
	ClearBulletsShot();
	globals.PreStartRecord = 0;
	globals.recording = 1;
	globals.NumMouseSamples = 0;
	globals.lasttime_mousechanged_recording = 0;
	globals.time_since_started_recording = globals.realtime;
}

void CancelRecording()
{
	globals.recording = 0; //Already 0 if in PreStartRecord
	globals.PreStartRecord = 0;
	globals.lasttime_mousechanged_recording = 0;
	printf("Mouse recording cancelled\n");
}

//Unused for now
void InitializeRawInput()
{
	/*
	RAWINPUTDEVICE Rid[2];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = 0;

	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = RIDEV_NOLEGACY;   // adds HID keyboard and also ignores legacy keyboard messages
	Rid[1].hwndTarget = 0;

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error
		printf("FAILED TO INITIALIZE RAW INPUT!");
		Sleep(2000);
		exit(-1);
	}
	*/
}