#pragma once
#include <Windows.h>

struct Profiler
{
	Profiler()
	{
		_totalMeasureTime = 0;
		_measureCnt = 0;
		_minTime = 0;
		_maxTime = 0;
		_overTimeCnt = 0;
		_overTime = 0;
	}

	void Renew(long long spendTime)
	{
		if (_maxTime == NULL && _minTime == NULL)
			_minTime = spendTime;

		else if (_minTime > spendTime)
			_minTime = spendTime;


		if (_maxTime < spendTime)
			_maxTime = spendTime;

		if (_overTime < spendTime)
			InterlockedIncrement64((volatile LONG64*)&_overTimeCnt);

		InterlockedIncrement64((volatile LONG64*)&_measureCnt);
		InterlockedAdd64((volatile LONG64*)&_totalMeasureTime, spendTime);

		if (_totalMeasureTime > 2000000000)
		{
			_totalMeasureTime /= 4;
			_measureCnt /= 4;
		}

	}


	// 시간 측정 관련 변수
	DWORD64	_totalMeasureTime;	// 총 측정 시간
	DWORD64 _measureCnt;		// 측정 횟수
	DWORD64	_minTime;			// 최소 측정 시간
	DWORD64	_maxTime;			// 최대 측정 시간
	DWORD64 _overTime;
	DWORD64 _overTimeCnt;		// 특별 카운트

	DWORD64	GetMinTime() { return _minTime; }
	DWORD64	GetMaxTime() { return _maxTime; }
	DWORD64	GetAvgTime()
	{
		if (_measureCnt == 0)
			return 0;
		else
			return (_totalMeasureTime / _measureCnt);
	}
};