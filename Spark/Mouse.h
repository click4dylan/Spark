#ifndef mouse_h
#define mouse_h
#define WINDOWS_MEAN_AND_LEAN
#include <Windows.h>
#include "Spark.h"
void CallOnStartLeftClickDown();
void CallOnStartLeftClickUp();
void FireBullet();
void SetNextFireBulletTime();
void InitializeResetCursorPos();
void ResetCursorPos(float absmaxspeed);
__forceinline void ClearBulletsShot() {
	//if (!globals.bulletsfired == 0)
	//{
	globals.bulletsfired = 0;
	//PrintBulletsFired();
	//}
}
void ClearResetCursorVars();
inline void PrintBulletsFired() { printf("Bullet: %i\n", globals.bulletsfired); }
void RecordMouse();
int RecordVariableMouseSample(POINT& mouse);
int RecordCycleTimeMouseSample(POINT& mouse);
void SaveMouseSample(POINT& mouse);
void FinishRecordingCycleTimeScript();
void FinishedRecordingVariableScript();
void QuickReRecordScript();
void StartRecording();
void CancelRecording();
inline bool bCoordsIdentical(POINT& pos1, POINT& pos2) { return (pos1.x == pos2.x && pos1.y == pos2.y); }
void InitializeRawInput();
#endif