#pragma once
#include <Windows.h>




class CCrashDump
{
public:
	CCrashDump();
	~CCrashDump();

	static void Crash();
	static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPtr);
	static void SetHandlerDump();


	static void myInvalidParameterHandler(const WCHAR* expression, const WCHAR* function, const WCHAR* file, UINT line, uintptr_t pReserved) { Crash(); }
	static int _custom_Report_hook(int repostType, char* message, int* returnValue) { Crash(); return true; }
	static void myPurecallHandler() { Crash(); }

public:

	static long m_dumpCount;
};



