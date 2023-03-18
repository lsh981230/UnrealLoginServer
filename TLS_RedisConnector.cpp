//#include <iostream>
//#include "TLS_RedisConnector.h"
//
//TLS_RedisConnector::TLS_RedisConnector(DWORD tlsRedisIndex)
//{
//	_tlsRedisIndex = tlsRedisIndex;
//
//	WORD version = MAKEWORD(2, 2);
//	WSADATA data;
//	WSAStartup(version, &data);
//}
//
//TLS_RedisConnector::~TLS_RedisConnector()
//{
//	int queueSize = _connectionQueue.GetQueueSize();
//	for (int i = 0; i < queueSize; i++)
//	{
//		cpp_redis::client* connection = nullptr;
//		_connectionQueue.Dequeue(&connection);
//		connection->disconnect();
//		delete connection;
//	}
//
//}
//
//void TLS_RedisConnector::SetPair(char* key, char* value)
//{
//	cpp_redis::client* connection = GetConnection();
//
//	auto cStart = std::chrono::system_clock::now();
//	connection->set(key, value);
//	connection->sync_commit();
//	auto cEnd = std::chrono::system_clock::now();
//
//	_setProfiler.Renew(std::chrono::duration_cast<std::chrono::microseconds>(cEnd - cStart).count());
//}
//
//void TLS_RedisConnector::GetValue(char* key, cpp_redis::reply* pValue)
//{
//	cpp_redis::client* connection = GetConnection();
//
//	auto cStart = std::chrono::system_clock::now();
//	std::future<cpp_redis::reply> get_reply = connection->get(key);
//	connection->sync_commit();
//	auto cEnd = std::chrono::system_clock::now();
//
//	_setProfiler.Renew(std::chrono::duration_cast<std::chrono::microseconds>(cEnd - cStart).count());
//
//	*pValue = get_reply.get();
//}
//
//cpp_redis::client* TLS_RedisConnector::GetConnection()
//{
//	cpp_redis::client* connection = nullptr;
//	connection = (cpp_redis::client*)TlsGetValue(_tlsRedisIndex);
//
//	if (connection == nullptr)
//	{
//		connection = new cpp_redis::client();
//		connection->connect();		// 로그인서버랑 Redis랑 같은 로컬에 있는거 아니면 매개변수로 IP,포트 값 줘야함
//
//		TlsSetValue(_tlsRedisIndex, connection);
//	}
//
//	return connection;
//}
