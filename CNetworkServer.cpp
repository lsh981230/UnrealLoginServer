
#include <atlsocket.h>
#include <Windows.h>
#include <process.h>
#include <timeapi.h>

#include "CLog.h"
#include "CNetworkServer.h"

#pragma comment(lib, "Iphlpapi.lib")
#include "iphlpapi.h"
#include "Profiler.h"



CNetworkServer::CNetworkServer()
{
	_isServerOpened = false;
}



CNetworkServer::~CNetworkServer()
{
	CloseServer();
}



void CNetworkServer::InitSession(SESSION* pSession)
{
	pSession->pRecvQ->ClearBuffer();
	DeleteEveryMsgInSendQ(pSession);

	pSession->recvOverlap.mode = MY_OVERLAPPED::MODE_RECV;
	pSession->sendOverlap.mode = MY_OVERLAPPED::MODE_SEND;
	pSession->reqSendOverlap.mode = MY_OVERLAPPED::MODE_REQSEND;

	ZeroMemory(&pSession->recvOverlap.overlap, sizeof(OVERLAPPED));
	ZeroMemory(&pSession->sendOverlap.overlap, sizeof(OVERLAPPED));
	ZeroMemory(&pSession->reqSendOverlap.overlap, sizeof(OVERLAPPED));
	ZeroMemory(pSession->ip, sizeof(pSession->ip));

	pSession->disconnectAfterSendFlag = false;
	pSession->sendMsgCnt = 0;
	pSession->releaseFlag = 1;
	pSession->isSocketClosed = 0;
	pSession->isSending = 0;

	pSession->socket = INVALID_SOCKET;
}



void CNetworkServer::DisconnectSession(DWORD64 sessionID)
{
	SESSION* pSession = GetSession(sessionID);

	if (InterlockedIncrement((DWORD*)&pSession->usingCnt) == 1 || pSession->sessionID != sessionID)
	{
		DecrementUsingCnt(pSession);
		return;
	}


	SocketClose(pSession);
	DecrementUsingCnt(pSession);
}





// Content -> Network
void CNetworkServer::EnqueueMsg2SendQ(DWORD64 sessionID, CSerializeBuffer* pMessage)
{
	//-------------------------------------------------------------------
	// 1. Session ã��
	//-------------------------------------------------------------------

	SESSION* pSession = GetSession(sessionID);

	if (InterlockedIncrement((DWORD*)&pSession->usingCnt) == 1 || pSession->sessionID != sessionID)
	{
		DecrementUsingCnt(pSession);
		CSerializeBuffer::Free(pMessage);
		return;
	}


	//-------------------------------------------------------------------
	// 2. �޼��� ��ȣȭ
	//-------------------------------------------------------------------

	pMessage->Encode();


	//-------------------------------------------------------------------
	// 3. �޼����� �ּҸ� SendQ�� ����
	//-------------------------------------------------------------------

	pSession->sendQ.Enqueue(pMessage);


	//-------------------------------------------------------------------
	// 4. Request SendPost
	//-------------------------------------------------------------------

	RequestSendPost(pSession);


	//-------------------------------------------------------------------
	// 5. Decrement IO Count
	//-------------------------------------------------------------------

	DecrementUsingCnt(pSession);

}






