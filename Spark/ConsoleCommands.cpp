#include "stdafx.h"
#include "ConsoleCommands.h"
#include "Spark.h"
#include "ScriptFile.h"
#include "Mouse.h"
#include <iostream>

void PrintStartupText()
{
	printf(
	"Spark by Dylan Hughes (click4dylan@live.com). NOT FOR DISTRIBUTION!\n\n"
	"Console Commands:\n sensitivity, toggleredx, togglegreenx, debugscript, renderdebug,\n"
	" record, allowquickrecord, pause, exit, quit, editscript, mouse_reset_speed"
	"\n\n"

	"Hotkeys:\n"
	" DELETE + END + PAGEDOWN: Enter new weapon script while playing,\n"
	" CTRL + CAPSLOCK:\t  Pause the program\n"
	" CTRL + DELETE:\t\t  Re-record currently selected script file (Requires\n"
	"\t\t\t  \"allowquickrecord\" command enabled) to function.\n"
	"\n"
	"Be sure to match the command \"sensitivity\" to the sensitivity you use in-game.\n"
	"\n"
	"Windowed Fullscreen (Borderless Window Mode) is recommended, with Aero theme\n"
	"  disabled and Vsync Off.\n"
	"\n"
	"The crosshair might not render properly in Windows 8 and higher without\n"
	" disabling Desktop Window Manager.\n"
	"_______________________________________________________________________________\n"
	"Type console commands below, or the name of a script file to load it:\n"
	);
}

cvar_t mouse_sensitivity = {
	"sensitivity",
	"Set this to the same as the sensitivity value you use in the game.",
	CVAR_TYPES::CVAR_FLOAT,
	CVAR_FLAGS::FL_NONE,
	ALLOC_CONVAL(2.0f)
};

cvar_t enable_bullet_crosshair = {
	"toggleredx",
	"Toggles rendering the red X crosshair used to know where the bullets will land",
	CVAR_TYPES::CVAR_INT,
	CVAR_FLAGS::FL_TOGGLE,
	ALLOC_CONVAL(1)
};

cvar_t enable_center_crosshair = {
	"togglegreenx",
	"Toggles rendering the center green X crosshair. Useful for weapons with no crosshair",
	CVAR_TYPES::CVAR_INT,
	CVAR_FLAGS::FL_TOGGLE,
	ALLOC_CONVAL(1)
};

cvar_t enable_quick_record = {
	"allowquickrecord",
	"Toggles the ctrl+delete hotkey that allows you to quickly re-record the currently selected script",
	CVAR_TYPES::CVAR_INT,
	CVAR_FLAGS::FL_TOGGLE,
	ALLOC_CONVAL(0)
};

cvar_t record = {
	"record",
	"Records a new weapon script file",
	CVAR_TYPES::CON_COMMAND,
	CVAR_FLAGS::FL_NONE,
	&RecordCommand
};

cvar_t debug_script = {
	"debugscript",
	"Toggles script debugging and printing the current bullet number. Press Numpad + or - to cap the max mouse samples when firing",
	CVAR_TYPES::CVAR_INT,
	CVAR_FLAGS::FL_TOGGLE,
	ALLOC_CONVAL(0)
};

cvar_t render_debug = {
	"renderdebug",
	"Toggles rendering weapon script debug info on-screen",
	CVAR_TYPES::CVAR_INT,
	CVAR_FLAGS::FL_TOGGLE,
	ALLOC_CONVAL(0)
};

cvar_t pause = {
	"pause",
	"Toggles pausing of the entire program",
	CVAR_TYPES::CON_COMMAND,
	CVAR_FLAGS::FL_NONE,
	&TogglePause
};

cvar_t quitcmd = {
	"quit",
	"Quits the program",
	CVAR_TYPES::CON_COMMAND,
	CVAR_FLAGS::FL_NONE,
	&Quit
};

cvar_t exitcmd = {
	"exit",
	"Exits the program",
	CVAR_TYPES::CON_COMMAND,
	CVAR_FLAGS::FL_NONE,
	&Quit
};

cvar_t editscript = {
	"editscript",
	"Edits a script file",
	CVAR_TYPES::CON_COMMAND,
	CVAR_FLAGS::FL_NONE,
	&EditScriptFileCommand
};

