#pragma once
class CPUUsage
{
public:
	CPUUsage(HANDLE hProcess);
	~CPUUsage();

	void	UpdateCPUTime();

	float	GetProcessorTotal() { return m_processorTotal; }
	float	GetProcessorUser() { return m_processorUser; }
	float	GetProcessorKernel() { return m_processorKernel; }

	float	GetProcessTotal() { return m_processTotal; }
	float	GetProcessUser() { return m_processUser; }
	float	GetProcessKernel() { return m_processKernel; }



private:

	HANDLE	m_hProcess;
	int		m_processorsSize;

	float	m_processorTotal;
	float	m_processorUser;
	float	m_processorKernel;
			
	float	m_processTotal;
	float	m_processUser;
	float	m_processKernel;


	ULARGE_INTEGER m_processor_LastKernel;
	ULARGE_INTEGER m_processor_LastUser;
	ULARGE_INTEGER m_processor_LastIdle;

	ULARGE_INTEGER m_process_LastKernel;
	ULARGE_INTEGER m_process_LastUser;
	ULARGE_INTEGER m_process_LastTime;
};

// »ç¿ë¹ý

//CPUUsage* cpuTime = new CPUUsage(GetCurrentProcess());
//
//for (;;)
//{
//	cpuTime->UpdateCPUTime();
//
//	wprintf(L"Processor : %.1f / Process : %.1f\n", cpuTime->GetProcessorTotal(), cpuTime->GetProcessTotal());
//	wprintf(L"ProcessorKernel : %.1f / ProcesKernel : %.1f\n", cpuTime->GetProcessorKernel(), cpuTime->GetProcessKernel());
//	wprintf(L"ProcessorUser : %.1f / ProcesUser : %.1f\n\n\n", cpuTime->GetProcessorUser(), cpuTime->GetProcessUser());
//	Sleep(1000);
//}