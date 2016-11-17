#include "stdafx.h"
#include "ScriptFile.h"
#include "Spark.h"
#include <fstream>
#include "CharSplit.h"
//#include <iostream>

//Tries to load a weapon script file
void TryLoadingScriptFile(char* fileName)
{
	char *OriginalWeaponName = new char[64];
	if (strlen(globals.WeaponName) == 0)
		strcpy(OriginalWeaponName, "None");
	else
		strcpy(OriginalWeaponName, globals.WeaponName);
	strcpy(globals.WeaponName, fileName);
	if (!ReadWeaponConfig())
	{
		if (strcmp(OriginalWeaponName, "None"))
		{
			printf("Using Previous Weapon Script : %s\n", OriginalWeaponName);
		}
		strcpy(globals.WeaponName, OriginalWeaponName);
		if (strcmp(globals.WeaponName, "None"))
		{
			ReadWeaponConfig();
		}
	}
	delete OriginalWeaponName;
}

int ReadWeaponConfig()
{
	if (strlen(globals.WeaponName) == 0)
		return 0;

	char *fileName = new char[128];
	char *tempString = new char[1024];

	sprintf(fileName, "%s.txt", globals.WeaponName);

	std::ifstream scriptFile(fileName);

	if (!scriptFile.is_open())
	{
		printf("ERROR: could not open %s\n", fileName);
		delete[] fileName;
		delete[] tempString;
		return 0;
	}

	globals.NumMouseSamples = 0;

	scriptFile.getline(tempString, 1024);
	if (strstr(tempString, "Spark Script Version"))
	{
		StringList *list = SplitChar(tempString, " ");
		globals.ScriptVersion = (float)atof(list->Get(3));
		delete list;
		if (globals.ScriptVersion > SCRIPT_FILE_VERSION)
		{
			//Script file is newer than this spark revision
			printf("Error: Script File is newer (%f) than this Spark revision supports (%f)!\n", globals.ScriptVersion, SCRIPT_FILE_VERSION);
			delete[] tempString;
			return 0;
		}
		scriptFile.getline(tempString, 1024);
		strcpy(globals.WeaponName, tempString);

		if (globals.ScriptVersion >= 4.0f)
		{
			scriptFile.getline(tempString, 1024);
			strcpy(globals.TargetWindowTitle, tempString);
		}
		else
		{
			strcpy(globals.TargetWindowTitle, "any");
		}
	}
	else
	{
		//Backwards compatibility with 3.9.5.1 and earlier
		globals.ScriptVersion = 1.0f;
		strcpy(globals.WeaponName, tempString);
		strcpy(globals.TargetWindowTitle, "any");
	}

	scriptFile.getline(tempString, 1024);
	globals.ScriptType = atoi(tempString);

	globals.AutoClicker = globals.ScriptType == ScriptTypes::CycleTimeAutoClicker || globals.ScriptType == ScriptTypes::VariableTimeAutoClicker;
	globals.VariableTime = globals.ScriptType == ScriptTypes::VariableTime || globals.ScriptType == ScriptTypes::VariableTimeAutoClicker;

	scriptFile.getline(tempString, 1024);
	globals.MaxBullets = atoi(tempString);

	scriptFile.getline(tempString, 1024);
	globals.CycleTime = (float)atof(tempString);

	if (globals.ScriptVersion >= 3.0f)
	{
		scriptFile.getline(tempString, 1024);
		globals.ClickDownTime = (float)atof(tempString);

		scriptFile.getline(tempString, 1024);
		globals.ClickUpTime = (float)atof(tempString);
	}

	scriptFile.getline(tempString, 1024);
	globals.TimeToIdle = (float)atof(tempString);

	scriptFile.getline(tempString, 1024);
	globals.ReloadTime = (float)atof(tempString);

	scriptFile.getline(tempString, 1024);
	globals.record_mouse_sensitivity = (float)atof(tempString);

	scriptFile.getline(tempString, 1024);
	globals.RecordedScreenRes.x = atoi(tempString);

	scriptFile.getline(tempString, 1024);
	globals.RecordedScreenRes.y = atoi(tempString);

	globals.RecordedScreenRes_Sum = (float)(globals.RecordedScreenRes.x + globals.RecordedScreenRes.y);
	globals.RecordedScreenRes_Difference = (float)(globals.RecordedScreenRes.x - globals.RecordedScreenRes.y);
	globals.RecordedScreenRes_AspectRatio = (float)globals.RecordedScreenRes.x / (float)globals.RecordedScreenRes.y;

	for (int i = 1; i <= globals.MaxBullets; i++)
	{
		scriptFile.getline(tempString, 1024);
		StringList *list = SplitChar(tempString, ":");

		int size = list->GetSize();

		if (size < 3 || size > 4)
		{
			printf("ERROR: script file %s has malformed line on bullet %i!\n", fileName, i);
			delete list;
			delete[] fileName;
			delete[] tempString;
			return 0;
		}

		globals.MouseSprayPattern[i].bullet = atoi(list->Get(0));
		globals.MouseSprayPattern[i].coords.x = atoi(list->Get(1));
		globals.MouseSprayPattern[i].coords.y = atoi(list->Get(2));
		if (size == 4)
			globals.MouseSprayPattern[i].timeuntilnext = (float)atof(list->Get(3));
		else
			globals.MouseSprayPattern[i].timeuntilnext = 0.0f;

		globals.NumMouseSamples++;

		delete list;
	}

	printf("Mouse Samples: %i\n", globals.NumMouseSamples);
	scriptFile.close();
	delete[] fileName;
	delete[] tempString;

	if (globals.VariableTime)
	{
		//Record/playback as fast a possible in variable record mode
		globals.TimerResolutionMS = 1;
	}
	else
	{
		//Record/playback only as fast as the weapon's CycleTime
		globals.TimerResolutionMS = (int)(roundf((globals.CycleTime * 1000.0f)));
	}

	SetTitleBarText();
	return 1;
}