cvar_t edit = {
	"edit",
	"Edits a script file",
	CVAR_TYPES::CON_COMMAND,
	CVAR_FLAGS::FL_NONE,
	&EditScriptFileCommand
};

cvar_t mouse_reset_speed = {
	"mouse_reset_speed",
	"The speed in seconds at which to reset the mouse back to the starting position after letting go of the fire button",
	CVAR_TYPES::CVAR_FLOAT,
	CVAR_FLAGS::FL_NONE,
	ALLOC_CONVAL(0.025f)
};

std::vector<cvar_t*> cvars; //Global console command list
StringList *cmd_argv; //Contains the current console command arguments

//Returns the number of current console command arguments
unsigned int cmd_argc()
{
	return cmd_argv->GetSize();
}

//Prints convar description
void PRINT_CONDESC(cvar_t* cvar)
{
	std::cout << "Description: " << cvar->description << std::endl;
}

//Prints convar value
void PRINT_CONVAL(cvar_t* cvar)
{
	std::cout << "Value: ";
	switch (cvar->type)
	{
		case CVAR_TYPES::CVAR_BOOL:
		{
			std::cout << *(bool*)cvar->value << std::endl;
			break;
		}
		case CVAR_TYPES::CVAR_INT:
		{
			std::cout << *(int*)cvar->value << std::endl;
			break;
		}
		case CVAR_TYPES::CVAR_FLOAT:
		{
			std::cout << *(float*)cvar->value << std::endl;
			break;
		}
		case CVAR_TYPES::CVAR_DOUBLE:
		{
			std::cout << *(double*)cvar->value << std::endl;
			break;
		}
		case CVAR_TYPES::CVAR_STRING:
		{
			std::cout << *(char*)cvar->value << std::endl;
			//strcpy((char*)cvar->value, cmd_argv->Get(1));
			break;
		}
	}
}

//Sets convar value
void SET_CONVAL(cvar_t* cvar, const char* value, bool verbose)
{
	switch (cvar->type) {
		case CVAR_TYPES::CVAR_BOOL:
		{
			*(bool*)cvar->value = (bool)atoi(value);
			if (verbose)
				printf("%s is now %i\n", cvar->name, *(bool*)cvar->value);
			break;
		}
		case CVAR_TYPES::CVAR_INT:
		{
			*(int*)cvar->value = atoi(value);
			if (verbose)
				printf("%s is now %i\n", cvar->name, *(int*)cvar->value);
			break;
		}
		case CVAR_TYPES::CVAR_FLOAT:
		{
			*(float*)cvar->value = (float)atof(value);
			if (verbose)
				printf("%s is now %f\n", cvar->name, *(float*)cvar->value);
			break;
		}
		case CVAR_TYPES::CVAR_DOUBLE:
		{
			*(double*)cvar->value = atof(value);
			if (verbose)
				printf("%s is now %f\n", cvar->name, *(double*)cvar->value);
			break;
		}
		case CVAR_TYPES::CVAR_STRING:
		{
			strcpy((char*)cvar->value, value);
			if (verbose)
				printf("%s is now %s\n", cvar->name, (char*)cvar->value);
			break;
		}
	}
}

//Toggles a convar value
void TOGGLE_CONVAL(cvar_t* cvar)
{
	*(int*)cvar->value ^= 1;
	printf("%s is now %i\n", cvar->name, *(int*)cvar->value);
}

//Adds a new console command to the command list
void AddConsoleCommand(cvar_t *cvar)
{
	cvars.push_back(cvar);
}

//Registers all the console commands
void RegisterConsoleCommands()
{
#ifdef KEYGEN
	AddConsoleCommand(&generateserial);
#endif
	AddConsoleCommand(&mouse_sensitivity);
	AddConsoleCommand(&enable_bullet_crosshair);
	AddConsoleCommand(&enable_center_crosshair);
	AddConsoleCommand(&enable_quick_record);
	AddConsoleCommand(&record);
	AddConsoleCommand(&debug_script);
	AddConsoleCommand(&render_debug);
	AddConsoleCommand(&pause);
	AddConsoleCommand(&quitcmd);
	AddConsoleCommand(&exitcmd);
	AddConsoleCommand(&editscript);
	AddConsoleCommand(&edit);
	AddConsoleCommand(&mouse_reset_speed);
}

