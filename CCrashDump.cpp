#pragma comment(lib, "DbgHelp.Lib")

#include <Windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <Psapi.h>
#include <minidumpapiset.h>
#include <iostream>


#include "CCrashDump.h"




long CCrashDump::m_dumpCount = 0;

CCrashDump::CCrashDump()
{

	_invalid_parameter_handler newHandler;
	newHandler = myInvalidParameterHandler;



	_CrtSetReportMode(_CRT_WARN, 0);
	_CrtSetReportMode(_CRT_ASSERT, 0);
	_CrtSetReportMode(_CRT_ERROR, 0);


	_CrtSetReportHook(_custom_Report_hook);



	// Default pure virtual function called ���� �ڵ鷯�� ���� ������ �Լ��� ��ȸ��Ŵ
	_set_purecall_handler(myPurecallHandler);


	SetHandlerDump();
}





CCrashDump::~CCrashDump()
{
}






// ũ���� ����
void CCrashDump::Crash()
{
	int* p = nullptr;
	*p = 0;

}






LONG __stdcall CCrashDump::MyExceptionFilter(PEXCEPTION_POINTERS pExceptionPtr)
{
	int workingMemory = 0;
	SYSTEMTIME sysNowTime;


	long dumpCnt = InterlockedIncrement(&m_dumpCount);





	//-----------------------------------------------------------------
	// 1. ���� ���μ����� �޸� ��뷮 ������
	//-----------------------------------------------------------------

	PROCESS_MEMORY_COUNTERS pmc;
	HANDLE hProcess = NULL;



	hProcess = GetCurrentProcess();
	if (hProcess == NULL)
		return 0;


	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		workingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);


	CloseHandle(hProcess);







	//-----------------------------------------------------------------
	// 2. ���� ��¥�� �ð� �˾ƿͼ� ���ϸ� �ʱ�ȭ�ϱ�
	//-----------------------------------------------------------------

	WCHAR fileName[45];

	GetLocalTime(&sysNowTime);

	swprintf_s(fileName, L"Dump_%d%02d%02d.%02d.%02d.%02d_%d%dMB.dmp", sysNowTime.wYear, sysNowTime.wMonth, sysNowTime.wDay, sysNowTime.wHour, sysNowTime.wMinute, sysNowTime.wSecond, dumpCnt, workingMemory);

	printf("\n\n\n[ERROR] Crash Error\n");
	printf("[SYSTEM] Saving Dump File..\n");







	//-----------------------------------------------------------------
	// 3. ���� ���� �����
	//-----------------------------------------------------------------

	HANDLE hDumpFile = CreateFileW(fileName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);



	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION minidumpExceptionInfo;

		minidumpExceptionInfo.ThreadId = GetCurrentThreadId();
		minidumpExceptionInfo.ExceptionPointers = pExceptionPtr;
		minidumpExceptionInfo.ClientPointers = TRUE;


		MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory,
			&minidumpExceptionInfo,
			NULL,
			NULL);


		CloseHandle(hDumpFile);

		printf("[SYSTEM] Finshed Saving Dump Completely\n");
	}





	return EXCEPTION_EXECUTE_HANDLER;
}




void CCrashDump::SetHandlerDump()
{
	SetUnhandledExceptionFilter(MyExceptionFilter);
}