void WriteWeaponConfig()
{
	if (strlen(globals.WeaponName) == 0)
		return;

	char *fileName = new char[128];

	sprintf(fileName, "%s.txt", globals.WeaponName);

	printf("Writing %s\n", fileName);
	remove(fileName);

	std::ofstream scriptFile(fileName);

	if(!scriptFile.is_open())
	{
		printf("ERROR: could not create %s\n", fileName);
		delete[] fileName;
		return;
	}

	scriptFile << "Spark Script Version " << SCRIPT_FILE_VERSION << std::endl;
	scriptFile << globals.WeaponName << std::endl; //WeaponName
	scriptFile << globals.TargetWindowTitle << std::endl; //TargetWindowTitle
	scriptFile << globals.ScriptType << std::endl; //ScriptType
	scriptFile << globals.MaxBullets << std::endl; //MaxBullets
	scriptFile << globals.CycleTime << std::endl; //CycleTime
	scriptFile << globals.ClickDownTime << std::endl; //ClickDownTime
	scriptFile << globals.ClickUpTime << std::endl; //ClickUpTime
	scriptFile << globals.TimeToIdle << std::endl; //TimeToIdle
	scriptFile << globals.ReloadTime << std::endl; //ReloadTime
	scriptFile << globals.record_mouse_sensitivity << std::endl; //Recorded mouse sensitivity

	scriptFile << globals.RecordedScreenRes.x << std::endl; //Recorded screen resolution X

	scriptFile << globals.RecordedScreenRes.y << std::endl; //Recorded screen resolution Y

	for (int i = 1; i <= globals.MaxBullets; i++)
	{
		scriptFile << i << ":"; //Bullet Number
		scriptFile << globals.MouseSprayPattern[i].coords.x << ":"; //X Coordinate
		scriptFile << globals.MouseSprayPattern[i].coords.y << ":"; //Y Coordinate
		scriptFile << globals.MouseSprayPattern[i].timeuntilnext << std::endl; //Time until next bullet (Variable script mode only)
	}

	scriptFile.close();

	delete[] fileName;

	SetTitleBarText();
}