//Records a new weapon script file
void RecordCommand()
{
	if (globals.AllowRecording >= 1.0f) {
		printf("Enter Name of Weapon (File):\n");
		std::cin >> globals.WeaponName;

		printf("Enter the game window name this script will target\n"
			"(Enter any for all processes):\n");
		std::cin >> globals.TargetWindowTitle;

		printf("Enter Type of Recording (0 for CycleTime, 1 for Continuous,\n"
			"2 for CycleTime AutoClicker, 3 for Continuous AutoClicker):\n");
		std::cin >> globals.ScriptType;
		globals.AutoClicker = globals.ScriptType == ScriptTypes::CycleTimeAutoClicker || globals.ScriptType == ScriptTypes::VariableTimeAutoClicker;
		globals.VariableTime = globals.ScriptType == ScriptTypes::VariableTime || globals.ScriptType == ScriptTypes::VariableTimeAutoClicker;

		if (globals.VariableTime)
		{
			globals.MaxBullets = 0;
			globals.CycleTime = 0;
			globals.TimerResolutionMS = 1; //Record/playback as fast as possible in variable mode
		}
		else
		{
			printf("Enter Max Bullets:\n");
			std::cin >> globals.MaxBullets;
			printf("Enter Cycle Time (Sample Capture Rate):\n");
			std::cin >> globals.CycleTime;
			globals.TimerResolutionMS = (int)(roundf(globals.CycleTime * 1000.0f)); //Record/playback only as fast as the weapon's CycleTime
		}

		if (globals.AutoClicker)
		{
			printf("Enter Mouse Click Down Time:\n");
			std::cin >> globals.ClickDownTime;
			printf("Enter Mouse Click Up Time:\n");
			std::cin >> globals.ClickUpTime;
		}

#ifdef USE_TIME_TO_IDLE
		printf("Enter Time To Idle (Return to Sample 0 Rate):\n");
		std::cin >> globals.TimeToIdle;
#else
		globals.TimeToIdle = 0.001f;
#endif

#ifdef USE_RELOAD_TIME
		printf("Enter Reload Time (Max Bullets Hit/Reload Wait):\n");
		std::cin >> globals.ReloadTime;
#else
		globals.ReloadTime = 0.001f;
#endif
		printf("Enter the current game sensitivity for recording (default 2):\n");
		std::cin >> globals.record_mouse_sensitivity;
		printf("Enter the screen resolution width used for recording:\n");
		std::cin >> globals.RecordedScreenRes.x;
		printf("Enter the screen resolution height used for recording:\n");
		std::cin >> globals.RecordedScreenRes.y;
		printf("Now press Shift when ready to record..\n");
		globals.PreStartRecord = 1;
		globals.nexttime_recordmouse = (float)GetCurTime();
	}
	else
	{
		printf("Your license does not permit you to record!\n");
	}
}

//Edits an existing script file
void EditScriptFileCommand()
{
	if (globals.AllowRecording >= 1.0f) {
		PRINT_CONDESC(&editscript);
		if (!GET_CONVAL_INT(&debug_script))
		{
			printf("Script debugging is not enabled. Would you like to enable it to see bullet\n"
				"numbers when firing? Enter 1 or 0:\n");
			int result;
			std::cin >> result;
			SET_CONVAL_INT(&debug_script, result);
		}
		printf("Enter Name of Weapon (File):\n");
		std::cin >> globals.WeaponName;
		if (ReadWeaponConfig())
		{
			printf("Enter the first bullet to edit:\n");
			int firstbullet, lastbullet, xchange, ychange;
			std::cin >> firstbullet;
			while (firstbullet < 1)
			{
				printf("First bullet is invalid! Try again:\n");
				std::cin >> firstbullet;
			}
			printf("Enter the last bullet to edit:\n");
			std::cin >> lastbullet;
			while (lastbullet > globals.NumMouseSamples)
			{
				printf("Last bullet is higher than the number of mouse samples (%i)!\n"
					"Try again:\n", globals.NumMouseSamples);
				std::cin >> lastbullet;
			}
			while (lastbullet < firstbullet)
			{
				printf("Last bullet is lower than the first bullet!\n"
					"Try again:\n");
				std::cin >> lastbullet;
			}
			printf("Enter the amount of x change\n"
				"(Positive values = right, negative = left):\n");
			std::cin >> xchange;
			printf("Enter the amount of y change\n"
				"(Positive values = down, negative = up):\n");
			std::cin >> ychange;
			printf("Do you wish to overwrite and save this script?\n"
				"Enter 1 to save, 0 to cancel:\n");
			int save;
			std::cin >> save;
			if (save)
			{
				for (int i = firstbullet; i <= lastbullet; i++)
				{
					globals.MouseSprayPattern[i].coords.x += xchange;
					globals.MouseSprayPattern[i].coords.y += ychange;
				}
				WriteWeaponConfig();
			}
		}
	}
	else
	{
		printf("Your license does not permit you to edit script files!\n");
	}
}

