#pragma once

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>

#include <string>
#include <Windows.h>

class Tls_DBConnector
{
public:
	Tls_DBConnector(std::string& schemaName, DWORD tlsIndex);
	~Tls_DBConnector();

	void	DisconnectDB();

	sql::ResultSet* SendQuery(OUT bool* bSucceed, sql::Statement* pStatement, const char* queryFormat, ...);

private:
	sql::Connection* GetDBConnect();


private:

	enum { COMPARE_TIME_toMAKELOG = 2000000 };
	DWORD	_tlsIndex;
	std::string	_schemaName;
};