void CNetworkServer::SendPost(SESSION* pSession)
{
	//-------------------------------------------------------
	// 1. ���� ó�� 
	//-------------------------------------------------------

	if (pSession->sendQ.GetQueueSize() <= 0)
		return;




	if (InterlockedExchange(&pSession->isSending, 1) == 0)
	{
		WSABUF wsabuf[200];
		DWORD sendByte, retval, bufferCnt;


		//-------------------------------------------------------
		// 2. ���� �޼��� WSABUF�� �ʱ�ȭ
		//-------------------------------------------------------

		int sendQSize = pSession->sendQ.GetQueueSize();
		if (sendQSize == 0)
		{
			InterlockedExchange(&pSession->isSending, 0);
			return;
		}


		for (bufferCnt = 0; bufferCnt < sendQSize; ++bufferCnt)
		{
			CSerializeBuffer* pMsg = nullptr;
			if (pSession->sendQ.Peek(&pMsg, bufferCnt) == false)
				CCrashDump::Crash();

			if (pMsg->disconnectFlag)
				pSession->disconnectAfterSendFlag = true;

			wsabuf[bufferCnt].len = pMsg->GetPayloadLen(true) + dfNETMESSAGE_HEADER_SIZE;
			wsabuf[bufferCnt].buf = pMsg->GetHeaderPtr();
		}



		pSession->sendMsgCnt = bufferCnt;
		ZeroMemory(&pSession->sendOverlap.overlap, sizeof(OVERLAPPED));
		pSession->sendOverlap.mode = pSession->sendOverlap.MODE_SEND;




		//-------------------------------------------------------
		// 3. I/O Count ����
		//-------------------------------------------------------

		InterlockedIncrement((DWORD*)&pSession->usingCnt);



		//-------------------------------------------------------
		// 4. WSASend()
		//-------------------------------------------------------

		retval = WSASend(pSession->socket, wsabuf, bufferCnt, &sendByte, 0, (LPOVERLAPPED)&pSession->sendOverlap, NULL);



		//-------------------------------------------------------
		// 5. ���ϰ� Ȯ��
		//-------------------------------------------------------

		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				InterlockedExchange(&pSession->isSending, 0);
				DecrementUsingCnt(pSession);
			}

		}

	}

}


void CNetworkServer::RequestSendPost(SESSION* pSession)
{
	PostQueuedCompletionStatus(_hWorkerCP, NULL, (ULONG_PTR)pSession, (LPOVERLAPPED)&pSession->reqSendOverlap);
}






bool CNetworkServer::StartNetworking()
{

	//---------------------------------------------------
	// 1. Create I/O Completion Port
	//--------------------------------------------------- 

	_hWorkerCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _runningThreadSize);
	if (_hWorkerCP == NULL)
		return false;




	//--------------------------------------------------- 
	// 2. WinSock Init
	//--------------------------------------------------- 

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;




	//--------------------------------------------------- 
	// 3. socket
	//--------------------------------------------------- 

	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		CLog::Log((WCHAR*)L"Server", CLog::LEVEL_ERROR, (WCHAR*)L"socket() has failed, %d", GetLastError());
		return false;
	}



	//--------------------------------------------------- 
	// 4. bind
	//--------------------------------------------------- 

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	InetPtonW(AF_INET, _listenIP, &serverAddr.sin_addr.S_un.S_addr);
	serverAddr.sin_port = htons(_port);

	if (bind(_listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		CLog::Log((WCHAR*)L"Server", CLog::LEVEL_ERROR, (WCHAR*)L"bind() has failed, %d", GetLastError());
		return false;
	}



	//--------------------------------------------------- 
	// 5. listen
	//--------------------------------------------------- 

	if (listen(_listenSock, SOMAXCONN_HINT(10000)) == SOCKET_ERROR)
	{
		CLog::Log((WCHAR*)L"Server", CLog::LEVEL_ERROR, (WCHAR*)L"listen() has failed, %d", GetLastError());
		return false;
	}

	CLog::Log((WCHAR*)L"Server", CLog::LEVEL_SYSTEM, (WCHAR*)L"Server Open.  Date : %S, Time : %S, Server Port : %d", (WCHAR*)__DATE__, (WCHAR*)__TIME__, _port);



	//--------------------------------------------------- 
	// 6. Create Thread
	//--------------------------------------------------- 

	_hWorkThreadAry = new HANDLE[_workerThreadPoolSize];

	for (int i = 0; i < _workerThreadPoolSize; i++)
	{
		_hWorkThreadAry[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, nullptr);
	}


	HANDLE hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, this, 0, nullptr);
	CloseHandle(hAcceptThread);



	return true;
}





