#pragma once

#pragma comment(lib, "pdh.lib")
#include <Pdh.h>
#include <unordered_map>

enum enPDHIndex { PRIVATE_MEM = 0, NON_PAGED_POOL, NET_SENT };

class PDH
{
public:
	PDH();
	~PDH();

	enum { PDH_ERROR = -1 };

	bool	AddCounter(WCHAR* counterName, int counterIndex);

	void	Update();

	__int64	GetCounterValue(int counterIndex);

private:

	std::unordered_map<int, HCOUNTER*> _counterMap;
	HQUERY		_hQuery;

};

