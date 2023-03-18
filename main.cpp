#include "CParser.h"
#include "CLog.h"
#include "CCrashDump.h"
#include <conio.h>
#include "LoginServer.h"

inline bool GetKeyInput(LoginServer*);
inline void PrintData(LoginServer*);


int main()
{
	//------------------------------------------
	// 1. Parser
	//------------------------------------------

	ConfigGameServer* pConfig = new ConfigGameServer;
	CParser parser;
	parser.LoadFile(L"_Setting.txt");
	parser.FindCategory(L":GAME");


	parser.GetValue_String(L"BIND_IP", pConfig->game_IP);
	parser.GetValue_Int(L"PORT", &pConfig->game_port);
	parser.GetValue_Int(L"MAX_SESSION_SIZE", &pConfig->game_maxSessionSize);
	parser.GetValue_Int(L"RUNNING_THREAD_SIZE", &pConfig->game_runningWorkerThreadSize);
	parser.GetValue_Int(L"WORKER_THREAD_POOL_SIZE", &pConfig->game_workerThreadPoolSize);


	//------------------------------------------
	// 2. Data Init
	//------------------------------------------

	CCrashDump crashDump;
	CLog::Init();
	CLog::SetLogLevel(CLog::LEVEL_WARNING);
	CLog::ChangeMinLogLevel(CLog::LOWEST_LEVEL);



	//------------------------------------------
	// 3. Content Class Init
	//------------------------------------------

	LoginServer loginServer;
	loginServer.OpenGameServer(pConfig);



	//------------------------------------------
	// 4. Get Key Input
	//------------------------------------------

	for (;;)
	{
		PrintData(&loginServer);

		if (!GetKeyInput(&loginServer))
			break;


		Sleep(1000);
	}



	return 0;
}






inline void PrintData(LoginServer* content)
{

	if (content->_isServerOpened == false)
		return;


	content->PrintData();
}





inline bool GetKeyInput(LoginServer* content)
{
	if (_kbhit())
	{
		WCHAR inputKey = _getwch();


		// 서버 종료 키 (ESC)
		if (inputKey == 27)
		{
			content->CloseServer();
			return false;
		}

	}

	return true;
}
