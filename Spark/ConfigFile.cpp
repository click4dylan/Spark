#include "stdafx.h"
#include "ConfigFile.h"
#include <fstream>
#include <iostream>
#include "CharSplit.h"
#include "ConsoleCommands.h""
#include "Spark.h"

const char *fileName = "Spark.cfg";

int ReadConfigFile()
{
	char *tempString = new char[1024];

	std::ifstream configFile(fileName);

	if (!configFile.is_open())
	{
		printf("WARNING: could not open %s\n", fileName);
		delete[] tempString;
		return 0;
	}

	configFile.getline(tempString, 1024);
	if (strstr(tempString, "Spark Config Version"))
	{
		StringList *list = SplitChar(tempString, " ");
		globals.ConfigVersion = (float)atof(list->Get(3));
		delete list;
		if (globals.ConfigVersion > CONFIG_FILE_VERSION)
		{
			printf("Error: Spark.cfg is newer (%f) than this Spark revision supports (%f)!\n", globals.ConfigVersion, CONFIG_FILE_VERSION);
			delete[] tempString;
			configFile.close();
			return 0;
		}
	}
	else
	{
		printf("ERROR: config file is corrupt!\n");
		delete[] tempString;
		configFile.close();
		return 0;
	}

	configFile.getline(tempString, 1024);
	while (strlen(tempString) != 0)
	{
		StringList *command = SplitChar(tempString, " ");
		int argl = command->GetSize();
		if (argl != 0)
		{
			//Ignore commented strings
			if (!strstr(command->Get(0), "//"))
			{
				ConsoleCommand(tempString, false);
				/*
				//FIXME: Use ConsoleCommand function instead?
				cvar_t *cvar = FindConsoleCommand((char*)command->Get(0));
				if (cvar)
				{
					if (cvar->type == CVAR_TYPES::CON_COMMAND)
					{
						CALL_CON_COMMAND(cvar->value);
					}
					else
					{
						SET_CONVAL(cvar, command->Get(1));
					}
				}
				else
				{
					printf("ReadConfigFile(): Couldn't find command: %s\n", command->Get(0));
				}
				*/
			}
		}
		else
		{
			//invalid
			DEBUG_PRINT("Found Invalid line in Spark.cfg");
		}
		delete command;
		tempString[0] = 0;
		configFile.getline(tempString, 1024);
	}
	configFile.close();
	delete[] tempString;
	return 1;
}

int WriteConfigFile(void)
{
	printf("Writing %s\n", fileName);
	remove(fileName);

	std::ofstream configFile(fileName);

	if (!configFile.is_open())
	{
		printf("ERROR: could not create %s\n", fileName);
		return 0;
	}

	configFile << "Spark Config Version " << CONFIG_FILE_VERSION << std::endl;

	for (std::vector<cvar_t*>::iterator cvar = cvars.begin(); cvar != cvars.end(); cvar++) {
		cvar_t* var = *cvar;
		if (var->type != CVAR_TYPES::CON_COMMAND)
		{
			configFile << "//" << var->description << std::endl;
			char convalue[128];
			GET_CONVAL(var, convalue);
			configFile << var->name << " " << convalue << std::endl;
		}
	}

	configFile.close();
	return 1;
}