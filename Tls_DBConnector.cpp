#include <strsafe.h>
#include <Windows.h>

#include "Tls_DBConnector.h"
#include "CLog.h"

using namespace std;

Tls_DBConnector::Tls_DBConnector(string& schemaName, DWORD tlsIndex)
{
	_schemaName = schemaName;
	_tlsIndex = tlsIndex;
}

Tls_DBConnector::~Tls_DBConnector()
{
	TlsFree(_tlsIndex);
}


sql::Connection* Tls_DBConnector::GetDBConnect()
{
	sql::Connection* con = (sql::Connection*)TlsGetValue(_tlsIndex);

	if (con == nullptr)
	{
		sql::Driver* driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
		con->setSchema(_schemaName);
		TlsSetValue(_tlsIndex, con);
	}

	return con;
}



// DB Connector 사용한 모든 스레드에서 스레드 종료 전 호출, 안하면 mysql 커넥션 남아있음
void Tls_DBConnector::DisconnectDB()
{
	delete GetDBConnect();
}




sql::ResultSet* Tls_DBConnector::SendQuery(OUT bool* bSucceed, sql::Statement* pStatement, const char* queryFormat, ...)
{
	// 1. 가변인자로 들어온 쿼리 포맷에 맞춰 쿼리 생성

	char query[512] = "";
	va_list arg;
	va_start(arg, queryFormat);
	StringCchVPrintfA(query, sizeof(query) / sizeof(char), queryFormat, arg);
	va_end(arg);


	sql::Connection* con = GetDBConnect();
	pStatement = con->createStatement();
	sql::ResultSet* res = nullptr;

	*bSucceed = true;

	try
	{
		res = pStatement->executeQuery(query);
	}

	catch (sql::SQLException& e)
	{
		int errorCode = e.getErrorCode();
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;


		if (errorCode != 0)
		{
			*bSucceed = false;

			CLog::Log(L"DB", CLog::LEVEL_ERROR, L"[Tls_DBConnector::SendQuery()], Error : %s, code : %d", (char*)e.what(), e.getErrorCode());

			if (res != nullptr)
			{
				delete res;
				res = nullptr;
			}
		}
	}


	delete pStatement;

	return res;
}