unsigned int __stdcall CNetworkServer::WorkerThread(LPVOID arg)
{
	CNetworkServer* pNetwork = (CNetworkServer*)arg;


	for (;;)
	{

		//------------------------------------------------------------
		// 1. Init Data
		//------------------------------------------------------------

		DWORD dwTransferred = 0;
		SESSION* pSession = nullptr;
		MY_OVERLAPPED* pMyOverlap = nullptr;




		//------------------------------------------------------------
		// 2. GQCS()
		//------------------------------------------------------------

		GetQueuedCompletionStatus(pNetwork->_hWorkerCP, &dwTransferred, (PULONG_PTR)&pSession, (LPOVERLAPPED*)&pMyOverlap, INFINITE);





		//------------------------------------------------------------
		// 3. ���� ó��
		//------------------------------------------------------------

		// ���� ����
		if (pMyOverlap == NULL && pSession == NULL && dwTransferred == NULL)
		{
			CLog::Log((WCHAR*)L"Server", CLog::LEVEL_SYSTEM, (WCHAR*)L"Thread Close.  Work Thread ID : %d", GetCurrentThreadId());
			return 1;
		}



		// IOCP Error
		if (pMyOverlap == NULL)
		{
			CLog::Log((WCHAR*)L"Server", CLog::LEVEL_ERROR, (WCHAR*)L"[CNetworkServer::WorkerThread()], IOCP I/O error : %d", WSAGetLastError());
			break;
		}




		//------------------------------------------------------------
		// 4. SendPost Request
		//------------------------------------------------------------

		if (pMyOverlap->mode == pMyOverlap->MODE_REQSEND)
		{
			InterlockedIncrement(&pSession->usingCnt);
			pNetwork->SendPost(pSession);
		}




		//------------------------------------------------------------
		// 5. Send Complete
		//------------------------------------------------------------

		else if (pMyOverlap->mode == pMyOverlap->MODE_SEND)
		{
			pNetwork->DeleteSendMsg(pSession);

			if (!pSession->disconnectAfterSendFlag)
			{
				InterlockedExchange(&pSession->isSending, 0);
				pNetwork->SendPost(pSession);
			}
			else
			{
				// Send�� Ǯ�� �ʰ�, closesocket�� ������ ���̻��� send ������ ����
				if (InterlockedExchange(&pSession->isSocketClosed, 1) == 0)		
				{
					closesocket(pSession->socket);
					pSession->socket = INVALID_SOCKET;
				}
			}
		}





		//------------------------------------------------------------
		// 6. Recv Complete
		//------------------------------------------------------------

		else if (pMyOverlap->mode == pMyOverlap->MODE_RECV)
		{
			if (dwTransferred != 0)
				pNetwork->RecvComplete(pSession, dwTransferred);
		}





		//------------------------------------------------------------
		// 7. I/O Using Count ����
		//------------------------------------------------------------

		pNetwork->DecrementUsingCnt(pSession);
	}


	return 0;
}




unsigned int __stdcall CNetworkServer::AcceptThread(LPVOID arg)
{
	SOCKET clientSock;
	SOCKADDR_IN addr;
	int addrLen = sizeof(addr);
	int retval;

	CNetworkServer* pNetwork = (CNetworkServer*)arg;


	for (;;)
	{

		//-----------------------------------------------------------------
		// 1. Accept
		//-----------------------------------------------------------------

		clientSock = accept(pNetwork->_listenSock, (SOCKADDR*)&addr, &addrLen);
		if (clientSock == INVALID_SOCKET)
		{
			if (WSAGetLastError() == 10004)
				CLog::Log((WCHAR*)L"Server", CLog::LEVEL_SYSTEM, (WCHAR*)L"Thread Close.  Accept Thread ID : %d", GetCurrentThreadId());

			else
				CLog::Log((WCHAR*)L"Server", CLog::LEVEL_ERROR, (WCHAR*)L"Accept Failed, %d", WSAGetLastError());

			break;
		}




		//-----------------------------------------------------------------
		// 2. ������ Ŭ���̾�Ʈ ó��
		//-----------------------------------------------------------------

		SESSION* pSession = pNetwork->AllocSession(clientSock);
		while (pSession == nullptr)
		{
			WaitForSingleObject(pNetwork->_hSessionFree, INFINITE);
			pSession = pNetwork->AllocSession(clientSock);
		}

		inet_ntop(AF_INET, &addr.sin_addr, pSession->ip, sizeof(pSession->ip));
		++pNetwork->_acceptTPS;

		/* �̸� UsingCnt�� �ø� ���� : WSARecv()�ɱ� ���� OnEnterServer() ȣ���� ���� Player ��ü ������ �س��ƾ� �ش� ������ �޼���(��Ŷ)�� ������ ��
		   Player �˻� �� Player�� nullptr�� ��츦 ���� �� �ֱ⿡ OnEnterServer()�� WSARecv()�� �����ؼ� ȣ���ؾ��ϰ�, 
		   ��������Ʈ���� �ش� ���ǿ� �޼����� ���� ��� WSASend()�ϸ鼭 IOCount�� �÷ȴٰ� ������ �������� 1�� �ö󰡰� �ٽ� 0���� �������� ������ �� �� �����Ƿ� �̸� IOCount�� �÷���*/
		InterlockedIncrement((DWORD*)&pSession->usingCnt);		
		pSession->releaseFlag = 0;			// �������� Release ����
															

		pNetwork->OnEnterServer(pSession->sessionID);




		//-----------------------------------------------------------------
		// 3. WSARecv()
		//-----------------------------------------------------------------

		WSABUF wsabuf[2];
		DWORD flag = 0, wsaBuffCnt = 1;

		wsabuf[0].buf = pSession->pRecvQ->GetRearPtr();
		wsabuf[0].len = pSession->pRecvQ->Getrear2endSize();

		if (pSession->pRecvQ->GetAvailableSize() > pSession->pRecvQ->Getrear2endSize())
		{
			wsabuf[1].buf = pSession->pRecvQ->GetBufferPtr();
			wsabuf[1].len = pSession->pRecvQ->GetAvailableSize() - pSession->pRecvQ->Getrear2endSize();

			++wsaBuffCnt;
		}


		retval = WSARecv(pSession->socket, wsabuf, wsaBuffCnt, 0, &flag, (OVERLAPPED*)&pSession->recvOverlap, NULL);


		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				pNetwork->DecrementUsingCnt(pSession);
			}
		}


	}



	return 0;
}






