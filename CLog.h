#pragma once
#include <Windows.h>


class CLog
{
public:

	enum 
	{ 
		LEVEL_DEBUG = 0, 
		LEVEL_WARNING, 
		LEVEL_ERROR,
		LEVEL_SYSTEM,

		LOWEST_LEVEL = LEVEL_DEBUG, 
		HIGHEST_LEVEL = LEVEL_SYSTEM
	};

	static void Init();
	static int  SetLogLevel(int minLogLevel);
	static void Log(const WCHAR* Type, int logLevel, const WCHAR* logStringFormat, ...);
	static void ChangeMinLogLevel(int addLevel);


private:
	CLog() {}
	~CLog() {}


private:
	static int m_saveMinLogLevel;		// 저장할 최소 로그 레벨
	static __int64 m_logCount;
	static CRITICAL_SECTION m_csLogFileMap;
};



// 사용법
// CLog::->Log((WCHAR*)L"TYPE", CLog::LEVEL_DEBUG, (WCHAR*)L"FORMAT", L"CONTENTS");