//Quits the entire program
void Quit()
{
	globals.quit = 1;
	exit(0);
}

//Toggles pausing the program, returns the value of pause after setting it
int TogglePause()
{
	ClearResetCursorVars();
	globals.nexttime_firebullet = globals.realtime + 1.0f;
	globals.done_resetting_cursor = 1;
	globals.bulletsfired = 0;
	globals.paused ^= 1;
	printf("paused is now %i\n", globals.paused);
	SetTitleBarText();
	return globals.paused;
}

#ifdef KEYGEN
void GenerateSerialCommand()
{
	if (cmd_argc() == 5)
	{
		GenerateSerial(atof(cmd_argv->Get(1)), atof(cmd_argv->Get(2)), atof(cmd_argv->Get(3)), atof(cmd_argv->Get(4)));
	}
	else
	{
		printf("Usage: generateserial allowrecording year month day\n");
	}
}
#endif

cvar_t* FindConsoleCommand(char* str)
{
	for (std::vector<cvar_t*>::iterator cvar = cvars.begin(); cvar != cvars.end(); cvar++) {
		cvar_t* var = *cvar;
		if (!_stricmp(str, var->name))
		{
			//cvar_t &cvarp = *cvar;
			//memcpy(&current_cvar, &*cvar, sizeof(cvar_t)); //Set the current cvar for future use
			return var;
		}
	}
	return nullptr;
}

//Calls a void function
void CALL_CON_COMMAND(void* function)
{
	((void(*)(void))function)();
}


//Received console command, check if its valid and process it. Verbose tells it to print the value after setting it
void ConsoleCommand(char *str, bool verbose)
{
	cmd_argv = SplitChar(str, " ");
	unsigned int count = cmd_argc();
	if (count == 0)
	{
		//Blank command was entered, just return
		delete cmd_argv;
		return;
	}

	//For each console command registered, check to see if the entered command exists
	cvar_t *cvar = FindConsoleCommand((char*)cmd_argv->Get(0));
	if (cvar)
	{
		if (count == 1)
		{
			//Entered only the command name
			if (cvar->flags & CVAR_FLAGS::FL_TOGGLE)
			{
				//Command is a toggleable variable
				PRINT_CONDESC(cvar);
				TOGGLE_CONVAL(cvar);
			}
			else if (cvar->type == CVAR_TYPES::CON_COMMAND)
			{
				//Call the function declared in the concommand
				CALL_CON_COMMAND(cvar->value);
			}
			else
			{
				//Not toggleable and not a concommand, just print a helpful description
				if (cvar->value)
				{
					PRINT_CONDESC(cvar);
					PRINT_CONVAL(cvar);
				}
				else
				{
					PRINT_CONDESC(cvar);
				}
			}
		}
		else
		{
			//Entered a command with arguments
			if (cvar->type == CVAR_TYPES::CON_COMMAND)
			{
				//Call the function declared in the concommand. Arguments will get handled by the function using cmd_argv
				CALL_CON_COMMAND(cvar->value);
			}
			else
			{
				//Set the variable's value
				SET_CONVAL(cvar, cmd_argv->Get(1), verbose);
			}
		}
	}
	else
	{
		//Command was not found, try loading a script file instead.
		TryLoadingScriptFile(str);
	}
	delete cmd_argv;
}

