#include <Windows.h>
#include "CPUUsage.h"


CPUUsage::CPUUsage(HANDLE hProcess)
{
	m_hProcess = hProcess;


	// ���μ��� ����� / CPU ���� = ���� CPU ����

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


	// �ý��� ��� �ð� ���ϱ�

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
		// Ŀ�� Ÿ�ӿ� ���̵� Ÿ���� �����Ƿ� ���� ���
		m_processorTotal = (float)((double)(total - idleDiff) / total * 100.0f);
		m_processorUser = (float)((double)userDiff / total * 100.0f);
		m_processorKernel = (float)((double)(kernelDiff - idleDiff) / total * 100.0f);
	}


	m_processor_LastKernel = kernel;
	m_processor_LastUser = user;
	m_processor_LastIdle = idle;



	// ���μ��� ���� ����

	ULARGE_INTEGER none;
	ULARGE_INTEGER nowTime;



	// ���μ��� ���� �Ǵ��� ����
	// a = ���ð����� �ý��� �ð� (�׳� ������ �ð�)
	// b = ���μ����� CPU ��� �ð�

	// a : 100 = b : ����

	// ����ð� ���ϱ� (100���뼼���� ����)
	GetSystemTimeAsFileTime((LPFILETIME)&nowTime);


	// �ش� ���μ����� ����� �ð� ���ϱ�
	// 2,3 ���ڴ� ����,���� �ð����� �̻��
	GetProcessTimes(m_hProcess, (LPFILETIME)&none, (LPFILETIME)&none, (LPFILETIME)&kernel, (LPFILETIME)&user);



	// ������ ����� ���μ��� �ð����� ���̸� ����

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
