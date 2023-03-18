#include <Windows.h>
#include "CPUUsage.h"


CPUUsage::CPUUsage(HANDLE hProcess)
{
	m_hProcess = hProcess;


	// 프로세스 실행률 / CPU 개수 = 실제 CPU 사용률

	SYSTEM_INFO sysInfo;

	GetSystemInfo(&sysInfo);
	m_processorsSize = sysInfo.dwNumberOfProcessors;


	// Data Set 0

	m_processorTotal = 0;
	m_processorUser = 0;
	m_processorKernel = 0;

	m_processTotal = 0;
	m_processUser = 0;
	m_processKernel = 0;

	m_processor_LastKernel.QuadPart = 0;
	m_processor_LastUser.QuadPart = 0;
	m_processor_LastIdle.QuadPart = 0;

	m_process_LastKernel.QuadPart = 0;
	m_process_LastUser.QuadPart = 0;
	m_process_LastTime.QuadPart = 0;



	UpdateCPUTime();
}



CPUUsage::~CPUUsage()
{
}



void CPUUsage::UpdateCPUTime()
{
	ULARGE_INTEGER idle;
	ULARGE_INTEGER kernel;
	ULARGE_INTEGER user;


	// 시스템 사용 시간 구하기

	if (GetSystemTimes((PFILETIME)&idle, (PFILETIME)&kernel, (PFILETIME)&user) == false)
		return;


	ULONGLONG kernelDiff = kernel.QuadPart - m_processor_LastKernel.QuadPart;
	ULONGLONG userDiff = user.QuadPart - m_processor_LastUser.QuadPart;
	ULONGLONG idleDiff = idle.QuadPart - m_processor_LastIdle.QuadPart;

	ULONGLONG total = kernelDiff + userDiff;
	ULONGLONG timeDiff;


	if (total == 0)
	{
		m_processorUser= 0.0f;
		m_processorKernel = 0.0f;
		m_processorTotal = 0.0f;
	}

	else
	{
		// 커널 타임에 아이들 타임이 있으므로 빼서 계산
		m_processorTotal = (float)((double)(total - idleDiff) / total * 100.0f);
		m_processorUser = (float)((double)userDiff / total * 100.0f);
		m_processorKernel = (float)((double)(kernelDiff - idleDiff) / total * 100.0f);
	}


	m_processor_LastKernel = kernel;
	m_processor_LastUser = user;
	m_processor_LastIdle = idle;



	// 프로세스 사용률 갱신

	ULARGE_INTEGER none;
	ULARGE_INTEGER nowTime;



	// 프로세스 사용률 판단의 공식
	// a = 샘플간격의 시스템 시간 (그냥 지나간 시간)
	// b = 프로세스의 CPU 사용 시간

	// a : 100 = b : 사용률

	// 현재시간 구하기 (100나노세컨드 단위)
	GetSystemTimeAsFileTime((LPFILETIME)&nowTime);


	// 해당 프로세스가 사용한 시간 구하기
	// 2,3 인자는 실행,종료 시간으로 미사용
	GetProcessTimes(m_hProcess, (LPFILETIME)&none, (LPFILETIME)&none, (LPFILETIME)&kernel, (LPFILETIME)&user);



	// 이전에 저장된 프로세스 시간과의 차이를 구함

	timeDiff = nowTime.QuadPart - m_process_LastTime.QuadPart;
	userDiff = user.QuadPart - m_process_LastUser.QuadPart;
	kernelDiff = kernel.QuadPart - m_process_LastKernel.QuadPart;

	total = kernelDiff + userDiff;


	m_processTotal	= (float)(total / (double)m_processorsSize / (double)timeDiff * 100.0f);
	m_processKernel = (float)(kernelDiff / (double)m_processorsSize / (double)timeDiff * 100.0f);
	m_processUser	= (float)(userDiff / (double)m_processorsSize / (double)timeDiff * 100.0f);


	m_process_LastTime	= nowTime;
	m_process_LastKernel= kernel;
	m_process_LastUser	= user;
}
