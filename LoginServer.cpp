
#include <process.h>
#include "TLS_RedisConnector.h"
#include "Tls_DBConnector.h"

#include "CSerializeBuffer.h"
#include "CommonProtocol.h"
#include "CLog.h"

#include "LoginServer.h"
#include "MyPDH.h"


LoginServer::LoginServer()
{
	_pCpuUsage = new CPUUsage(GetCurrentProcess());


	_pdh = new PDH();

	if (!_pdh->AddCounter((WCHAR*)L"\\Process(Portfolio)\\Private Bytes", PRIVATE_MEM))
	{
		CLog::Log(L"Game", CLog::LEVEL_ERROR, L"PDH AddCounter Failed, Private Bytes");
	}

	if (!_pdh->AddCounter((WCHAR*)L"\\Memory\\Pool Nonpaged Bytes", NON_PAGED_POOL))
	{
		CLog::Log(L"Game", CLog::LEVEL_ERROR, L"PDH AddCounter Failed, Pool Nonpaged Bytes");
	}

	if (!_pdh->AddCounter((WCHAR*)L"\\Network Adapter(Realtek PCIe GBE Family Controller)\\Bytes Sent/sec", NET_SENT))
	{
		CLog::Log(L"Game", CLog::LEVEL_ERROR, L"PDH AddCounter Failed, Network Adapter");
	}

	_beginLevel = 1;
	_beginMaxHP = 100;
	_beginAttackDamage = 10;
}



LoginServer::~LoginServer()
{
	delete _pCpuUsage;

	delete _pAccountDBConnector;
	delete _pGameDBConnector;
}





void LoginServer::OnRecv(DWORD64 sessionID, CSerializeBuffer* pMessage)
{
	if (pMessage == nullptr)
	{
		CCrashDump::Crash();
	}


	SwitchMessage(pMessage, sessionID);
	CSerializeBuffer::Free(pMessage);
}



void LoginServer::OnEnterServer(DWORD64 sessionID)
{

}



void LoginServer::OnLeaveServer(DWORD64 sessionID)
{

}




void LoginServer::PrintData()
{

	// Print
	printf("\n\n\n===================================================================\n\n");

	printf("ESC	: Server Close\n");
	printf(" 1	: Server Start\n");
	printf(" 2	: Server Stop\n");


	printf("===============================================\n");
	printf("Login Server\n");
	printf("===============================================\n");
	printf("Session Size : %lld\n", _maxSessionSize - GetSessionIndexStackCount());
	printf("Packet Pool Usage Count : %d\n", CSerializeBuffer::GetUsingCnt());
	printf("Packet Pool Alloc Count : %d\n", CSerializeBuffer::GetAllocCnt());
	printf("Accept Total : %lld\n", _sessionCnt);
	printf("Accept TPS : %d\n", _acceptTPS);
	printf("Recv TPS : %d\n", _recvPacketTPS);
	printf("Send TPS : %d\n\n", _sendPacketTPS);

	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;


	printf("------------------------------------------------------------------------------------\n");
	//printf("CPU Usage : %.1f%% (User : %.1f%% / Kernel : %.1f%%)\n", _pCpuUsage->GetProcessTotal(), _pCpuUsage->GetProcessUser(), _pCpuUsage->GetProcessKernel/());
	//printf("Non-Paged Pool Usage : %.1lfK\n", (double)(_pdh->GetCounterValue(NON_PAGED_POOL) / 1024.f));
	printf("Process Private Memory Usage : %.1lfMB\n", (double)(_pdh->GetCounterValue(PRIVATE_MEM) / 1048576.f));

	double netSent = _pdh->GetCounterValue(NET_SENT) / 1024.f;
	if (netSent > 1000)
		printf("Network Sent : %.1lfMbps\n", netSent / 1024.f);
	else
		printf("Network Sent : %.1lfKbps\n", netSent);
	printf("------------------------------------------------------------------------------------\n");

}