//Static vars used for ProcessConsoleInput()
#define CONSOLE_TEXT_MAXLENGTH 256
int console_textlen;
char console_text[CONSOLE_TEXT_MAXLENGTH];
#define MAX_REMEMBERED_CONSOLE_COMMANDS 32
char lastconsole_text[MAX_REMEMBERED_CONSOLE_COMMANDS][CONSOLE_TEXT_MAXLENGTH]; //last commands remembered
int lastconsole_text_length[MAX_REMEMBERED_CONSOLE_COMMANDS];
int lastconsole_text_num; //number of last commands stored
int index_lastconsole_text = -1;
float time_startedholding;
float time_repeatedkey;
bool first_key = 1;
int last_key;
HANDLE hinput;
HANDLE houtput;

//Remembers the currently entered console command
void RememberCommand(char* string)
{
	if (lastconsole_text_num + 1 > MAX_REMEMBERED_CONSOLE_COMMANDS)
	{
		for (int i = 1; i <= MAX_REMEMBERED_CONSOLE_COMMANDS; i++)
		{
			strcpy(lastconsole_text[i - 1], lastconsole_text[i]);
			lastconsole_text_length[i - 1] = lastconsole_text_length[i];
		}
		int index = lastconsole_text_num - 1;
		strcpy(lastconsole_text[index], string);
		lastconsole_text_length[index] = console_textlen;
	}
	else
	{
		lastconsole_text_num++;
		int index = lastconsole_text_num - 1;
		strcpy(lastconsole_text[index], string);
		lastconsole_text_length[index] = console_textlen;
	}
}

//Clears the currently typed text in the console window
void ClearCurrentlyTypedText()
{
	for (int i = 0; i <= console_textlen; i++)
	{
		std::cout << "\b" << " " << "\b"; //Press backspace and overwrite with space to clear currently typed text. There's probably a better way to do this...
	}
}

//User pressed up arrow, enter the last command based on index
void GetLastCommand()
{
	if (lastconsole_text_num > 0)
	{
		if (index_lastconsole_text == -1)
		{
			index_lastconsole_text = lastconsole_text_num;
		}
		if (index_lastconsole_text - 1 >= 0)
		{
			index_lastconsole_text--;
			console_text[0] = 0;
			ClearCurrentlyTypedText();
			std::cout << lastconsole_text[index_lastconsole_text];
			strcpy(console_text, lastconsole_text[index_lastconsole_text]);
			console_textlen = lastconsole_text_length[index_lastconsole_text];
		}
	}
}

//User pressed down arrow, enter the next command based on index
void GetNextCommand()
{
	if (lastconsole_text_num > 0)
	{
		if (index_lastconsole_text != -1)
		{
			int newindex = index_lastconsole_text + 1;
			if (newindex < MAX_REMEMBERED_CONSOLE_COMMANDS && newindex < lastconsole_text_num)
			{
				index_lastconsole_text = newindex;
				console_text[0] = 0;
				ClearCurrentlyTypedText();
				std::cout << lastconsole_text[index_lastconsole_text];
				strcpy(console_text, lastconsole_text[index_lastconsole_text]);
				console_textlen = lastconsole_text_length[index_lastconsole_text];
			}
		}
	}
}

