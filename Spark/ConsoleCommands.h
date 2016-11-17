#ifndef consolecommands_h
#define consolecommands_h
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include "CharSplit.h"

#define CVAR_STRING_LENGTH 128 //todo, might be more elegant to have this a dynamic size, but we don't have any string commands right now, so save it for later

void PrintStartupText(); //Move this somewhere else?

enum CVAR_FLAGS
{
	FL_NONE = 0,
	FL_TOGGLE = 1
};

enum CVAR_TYPES
{
	CVAR_BOOL = 0,
	CVAR_INT,
	CVAR_FLOAT,
	CVAR_DOUBLE,
	CVAR_STRING,
	CON_COMMAND //void*, Calls a function rather than being a variable
};

struct cvar_t
{
	const char* name;
	const char* description;
	CVAR_TYPES type;
	int flags;
	void* value;
};

//Allocates memory for convar value
inline void* ALLOC_CONVAL(bool value)
{
	void* memory = malloc(sizeof(bool));
	*(bool*)memory = value;
	return memory;
}

//Allocates memory for convar value
inline void* ALLOC_CONVAL(int value)
{
	void* memory = malloc(sizeof(int));
	*(int*)memory = value;
	return memory;
}

//Allocates memory for convar value
inline void* ALLOC_CONVAL(float value)
{
	void* memory = malloc(sizeof(float));
	*(float*)memory = value;
	return memory;
}

//Allocates memory for convar value
inline void* ALLOC_CONVAL(double value)
{
	void* memory = malloc(sizeof(double));
	*(double*)memory = value;
	return memory;
}

//Allocates memory for convar value
inline void* ALLOC_CONVAL(const char* value)
{
	//Might be more elegant to have this a dynamic size, but we don't have any string commands right now, so save it for later
	void* memory = malloc(CVAR_STRING_LENGTH);
	strcpy((char*)memory, value);
	return memory;
}

void PRINT_CONDESC(cvar_t* cvar);
void PRINT_CONVAL(cvar_t* cvar);
void SET_CONVAL(cvar_t* cvar, const char* value, bool verbose);
void TOGGLE_CONVAL(cvar_t* cvar);


__forceinline void GET_CONVAL(cvar_t* cvar, char* destination)
{
	switch (cvar->type)
	{
		case CVAR_TYPES::CVAR_BOOL:
		{
			sprintf(destination, "%d", *(bool*)cvar->value);
			break;
		}
		case CVAR_TYPES::CVAR_INT:
		{
			sprintf(destination, "%d", *(int*)cvar->value);
			break;
		}
		case CVAR_TYPES::CVAR_FLOAT:
		{
			sprintf(destination, "%f", *(float*)cvar->value);
			break;
		}
		case CVAR_TYPES::CVAR_DOUBLE:
		{
			sprintf(destination, "%lf", *(double*)cvar->value);
			break;
		}
		case CVAR_TYPES::CVAR_STRING:
		{
			strcpy(destination, (char*)cvar->value);
			break;
		}
	}
}

__forceinline bool GET_CONVAL_BOOL(cvar_t* cvar)
{
	return *(bool*)cvar->value;
}

__forceinline int GET_CONVAL_INT(cvar_t* cvar)
{
	return *(int*)cvar->value;
}

__forceinline float GET_CONVAL_FLOAT(cvar_t* cvar)
{
	return *(float*)cvar->value;
}

__forceinline double GET_CONVAL_DOUBLE(cvar_t* cvar)
{
	return *(double*)cvar->value;
}

__forceinline const char* GET_CONVAL_STRING(cvar_t* cvar)
{
	return (char*)cvar->value; //todo: test
}




__forceinline void SET_CONVAL_BOOL(cvar_t* cvar, bool value)
{
	*(bool*)cvar->value = value;
}

__forceinline void SET_CONVAL_INT(cvar_t* cvar, int value)
{
	*(int*)cvar->value = value;
}

__forceinline void SET_CONVAL_FLOAT(cvar_t* cvar, float value)
{
	*(float*)cvar->value = value;
}

__forceinline void SET_CONVAL_DOUBLE(cvar_t* cvar, double value)
{
	*(double*)cvar->value = value;
}

__forceinline void SET_CONVAL_STRING(cvar_t* cvar, const char* str)
{
	strcpy((char*)cvar->value, str); //todo: test
}

void AddConsoleCommand(cvar_t* cvar);
void RegisterConsoleCommands();
cvar_t* FindConsoleCommand(char* str);
void CALL_CON_COMMAND(void* function);
void ConsoleCommand(char *str, bool verbose);
void RecordCommand();
void EditScriptFileCommand();
void Quit(); //Quits the entire program
int TogglePause();
#ifdef KEYGEN
void GenerateSerialCommand();
#endif

extern std::vector<cvar_t*> cvars; //Global console command list
extern StringList *cmd_argv;
unsigned int cmd_argc(void);

#ifdef KEYGEN
extern cvar_t generateserial;
#endif
extern cvar_t mouse_sensitivity;
extern cvar_t enable_bullet_crosshair;
extern cvar_t enable_center_crosshair;
extern cvar_t enable_quick_record;
extern cvar_t record;
extern cvar_t debug_script;
extern cvar_t render_debug;
extern cvar_t pause;
extern cvar_t quitcmd;
extern cvar_t exitcmd;
extern cvar_t mouse_reset_speed;

//Static vars used for ProcessConsoleInput()
extern int console_textlen;
extern char console_text[256];
extern HANDLE hinput;
extern HANDLE houtput;
void ClearCurrentlyTypedText();
void ProcessConsoleInput();

int SetConsoleCXCY(HANDLE hStdout, short cx, short cy);
#endif