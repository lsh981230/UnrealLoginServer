
#include <list>
#include <unordered_map>
#include <Windows.h>
#include <strsafe.h>

#include "CNetworkServer.h"
#include "LockFreeFreeList.h"
#include "LockFreeQueue.h"
#include "MemoryPool_TLS.h"
#include "CPUUsage.h"
#include "StructInfo.h"

class CSerializeBuffer;
class MonitoringClient;
class PDH;
//class TLS_RedisConnector;
class Tls_DBConnector;
struct MONITORING_CONFIG;

using namespace std;




enum DEFINE { TIMEOUT = 40000, MAX_SPEED = 1000 , INTERVAL_PLAYER_UPDATE = 200};


class LoginServer : public CNetworkServer
{
public:

	LoginServer();
	virtual ~LoginServer();

	void	PrintData();
	void	OpenGameServer(ConfigGameServer* pConfig);


private:
	enum enPDHIndex { PRIVATE_MEM = 0, NON_PAGED_POOL, NET_SENT };
	enum enSendingType { SEND_TO_EVERYONE, SEND_TO_ME, SEND_TO_SOMEONE, SEND_TO_EVERYONE_EXCEPT_ME };

	//-----------------------------------------------------------------------------
	// Handler Functions
	
	virtual void OnRecv(DWORD64 sessionID, CSerializeBuffer* pMessage);
	virtual void OnEnterServer(DWORD64 sessionID);
	virtual void OnLeaveServer(DWORD64 sessionID);



	//-----------------------------------------------------------------------------
	// Logic Thread Functions

	inline void SwitchMessage(CSerializeBuffer* pMsg, DWORD64 sessionID);


	//-----------------------------------------------------------------------------

	void Request_Login(CSerializeBuffer* pMsg, DWORD64 sessionID);
	void Request_Regist(CSerializeBuffer* pMsg, DWORD64 sessionID);



private:

	// Game
	int _beginLevel;
	int _beginMaxHP;
	int _beginAttackDamage;

	// Connector
	Tls_DBConnector* _pAccountDBConnector;
	Tls_DBConnector* _pGameDBConnector;


	// Monitoring
	MonitoringClient* _pMonitoringClient;
	CPUUsage* _pCpuUsage;
	PDH* _pdh;
};