void ProcessConsoleInput()
{
	INPUT_RECORD	recs[1024];
	unsigned long	dummy;
	int				ch;
	unsigned long	numread, numevents;
	while (!globals.quit)
	{
		if (!GetNumberOfConsoleInputEvents(hinput, &numevents))
			exit(-1);

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			exit(-1);

		if (numread != 1)
			exit(-1);

		if (recs[0].EventType == KEY_EVENT)
		{
			BOOL KeyDown = recs[0].Event.KeyEvent.bKeyDown;
			bool PressKey = false;
			if (KeyDown)
			{
				bool IsShift = recs[0].Event.KeyEvent.wVirtualKeyCode == VK_LSHIFT || recs[0].Event.KeyEvent.wVirtualKeyCode == VK_RSHIFT;
				if (recs[0].Event.KeyEvent.uChar.AsciiChar != last_key)
				{
					first_key = true;
					last_key = recs[0].Event.KeyEvent.uChar.AsciiChar;
				}
				if (first_key)
				{
					if (!IsShift)
					{
						last_key = recs[0].Event.KeyEvent.uChar.AsciiChar;
						first_key = false;
						PressKey = true;
					}
				}
				else if (!IsShift)
				{
					last_key = recs[0].Event.KeyEvent.uChar.AsciiChar;
					if (time_repeatedkey == 0)
					{
						time_repeatedkey = globals.realtime;
					}
					else if (globals.realtime - time_repeatedkey >= 0.05f)
					{
						time_repeatedkey = globals.realtime;
						PressKey = true;
					}
				}
			}
			else
			{
				first_key = true;
			}
			if (PressKey)
			{
				time_repeatedkey = 0.0f;
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;
				switch (ch) {
				case '\r':
					WriteFile(houtput, "\r\n", 2, &dummy, NULL);
					if (console_textlen)
					{
						console_text[console_textlen] = 0;
						RememberCommand(console_text);
						console_textlen = 0;
						ConsoleCommand(console_text, true);
						index_lastconsole_text = -1;
					}
					break;

				case '\b':
					if (console_textlen)
					{
						console_textlen--;
						WriteFile(houtput, "\b \b", 3, &dummy, NULL);
					}
					break;

				default:
					if (recs[0].Event.KeyEvent.wVirtualKeyCode == VK_UP)
					{
						GetLastCommand();
					}
					else if (recs[0].Event.KeyEvent.wVirtualKeyCode == VK_DOWN)
					{
						GetNextCommand();
					}
					else if (recs[0].Event.KeyEvent.wVirtualKeyCode == VK_LEFT)
					{
						//Todo: move caret of currently typed command left
					}
					else if (recs[0].Event.KeyEvent.wVirtualKeyCode == VK_RIGHT)
					{
						//Todo: move caret right
					}
					else if (ch >= ' ')
					{
						if (console_textlen < sizeof(console_text) - 2)
						{
							WriteFile(houtput, &ch, 1, &dummy, NULL);
							console_text[console_textlen] = (char)ch;
							console_textlen++;
						}
					}
					break;
				}
			}
		}
	}
}

int SetConsoleCXCY(HANDLE hStdout, short cx, short cy)
{
	COORD						CoordMax;
	CONSOLE_SCREEN_BUFFER_INFO	sbinfo;

	CoordMax = GetLargestConsoleWindowSize(hStdout);

	if (cy > CoordMax.Y)
		cy = CoordMax.Y;

	if (cx > CoordMax.X)
		cx = CoordMax.X;

	if (!GetConsoleScreenBufferInfo(hStdout, &sbinfo))
		return 0;

	// set height
	sbinfo.srWindow.Left = 0;
	sbinfo.srWindow.Right = sbinfo.dwSize.X - 1;
	sbinfo.srWindow.Top = 0;
	sbinfo.srWindow.Bottom = cy - 1;

	if (cy < sbinfo.dwSize.Y)
	{
		if (!SetConsoleWindowInfo(hStdout, TRUE, &sbinfo.srWindow))
			return 0;

		sbinfo.dwSize.Y = cy;

		if (!SetConsoleScreenBufferSize(hStdout, sbinfo.dwSize))
			return 0;
	}
	else if (cy > sbinfo.dwSize.Y)
	{
		sbinfo.dwSize.Y = cy;

		if (!SetConsoleScreenBufferSize(hStdout, sbinfo.dwSize))
			return 0;

		if (!SetConsoleWindowInfo(hStdout, TRUE, &sbinfo.srWindow))
			return 0;
	}

	if (!GetConsoleScreenBufferInfo(hStdout, &sbinfo))
		return 0;

	// set width
	sbinfo.srWindow.Left = 0;
	sbinfo.srWindow.Right = cx - 1;
	sbinfo.srWindow.Top = 0;
	sbinfo.srWindow.Bottom = sbinfo.dwSize.Y - 1;

	if (cx < sbinfo.dwSize.X)
	{
		if (!SetConsoleWindowInfo(hStdout, TRUE, &sbinfo.srWindow))
			return 0;

		sbinfo.dwSize.X = cx;

		if (!SetConsoleScreenBufferSize(hStdout, sbinfo.dwSize))
			return 0;
	}
	else if (cx > sbinfo.dwSize.X)
	{
		sbinfo.dwSize.X = cx;

		if (!SetConsoleScreenBufferSize(hStdout, sbinfo.dwSize))
			return 0;

		if (!SetConsoleWindowInfo(hStdout, TRUE, &sbinfo.srWindow))
			return 0;
	}

	return 1;
}