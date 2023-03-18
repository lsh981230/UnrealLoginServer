#pragma once

#include <Windows.h>


struct QueryMessage
{
	__int64 accountNo;
	int loginStatus;
};





//---------------------------------------------------------------------------------------------------
// 
//			Monitoring
// 
//---------------------------------------------------------------------------------------------------



struct CHATSERVER_MONITORINGDATA
{
	DWORD memUsage;
	DWORD sessionSize;
	DWORD recvTPS;
	DWORD sendTPS;
	DWORD acceptTPS;
	DWORD logicThrdFPS;
	DWORD jobMsgQSize;
	DWORD authMsgQSize;
	DWORD leaveAccountNoMsgQSize;
	DWORD tokenDBResponseTime;
	DWORD accountDBResponseTime;
	DWORD authThreadFPS;
	DWORD sendQueryThreadFPS;
};


struct ConfigGameServer
{
	ConfigGameServer()
	{
		ZeroMemory(game_IP, sizeof(game_IP));
	}


	WCHAR	game_IP[16];
	int		game_port;
	int		game_maxSessionSize;
	int		game_runningWorkerThreadSize;
	int		game_workerThreadPoolSize;
	int		_authThreadSize;
	int		_postQueryThreadSize;
};