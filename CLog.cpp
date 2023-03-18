#include <Windows.h>
#include <iostream>
#include <strsafe.h>



#include "CLog.h"


int CLog::m_saveMinLogLevel;
__int64 CLog::m_logCount;
CRITICAL_SECTION CLog::m_csLogFileMap;



void CLog::ChangeMinLogLevel(int addLevel)
{
	m_saveMinLogLevel += addLevel; 

	if (m_saveMinLogLevel < 0)
		m_saveMinLogLevel = 0;
}



void CLog::Init()
{
	InitializeCriticalSection(&m_csLogFileMap);
}


int CLog::SetLogLevel(int minLogLevel)
{
	m_saveMinLogLevel = minLogLevel;
	return m_saveMinLogLevel;
}



void CLog::Log(const WCHAR * Type, int logLevel, const WCHAR * logStringFormat, ...)
{
	static WCHAR logLevelString[HIGHEST_LEVEL + 1][10] = { L"DEBUG", L"WARNING", L"ERROR", L"SYSTEM" };
	static WCHAR logGuideLine[65] = { L"\n\n---------------------------------------------------------\n\n" };


	if (logLevel < m_saveMinLogLevel)
		return;
	

	EnterCriticalSection(&m_csLogFileMap);


	//---------------------------------------------------------
	// 1. Log 내용 받아오기
	//---------------------------------------------------------

	WCHAR log[1024] = { 0, };


	va_list args;
	va_start(args, logStringFormat);
	StringCchVPrintfW(log, sizeof(log) / sizeof(WCHAR), logStringFormat, args);
	va_end(args);



	//---------------------------------------------------------
	// 2. 파일 명 초기화
	//---------------------------------------------------------

	WCHAR fileName[30];
	
	SYSTEMTIME sysNowTime;
	GetLocalTime(&sysNowTime);


	swprintf_s(fileName, L"%d%02d_%ws.txt", sysNowTime.wYear, sysNowTime.wMonth, Type);

	//---------------------------------------------------------
	// 3. 로그 정보 초기화
	//---------------------------------------------------------

	WCHAR logInfo[50] = { 0, };


	_InterlockedIncrement((volatile unsigned long long*)&m_logCount);

	swprintf_s(logInfo, L"[%ws] [%d/%d %d:%d:%d / %lld] ", logLevelString[logLevel], sysNowTime.wMonth, sysNowTime.wDay, sysNowTime.wHour, sysNowTime.wMinute, sysNowTime.wSecond, m_logCount);

	

	//---------------------------------------------------------
	// 3. 파일 오픈 
	//---------------------------------------------------------

	FILE* fp = nullptr;


	_wfopen_s(&fp, fileName, L"a+");

	if (fp == nullptr)
	{
		Log((WCHAR*)L"Server", CLog::LEVEL_ERROR, (WCHAR*)L"Logging Failed");
		return;
	}



	//---------------------------------------------------------
	// 4. 파일 포인터 마지막으로 보내기
	//---------------------------------------------------------

	fseek(fp, 0, SEEK_END);



	//---------------------------------------------------------
	// 5. 로그 내용 입력
	//---------------------------------------------------------

	/*fwrite(logInfo, lstrlenW(logInfo) * sizeof(WCHAR), 1, fp);
	fwrite(log, lstrlenW(log) * sizeof(WCHAR), 1, fp);
	fwrite(logGuideLine, lstrlenW(logGuideLine) * sizeof(WCHAR), 1, fp);*/
	
	fwprintf(fp, logInfo);
	fwprintf(fp, log);
	fwprintf(fp, logGuideLine);


	//---------------------------------------------------------
	// 6. 파일 닫기
	//---------------------------------------------------------

	fclose(fp);

	LeaveCriticalSection(&m_csLogFileMap);


	//---------------------------------------------------------
	// 7. 콘솔에 출력
	//---------------------------------------------------------

	printf("%ws %ws %ws", logInfo, log, logGuideLine);


}
