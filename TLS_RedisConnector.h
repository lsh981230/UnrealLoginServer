//#pragma once
//#include <cpp_redis/cpp_redis>
//#pragma comment (lib, "cpp_redis.lib")
//#pragma comment (lib, "tacopie.lib")
////#pragma comment (lib, "ws2_32.lib")
//#include "LockFreeQueue.h"
//#include "Profiler.h"
//
//class TLS_RedisConnector
//{
//public:
//	TLS_RedisConnector(DWORD tlsRedisIndex);
//	virtual ~TLS_RedisConnector();
//
//
//	void SetPair(char* key, char* value);
//	void GetValue(char* key, cpp_redis::reply* pValue);
//	bool isConnect() { return GetConnection()->is_connected(); }
//
//private:
//	cpp_redis::client* GetConnection();
//
//private:
//	DWORD _tlsRedisIndex;
//	LockFreeQueue<cpp_redis::client*> _connectionQueue;
//
//public:
//
//	Profiler	_setProfiler;
//
//};
//
