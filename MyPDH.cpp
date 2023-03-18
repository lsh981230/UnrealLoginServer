
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
	// 1. 해당 쿼리가 사용가능한지 확인
	//--------------------------------------------------------

	if (PdhValidatePath(counterName) != ERROR_SUCCESS)
		return false;



	//--------------------------------------------------------
	// 2. 카운터 할당
	//--------------------------------------------------------

	HCOUNTER* newCounter = new HCOUNTER;



	//--------------------------------------------------------
	// 3. 맵에 카운터 삽입
	//--------------------------------------------------------

	if (_counterMap[counterIndex] == NULL)
		_counterMap[counterIndex] = newCounter;
	else
		return false;


	//--------------------------------------------------------
	// 4. 카운터 추가
	//--------------------------------------------------------

	if (PdhAddCounter(_hQuery, counterName, 0, newCounter) != ERROR_SUCCESS)
	{
		_counterMap.erase(counterIndex);
		delete newCounter;
		return false;
	}
	


	//--------------------------------------------------------
	// 5. 포맷된 값을 표시하기 위해 두 개의 샘플 값을 위해 Update
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
	// 1. 카운터 맵에서 카운터 찾기
	//--------------------------------------------------------

	HCOUNTER* pCounter = _counterMap[counterIndex];
	if (pCounter == NULL)
		return PDH_ERROR;



	//--------------------------------------------------------
	// 2. 카운터에서 값 뽑기
	//--------------------------------------------------------

	PdhGetFormattedCounterValue(*pCounter, PDH_FMT_DOUBLE, &counterType, &counterValue);



	//--------------------------------------------------------
	// 3. 뽑은 값 리턴
	//--------------------------------------------------------

	return counterValue.doubleValue;
}
