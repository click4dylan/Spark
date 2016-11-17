#ifndef clock_h
#define clock_h

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#pragma comment(lib, "ntdll") //Needed for setting timer resolution

extern "C" {
	NTSYSAPI NTSTATUS NTAPI
		NtSetTimerResolution(
		IN  ULONG   desiredResolution,
		IN  BOOLEAN setResolution,
		OUT PULONG  currentResolution);

	NTSYSAPI NTSTATUS NTAPI
		NtQueryTimerResolution(
		OUT PULONG minimumResolution,
		OUT PULONG maximumResolution,
		OUT PULONG currentResolution);
}; /* extern "C" */

extern ULONG origResolution; //Original Timer Resolution
extern LARGE_INTEGER qpc_PerformanceFrequency;
extern LARGE_INTEGER qpc_MSPerformanceFrequency;
extern LARGE_INTEGER qpc_ClockStart;

void StoreOriginalTimerResolution();
void SetTimerResolution(double ms);
double GetTimerResolution();
double GetMaxTimerResolution();

void InitTime();
__forceinline double Plat_FloatTime()
{
	LARGE_INTEGER CurrentTime;

	QueryPerformanceCounter(&CurrentTime);

	double fRawSeconds = (double)(CurrentTime.QuadPart - qpc_ClockStart.QuadPart) / (double)(qpc_PerformanceFrequency.QuadPart);

	return fRawSeconds;
}
#define Sys_FloatTime Plat_FloatTime
#define GetTime Plat_FloatTime
__inline float MSEC_TO_SEC(float msec) { return msec / 1000.0f; }
#endif