inline void CNetworkServer::CreateSession()
{
	for (int i = _maxSessionSize - 1; i >= 0; --i)
	{
		// 1. Init Session Data
		SESSION* newSession = new SESSION;
		newSession->usingCnt = 0;
		InitSession(newSession);



		// 2. Session Pool Array�� Session�� ����
		_sessionAry[i] = newSession;



		// 3. Empty Session Index�� Stack�� Push
		_emptySessionIndexStack.Free_Push(i);
	}

}



inline SESSION* CNetworkServer::AllocSession(SOCKET socket)
{
	SESSION* pSession = nullptr;

	//-------------------------------------------------------
	// 1. Session Pool List�� �� �� ���� �̱�
	//-------------------------------------------------------

	DWORD sessionAryIndex = NULL;
	if (_emptySessionIndexStack.Alloc_Pop(&sessionAryIndex) == false)
	{
		return nullptr;
	}
	pSession = _sessionAry[sessionAryIndex];



	//-------------------------------------------------------
	// 2. ������ ����
	//-------------------------------------------------------

	InitSession(pSession);

	pSession->sessionID = 0;
	pSession->sessionID = ((DWORD64)(++_sessionCnt) << 32) + sessionAryIndex;
	pSession->socket = socket;



	//-------------------------------------------------------
	// 3. Completion Port�� ������ ����
	//-------------------------------------------------------

	CreateIoCompletionPort((HANDLE)pSession->socket, _hWorkerCP, (ULONG_PTR)pSession, NULL);



	return pSession;
}




void CNetworkServer::CloseServer()
{
	if (_isServerOpened == false)
		return;

	_isServerOpened = false;

	//------------------------------------------------
	// 1. WorkerThread, AcceptThread ����
	//------------------------------------------------

	StopEveryThread();


	//------------------------------------------------
	// 2. ��� WorkerThread�� ���� ������ ���
	//------------------------------------------------

	WaitForMultipleObjects(_workerThreadPoolSize, _hWorkThreadAry, true, INFINITE);
	CLog::Log((WCHAR*)L"Server", CLog::LEVEL_SYSTEM, (WCHAR*)L"%ws", L"Every WorkerThread has been closed");



	//------------------------------------------------
	// 3. Empty Session Index Stack�� �ִ� ��� ��� ����
	//------------------------------------------------

	_emptySessionIndexStack.ReleaseEveryNode();
	delete[] _sessionAry;


	//------------------------------------------------
	// 4. WorkerThread Handle ����
	//------------------------------------------------

	for (int i = 0; i < _workerThreadPoolSize; i++)
		CloseHandle(_hWorkThreadAry[i]);

	delete[] _hWorkThreadAry;


	//------------------------------------------------
	// 5. WorkerCompletionPort ����
	//------------------------------------------------

	CloseHandle(_hWorkerCP);
	WSACleanup();
}



