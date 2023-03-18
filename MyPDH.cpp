
using namespace std;

#include "MyPDH.h"


PDH::PDH()
{
	PdhOpenQuery(NULL, 0, &_hQuery);
}


PDH::~PDH()
{
	for (auto pair : _counterMap)
	{
		delete pair.second;
		_counterMap.erase(pair.first);
	}


	PdhCloseQuery(_hQuery);
}



bool PDH::AddCounter(WCHAR * counterName, int counterIndex)
{

	//--------------------------------------------------------
	// 1. �ش� ������ ��밡������ Ȯ��
	//--------------------------------------------------------

	if (PdhValidatePath(counterName) != ERROR_SUCCESS)
		return false;



	//--------------------------------------------------------
	// 2. ī���� �Ҵ�
	//--------------------------------------------------------

	HCOUNTER* newCounter = new HCOUNTER;



	//--------------------------------------------------------
	// 3. �ʿ� ī���� ����
	//--------------------------------------------------------

	if (_counterMap[counterIndex] == NULL)
		_counterMap[counterIndex] = newCounter;
	else
		return false;


	//--------------------------------------------------------
	// 4. ī���� �߰�
	//--------------------------------------------------------

	if (PdhAddCounter(_hQuery, counterName, 0, newCounter) != ERROR_SUCCESS)
	{
		_counterMap.erase(counterIndex);
		delete newCounter;
		return false;
	}
	


	//--------------------------------------------------------
	// 5. ���˵� ���� ǥ���ϱ� ���� �� ���� ���� ���� ���� Update
	//--------------------------------------------------------

	Update();


	return true;
}




void PDH::Update()
{
	PdhCollectQueryData(_hQuery);
}





__int64 PDH::GetCounterValue(int counterIndex)
{
	DWORD counterType;
	PDH_FMT_COUNTERVALUE counterValue;



	//--------------------------------------------------------
	// 1. ī���� �ʿ��� ī���� ã��
	//--------------------------------------------------------

	HCOUNTER* pCounter = _counterMap[counterIndex];
	if (pCounter == NULL)
		return PDH_ERROR;



	//--------------------------------------------------------
	// 2. ī���Ϳ��� �� �̱�
	//--------------------------------------------------------

	PdhGetFormattedCounterValue(*pCounter, PDH_FMT_DOUBLE, &counterType, &counterValue);



	//--------------------------------------------------------
	// 3. ���� �� ����
	//--------------------------------------------------------

	return counterValue.doubleValue;
}
