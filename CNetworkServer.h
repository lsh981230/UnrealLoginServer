#pragma once
#pragma comment(lib,"ws2_32")
#pragma comment(lib,"winmm.lib")

#include "CRingBuffer.h"
#include "LockFreeQueue.h"
#include "LockFreeStack.h"
#include "MyOverlapped.h"
#include "CSerializeBuffer.h"

#define dfNETMESSAGE_HEADER_SIZE 5
#define dfRINGBUFFER_SIZE	 2000



struct SESSION
{
	SESSION() { pRecvQ = new CRingBuffer(dfRINGBUFFER_SIZE);}
	~SESSION() { delete pRecvQ; }


	DWORD		usingCnt;
	DWORD		releaseFlag;		// Release 할 수 있는 상태 : 0 / 할 수 없는 상태 : 1


	MY_OVERLAPPED recvOverlap;		
	MY_OVERLAPPED sendOverlap;		 
	MY_OVERLAPPED reqSendOverlap;
	
	DWORD		sendMsgCnt;

	volatile DWORD	isSending;
	DWORD		isSocketClosed;
	bool		isRecievedEndCode;
	bool		disconnectAfterSendFlag;

	char		ip[16];
	SOCKET		socket;
	DWORD64		sessionID;

	CRingBuffer* pRecvQ;
	LockFreeQueue<CSerializeBuffer*> sendQ;
};

struct Profiler;


class CNetworkServer
{
public:
	CNetworkServer();
	virtual ~CNetworkServer();

	bool OpenServer(WCHAR* listenIP, int maxSessionSize, int port, int runningThreadSize, int workerThreadPoolSize);
	void CloseServer();


protected:

	virtual void OnEnterServer(DWORD64 sessionID) = 0;
	virtual void OnLeaveServer(DWORD64 sessionID) = 0;
	virtual void OnRecv(DWORD64 sessionID, CSerializeBuffer* pMessage) = 0;

	void DisconnectSession(DWORD64 sessionID);

	void EnqueueMsg2SendQ(DWORD64 sessionID, CSerializeBuffer* pMessage);

	inline SESSION* GetSession(DWORD64 sessionID) { return _sessionAry[(DWORD)sessionID]; }
	inline LONG64 GetSessionIndexStackCount() { return _emptySessionIndexStack.GetStackCount(); }
	inline const char* GetSessionIPAddress(DWORD64 sessionID) { return _sessionAry[(DWORD)sessionID]->ip; }


private:

	static UINT WINAPI WorkerThread(LPVOID arg);
	static UINT WINAPI AcceptThread(LPVOID arg);


	inline void DecrementUsingCnt(SESSION* pSession);
	inline void SocketClose(SESSION* pSession);
	

	void RecvComplete(SESSION* pSession, DWORD dwTransferred);
	void SendPost(SESSION* pSession);
	void RequestSendPost(SESSION* pSession);

	bool SeperateRecvMessage(SESSION* pSession, DWORD transferred);
	void DeleteSendMsg(SESSION* pSession);
	void DeleteEveryMsgInSendQ(SESSION* pSession);

	void InitSession(SESSION * pSession);
	bool StartNetworking();
	void StopEveryThread();

	void CreateSession();
	inline SESSION* AllocSession(SOCKET socket);
	
public:

	bool		_isServerOpened;

protected:

	WCHAR		_listenIP[16];
	int			_maxSessionSize;
	int			_port;
	int			_runningThreadSize;
	int			_workerThreadPoolSize;

	__int64		_sessionCnt;
	DWORD		_acceptTPS;
	DWORD		_recvPacketTPS;
	DWORD		_sendPacketTPS;

private:

	SOCKET		_listenSock;
	HANDLE		_hWorkerCP;			// Completion Port Handle
	HANDLE*		_hWorkThreadAry;
	HANDLE		_hSessionFree;


	SESSION**	_sessionAry;
	LockFreeStack<DWORD> _emptySessionIndexStack;


};