bool CNetworkServer::OpenServer(WCHAR* listenIP, int maxSessionSize, int port, int runningThreadSize, int workerThreadPoolSize)
{

	//------------------------------------------------
	// 1. ������ �ʱ�ȭ
	//------------------------------------------------

	lstrcpy(_listenIP, listenIP);
	_port = port;
	_maxSessionSize = maxSessionSize;
	_runningThreadSize = runningThreadSize;
	_workerThreadPoolSize = workerThreadPoolSize;

	_isServerOpened = true;
	_sessionCnt = 0;
	_acceptTPS = 0;
	_recvPacketTPS = 0;
	_sendPacketTPS = 0;

	_sessionAry = new SESSION * [_maxSessionSize];



	//------------------------------------------------
	// 2. ���� �迭 ���� �� �Ҵ�
	//------------------------------------------------

	CreateSession();



	//------------------------------------------------
	// 3. ��Ʈ��ũ �۾� & Worker/Accept ������ ����
	//------------------------------------------------

	if (!StartNetworking())
	{
		WSACleanup();

		return false;
	}

	return true;
}







inline void CNetworkServer::DecrementUsingCnt(SESSION* pSession)
{
	InterlockedDecrement((DWORD*)&pSession->usingCnt);


	if (InterlockedCompareExchange64((LONG64*)&pSession->usingCnt, 0x0000000100000000, (LONG64)0) == 0)
	{

		//--------------------------------------------------
		// 1. Session ������ �ʱ�ȭ
		//--------------------------------------------------

		
		OnLeaveServer(pSession->sessionID);
		SocketClose(pSession);
		DeleteEveryMsgInSendQ(pSession);



		//--------------------------------------------------
		// 2. Session Index Stack�� ����
		//--------------------------------------------------

		_emptySessionIndexStack.Free_Push((DWORD)pSession->sessionID);
		SetEvent(_hSessionFree);
	}

}



inline void CNetworkServer::SocketClose(SESSION* pSession)
{
	if (InterlockedExchange(&pSession->isSending, 1) != 0)
	{
		pSession->disconnectAfterSendFlag = true;
		return;
	}

	if (InterlockedExchange(&pSession->isSocketClosed, 1) == 0)
	{
		closesocket(pSession->socket);
		pSession->socket = INVALID_SOCKET;
	}

}




void CNetworkServer::StopEveryThread()
{
	//--------------------------------------------------
	// 1. Listen Socket�� �������ν�, AcceptThread ����
	//--------------------------------------------------

	if (_listenSock != INVALID_SOCKET)
	{
		closesocket(_listenSock);
	}


	//--------------------------------------------------
	// 2. Worker Completion Port�� NULL,NULL,NULL �޼����� ��Ŀ������ Ǯ ��ŭ ���������ν�, ��� WorkerThread ����
	//--------------------------------------------------

	for (int i = 0; i < _workerThreadPoolSize; i++)
	{
		PostQueuedCompletionStatus(_hWorkerCP, NULL, NULL, NULL);
	}
}






bool CNetworkServer::SeperateRecvMessage(SESSION* pSession, DWORD transferred)
{
	int msgLen;


	for (;;)
	{
		//------------------------------------------------------------
		// 1. RecvQ�� ��������ŭ ������� ������ �ݺ� ����
		//------------------------------------------------------------

		if (pSession->pRecvQ->GetUsingSize() < dfNETMESSAGE_HEADER_SIZE)
		{
			break;
		}



		//------------------------------------------------------------
		// 2. RecvQ���� ����ȭ���۷� �޼��� ��� ����
		//------------------------------------------------------------

		CSerializeBuffer* pMsg = CSerializeBuffer::Alloc();
		pMsg->AddUsingCnt();

		if (pSession->pRecvQ->Peek(pMsg->GetBufferZeroPtr(), dfNETMESSAGE_HEADER_SIZE) == false)
		{
			CSerializeBuffer::Free(pMsg);
			break;
		}



		//------------------------------------------------------------
		// 3. RecvQ���� msgLen��ŭ �̾Ƽ� ����ȭ���ۿ� ����
		//------------------------------------------------------------

		msgLen = pMsg->GetPayloadLen(true);

		if (pSession->pRecvQ->Peek(pMsg->GetBufferRearPtr(), msgLen, 5) == false)
		{
			CSerializeBuffer::Free(pMsg);
			break;
		}

		pSession->pRecvQ->MoveFront(msgLen + dfNETMESSAGE_HEADER_SIZE);




		//------------------------------------------------------------
		// 4. ����ȭ���� Rear�� msgLen��ŭ �б�
		//------------------------------------------------------------

		pMsg->MoveRearPos(msgLen);



		//------------------------------------------------------------
		// 5. ��� ������ üũ & ��ȣȭ
		//------------------------------------------------------------

		if (!pMsg->Decode())
		{
			CSerializeBuffer::Free(pMsg);
			return false;
		}

		InterlockedIncrement(&_recvPacketTPS);


		//------------------------------------------------------------
		// 6. �޼��� ó�� �Լ� ȣ��
		//------------------------------------------------------------

		OnRecv(pSession->sessionID, pMsg);
	}

	return true;
}



