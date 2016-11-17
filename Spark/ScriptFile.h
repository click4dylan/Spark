#ifndef scriptfile_h
#define scriptfile_h
#define SCRIPT_FILE_VERSION 4.0f

enum ScriptTypes {
	CycleTime = 0, //Cycle Time means spark moves the mouse at exactly the same defined time for each bullet, usually a multiple of the weapon's firing rate
	VariableTime, //Variable Time means each bullet has a separate wait time defined in the script as the next time to move the mouse. Usually used for replicating other macros
	CycleTimeAutoClicker, //Automatically clicks for the player, useful for pistols
	VariableTimeAutoClicker //Automatically clicks for the player, useful for pistols
};

void TryLoadingScriptFile(char* fileName);
int ReadWeaponConfig();
void WriteWeaponConfig();

#endif