void LoginServer::OpenGameServer(ConfigGameServer* pConfig)
{

	//------------------------------------------------------------------
	// 1. Create Thread
	//------------------------------------------------------------------

	_isServerOpened = true;


	//------------------------------------------------------------------
	// 2. Connect to DB
	//------------------------------------------------------------------

	string accountSchemaName = "account";
	_pAccountDBConnector = new Tls_DBConnector(accountSchemaName, TlsAlloc());

	string gameSchemaName = "game";
	_pGameDBConnector = new Tls_DBConnector(gameSchemaName, TlsAlloc());

	


	//------------------------------------------------------------------
	// 3. Net Server Open
	//------------------------------------------------------------------

	if (!OpenServer(pConfig->game_IP, pConfig->game_maxSessionSize, pConfig->game_port, pConfig->game_runningWorkerThreadSize, pConfig->game_workerThreadPoolSize))
	{
		CLog::Log(L"Login", CLog::LEVEL_ERROR, L"Open Failed");
		CCrashDump::Crash();
	}

}




inline void LoginServer::SwitchMessage(CSerializeBuffer* pMsg, DWORD64 sessionID)
{

	WORD type;
	*pMsg >> type;


	switch (type)
	{
	case en_PACKET_CS_LOGIN_REQ_LOGIN:
		Request_Login(pMsg, sessionID);
		break;
	case en_PACKET_CS_LOGIN_REQ_REGIST:
		Request_Regist(pMsg, sessionID);
		break;

	default:
	{
		CLog::Log(L"LoginServer", CLog::LEVEL_ERROR, L"[GameServer::SwitchMessage()] Msg Type is Unknown type, Type was %d", type);
		CCrashDump::Crash();
	}
	}

}







inline void LoginServer::Request_Login(CSerializeBuffer* pMsg, DWORD64 sessionID)
{
	// 1. 메시지 분해
	char id[40] = { 0, };
	char pw[65] = { 0, };

	*pMsg >> id;
	pMsg->GetData((char*)pw, 64);


	// 2. 로그인 메시지 생성
	CSerializeBuffer* pLoginMsg = CSerializeBuffer::Alloc();
	*pLoginMsg << (WORD)en_PACKET_SC_LOGIN_RES_LOGIN;


	// 3. account Schema에서 Row 검색
	int resCnt = 0;
	__int64 accountNo = 0;
	sql::Statement* pStatement = nullptr;
	sql::ResultSet* res = nullptr;
	bool bSucceed = true;

	res = _pAccountDBConnector->SendQuery(&bSucceed, pStatement, "SELECT account_no FROM account_information WHERE account_id = '%s' AND account_pw = '%s'", id, pw);


	while (res->next())
	{
		resCnt++;

		accountNo = res->getInt64("account_no");
	}

	if (!bSucceed || accountNo == NULL)
	{
		delete res;

		*pLoginMsg << (char)0;

		pLoginMsg->PutHeaderData();

		EnqueueMsg2SendQ(sessionID, pLoginMsg);
		return;
	}

	delete res;


	// 6. 로그인 완료 메시지 전송

	*pLoginMsg << (char)1;
	*pLoginMsg << accountNo;

	pLoginMsg->PutHeaderData();

	EnqueueMsg2SendQ(sessionID, pLoginMsg);
}



void LoginServer::Request_Regist(CSerializeBuffer* pMsg, DWORD64 sessionID)
{
	// 1. 메시지 분해
	char id[40] = { 0, };
	char pw[65] = { 0, };

	*pMsg >> id;
	pMsg->GetData((char*)pw, 64);



	// 2. 계정 생성 메시지 생성
	CSerializeBuffer* pCreateAccountMsg = CSerializeBuffer::Alloc();
	*pCreateAccountMsg << (WORD)en_PACKET_CS_LOGIN_RES_REGIST;



	// 3. Account DB에 Row 추가
	sql::Statement* pStatement = nullptr;
	sql::ResultSet* res = nullptr;
	bool bSucceed = true;

	res = _pAccountDBConnector->SendQuery(&bSucceed, pStatement, "Call Regist('%s', '%s');", id, pw);

	delete res;

	if (!bSucceed)
	{
		*pCreateAccountMsg << (char)0;
	}
	else
	{
		*pCreateAccountMsg << (char)1;
	}


	pCreateAccountMsg->PutHeaderData();
	EnqueueMsg2SendQ(sessionID, pCreateAccountMsg);
}







