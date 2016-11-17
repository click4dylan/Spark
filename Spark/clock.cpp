#include "stdafx.h"
#include "clock.h"
//#include <time.h>

ULONG origResolution = 0U; //Original timer resolution
ULONG minResolution = 0U;
ULONG maxResolution = 0U;
ULONG curResolution = 0U;

void StoreOriginalTimerResolution()
{
	NtQueryTimerResolution(&minResolution, &maxResolution, &curResolution);
	origResolution = curResolution;
}

void SetTimerResolution(double ms)
{
	NtSetTimerResolution((ULONG)(ms * 10000.0f), TRUE, &curResolution);
}

//Returns MS
double GetTimerResolution()
{
	NtQueryTimerResolution(&minResolution, &maxResolution, &curResolution);
	return (double)curResolution / 10000.0f;
}

//Returns MS
double GetMaxTimerResolution()
{
	if(maxResolution == 0U)
		NtQueryTimerResolution(&minResolution, &maxResolution, &curResolution);
	return (double)maxResolution / 10000.0f;
}

LARGE_INTEGER qpc_PerformanceFrequency;
LARGE_INTEGER qpc_MSPerformanceFrequency;
LARGE_INTEGER qpc_ClockStart;


void InitTime()
{
	if (!qpc_PerformanceFrequency.QuadPart)
	{
		QueryPerformanceFrequency(&qpc_PerformanceFrequency);
		qpc_MSPerformanceFrequency.QuadPart = qpc_PerformanceFrequency.QuadPart / 1000;
		QueryPerformanceCounter(&qpc_ClockStart);
	}
}