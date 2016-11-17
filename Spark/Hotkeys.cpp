#include "stdafx.h"
#include "Mouse.h"
#include "Hotkeys.h"
#include "ConsoleCommands.h"
#include "ScriptFile.h"
#include "SparkOverlays.h"

BOOL CalledOnLeftClick_Up = 0, CalledOnLeftClick_Down = 0;

void ListenForHotkeys()
{
	if (!globals.paused)
	{
		if (globals.recording || !globals.AutoClicker)
		{
			//Left Mouse Button
			if (GetAsyncKeyState(VK_LBUTTON) >> 8)
			{
				if (!CalledOnLeftClick_Down)
				{
					CallOnStartLeftClickDown();
				}
			}
			else
			{
				if (!CalledOnLeftClick_Up)
				{
					CallOnStartLeftClickUp();
				}
			}
		}
		else if (GetAsyncKeyState(VK_XBUTTON1) >> 8 || GetAsyncKeyState(VK_XBUTTON2) >> 8)
		//Autoclicker buttons
		{
			if (!CalledOnLeftClick_Down)
			{
				CallOnStartLeftClickDown();
			}
		}
		else
		{
			if (!CalledOnLeftClick_Up)
			{
				CallOnStartLeftClickUp();
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); //Let go of LMB during autoclick
			}
		}

		if (!globals.recording)
		{
			if (!globals.PreStartRecord)
			{
				if (GET_CONVAL_INT(&enable_quick_record))
				{
					if ((GetAsyncKeyState(VK_CONTROL) >> 8 && GetAsyncKeyState(VK_DELETE) >> 8))
					{
						QuickReRecordScript();
					}
				}
				if (globals.GameIsActiveWindow)
				{
					//Handle in-game weapon change overlay
					if (globals.InChangeWeaponOverlay)
					{
						if (GetAsyncKeyState(VK_ESCAPE) >> 8)
						{
							//Cancel the weapon switch overlay
							globals.ChangeWeaponOverlayText[0] = 0;
							globals.ChangeWeaponOverlayTextLength = 0;
							globals.InChangeWeaponOverlay = 0;
						}
						else if (GetAsyncKeyState(VK_RETURN) >> 8)
						{
							//change weapon
							if (globals.ChangeWeaponOverlayTextLength > 0)
							{
								strcpy(globals.WeaponName, globals.ChangeWeaponOverlayText);
								if (ReadWeaponConfig())
								{
									RenderTimedOverlay(0.7, XY(100, 75), "SUCCESS!", RGB(0, 255, 0), 1, BIG_FONT);
								}
								else
								{
									RenderTimedOverlay(0.7, XY(100, 75), "ERROR!", RGB(255, 0, 0), 1, BIG_FONT);
								}
							}
							//stop trying to change weapon
							globals.ChangeWeaponOverlayText[0] = 0;
							globals.ChangeWeaponOverlayTextLength = 0;
							globals.InChangeWeaponOverlay = 0;
						}
						else
						{
							static float LastTimePressedBackspace_WeaponSwitchMenu = 0.0f;
							static short LastButtonsPressed_WeaponSwitchMenu[91]; //List of keys pressed
							static float LastButtonTimePressed_WeaponSwitchMenu[91];  //Last time pressed that key

							if (GetAsyncKeyState(VK_BACK) >> 8)
							{
								//Pressed Backspace
								if (globals.ChangeWeaponOverlayTextLength > 0 && globals.realtime - LastTimePressedBackspace_WeaponSwitchMenu >= 0.2f)
								{
									globals.ChangeWeaponOverlayTextLength--;
									globals.ChangeWeaponOverlayText[globals.ChangeWeaponOverlayTextLength] = 0;
									globals.ChangeWeaponOverlayText[globals.ChangeWeaponOverlayTextLength - 1] = 0;
									LastTimePressedBackspace_WeaponSwitchMenu = globals.realtime;
								}
							}
							else
							{
								//Check other keys
								short keys[91];
								for (short x = 48; x <= 90; x++)
								{
									keys[x] = (GetAsyncKeyState(x) >> 8);
									if (keys[x])
									{
										char actualkey[2];
										if (x < 65) {
											//Number keys match exactly
											actualkey[0] = (char)x;
										}
										else {
											//Letters need + 32
											actualkey[0] = (char)x + 32;
										}
										actualkey[1] = 0;
										if (globals.ChangeWeaponOverlayTextLength + 1 < sizeof(globals.ChangeWeaponOverlayText))
										{
											if (LastButtonsPressed_WeaponSwitchMenu[x] && globals.realtime - LastButtonTimePressed_WeaponSwitchMenu[x] >= 0.2f)
											{
												//If we are pressing the same key as last time, only allow a keypress after 0.2 secs
												strcat(globals.ChangeWeaponOverlayText, actualkey);
												globals.ChangeWeaponOverlayTextLength++;
												globals.ChangeWeaponOverlayText[globals.ChangeWeaponOverlayTextLength] = 0;
												LastButtonTimePressed_WeaponSwitchMenu[x] = globals.realtime; //Set time we pressed this key
												LastTimePressedBackspace_WeaponSwitchMenu = 0; //Let us press backspace immediately after this key
											}
										}
									}
									else
									{
										//Not pressing this key, reset the last button state time
										LastButtonTimePressed_WeaponSwitchMenu[x] = 0;
									}
									//Update last button states
									LastButtonsPressed_WeaponSwitchMenu[x] = keys[x];
								}
							}
							
							char OverlayText[350] = "Load Script: \n";
							if (globals.ChangeWeaponOverlayTextLength > 0)
							{
								strcat(OverlayText, globals.ChangeWeaponOverlayText);
							}
							DrawTextOverlay(XY(95, 70), OverlayText, RGB(0, 0, 0), 1, BIG_FONT); //Black
							DrawTextOverlay(XY(97, 72), OverlayText, RGB(136, 0, 21), 1, BIG_FONT); //Dark Red
							DrawTextOverlay(XY(99, 74), OverlayText, RGB(255, 0, 0), 1, BIG_FONT); //Red
						}
					}
					else if (GetAsyncKeyState(VK_DELETE) >> 8 && GetAsyncKeyState(VK_END) >> 8 && GetAsyncKeyState(VK_NEXT) >> 8)
					{
						//Enable in-game weapon switching overlay
						globals.InChangeWeaponOverlay = 1;
					}
				}
			}
			if (globals.PreStartRecord)
			{
				//If currently waiting to start recording, check if we should cancel or start the recording
				if (GetAsyncKeyState(VK_SPACE) >> 8)
				{
					CancelRecording();
				}
				else if (GetAsyncKeyState(VK_SHIFT) >> 8)
				{
					StartRecording();
				}
			}
		}
		else
		{
			//Recording
			if (GetAsyncKeyState(VK_SPACE) >> 8)
			{
				CancelRecording();
				ClearBulletsShot();
			}
			else if (globals.VariableTime)
			{
				if (GetAsyncKeyState(VK_RETURN) >> 8)
				{
					//For some reason, as soon as we press SHIFT to start recording, windows says we are pressing RETURN, so for now just ignore it
					if (globals.realtime - globals.time_since_started_recording > 0.25f)
					{
						//Completed variable script
						FinishedRecordingVariableScript();
					}
				}
			}
		}
	}

	if (GetAsyncKeyState(VK_CONTROL) >> 8 && GetAsyncKeyState(VK_CAPITAL) >> 8)
	{
		TogglePause();
		Sleep(300);
	}
}