void CNetworkServer::RecvComplete(SESSION* pSession, DWORD dwTransferred)
{
	//------------------------------------------------------------
	// 1. ���� ��ŭ RecvQ�� Rear �б�
	//------------------------------------------------------------

	pSession->pRecvQ->MoveRear(dwTransferred);


	//------------------------------------------------------------
	// 2. �޼��� �ϳ��� �и��ؼ� ���������� �������� �Լ��� ����
	//------------------------------------------------------------

	if (!SeperateRecvMessage(pSession, dwTransferred))
	{
		SocketClose(pSession);
		return;
	}



	//------------------------------------------------------------
	// 3. WSABUF �ʱ�ȭ
	//------------------------------------------------------------

	WSABUF wsabuf[2];
	DWORD flag = 0;
	DWORD wsaBuffCnt = 1;


	if (pSession->pRecvQ->GetAvailableSize() <= pSession->pRecvQ->Getrear2endSize())
	{
		wsabuf[0].buf = pSession->pRecvQ->GetRearPtr();
		wsabuf[0].len = pSession->pRecvQ->GetAvailableSize();
	}

	else
	{
		wsabuf[0].buf = pSession->pRecvQ->GetRearPtr();
		wsabuf[0].len = pSession->pRecvQ->Getrear2endSize();

		wsabuf[1].buf = pSession->pRecvQ->GetBufferPtr();
		wsabuf[1].len = pSession->pRecvQ->GetAvailableSize() - pSession->pRecvQ->Getrear2endSize();

		++wsaBuffCnt;
	}



	//------------------------------------------------------------
	// 4. ���ī��Ʈ ����
	//------------------------------------------------------------

	InterlockedIncrement((DWORD*)&pSession->usingCnt);



	//------------------------------------------------------------
	// 5. WSARecv()
	//------------------------------------------------------------

	int retval = WSARecv(pSession->socket, wsabuf, wsaBuffCnt, 0, &flag, (OVERLAPPED*)&pSession->recvOverlap, NULL);




	//------------------------------------------------------------
	// 6. WSARecv Return value Ȯ��
	//------------------------------------------------------------

	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			DecrementUsingCnt(pSession);
		}
	}

}



void CNetworkServer::DeleteSendMsg(SESSION* pSession)
{
	//------------------------------------------------------------------
	// 1. ���� ������ �ʱ�ȭ
	//------------------------------------------------------------------

	DWORD sendMsgCnt = InterlockedExchange(&pSession->sendMsgCnt, 0);


	//------------------------------------------------------------------
	// 2. SendQ�� �ִ� Msg*���� SendMsgCnt��ŭ Dequeue
	//------------------------------------------------------------------

	for (int i = 0; i < sendMsgCnt; ++i)
	{
		CSerializeBuffer* pDeleteMsg = nullptr;


		if (pSession->sendQ.Dequeue(&pDeleteMsg) == false)
		{
			CCrashDump::Crash();
		}


		if (pDeleteMsg != nullptr)
		{
			CSerializeBuffer::Free(pDeleteMsg);
		}
		else
		{
			CCrashDump::Crash();
		}
	}

	InterlockedAdd((volatile LONG*)&_sendPacketTPS, sendMsgCnt);
}



void CNetworkServer::DeleteEveryMsgInSendQ(SESSION* pSession)
{

	for (;;)
	{
		CSerializeBuffer* pDeleteMsg = nullptr;


		if (pSession->sendQ.Dequeue(&pDeleteMsg) == false)
			break;



		if (pDeleteMsg != nullptr)
		{
			CSerializeBuffer::Free(pDeleteMsg);
		}
		else
			CCrashDump::Crash();

	}
}


