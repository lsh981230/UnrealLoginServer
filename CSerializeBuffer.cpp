#include "CSerializeBuffer.h"

MemoryPool_TLS<CSerializeBuffer>* CSerializeBuffer::m_bufferPool = new MemoryPool_TLS<CSerializeBuffer>(true, TlsAlloc(), false);
//CMemoryPool<CSerializeBuffer>* CSerializeBuffer::_bufferPool = new CMemoryPool<CSerializeBuffer>();
//SRWLOCK* CSerializeBuffer::_poolLock = new SRWLOCK;
DWORD CSerializeBuffer::_maxBuffSize = DEFAULT_MAX_BUFFSIZE;

CSerializeBuffer::CSerializeBuffer()
{
	pBuffer = new char[_maxBuffSize];

	m_bHeaderSize5Byte = false;

	m_usingCnt = 0;
	rear = 5;
	front = 5;
	usingDataSize = 0;
	encodeFlag = 0;
	bufferSize = DEFAULT_MAX_BUFFSIZE - 5;

	disconnectFlag = false;
}





//------------------------------------------------------------------
// Memory Pool Function (Start)




CSerializeBuffer* CSerializeBuffer::Alloc()
{
	CSerializeBuffer* allocBuffer = m_bufferPool->Alloc();
	//AcquireSRWLockExclusive(_poolLock);
	//CSerializeBuffer* allocBuffer = _bufferPool->Alloc();
	//ReleaseSRWLockExclusive(_poolLock);

	allocBuffer->Init();


	return allocBuffer;
}




void CSerializeBuffer::Free(CSerializeBuffer* pBuffer)
{

	if (InterlockedDecrement(&pBuffer->m_usingCnt) == 0)
	{
		pBuffer->encodeFlag = 0;

		m_bufferPool->Free(pBuffer);
		//AcquireSRWLockExclusive(_poolLock);
		//_bufferPool->Free(pBuffer);
		//ReleaseSRWLockExclusive(_poolLock);
	}
}






// Memory Pool Function	(End)
//------------------------------------------------------------------







WORD CSerializeBuffer::GetPayloadLen(bool header5Byte)
{
	if (header5Byte)
		return *((WORD*)&pBuffer[PAYLOAD_LEN_INDEX]);
	else
		return*((WORD*)&pBuffer[0]);
}



void CSerializeBuffer::Encode()
{
	//--------------------------------------------
	// 0. 예외 처리
	//--------------------------------------------
	// 하나의 메세지를 여러 섹터에 뿌려주는 경우, 한번만 암호화 시키기 위한 예외 처리
	if (InterlockedExchange(&encodeFlag, 1) != 0)
		return;


	//--------------------------------------------
	// 1. 암호화
	//--------------------------------------------

	BYTE lastP = 0;
	BYTE lastE = 0;
	BYTE randomKey = pBuffer[RANDOMKEY_INDEX];
	char* pData = &pBuffer[CHECKSUM_INDEX];

	int repeatTime = usingDataSize + 1;		// CheckSum부터 마지막 Data까지
	for (int i = 0; i < repeatTime; ++i)
	{
		BYTE E = *(pData + i) ^ (lastP + randomKey + i + 1);
		lastP = E;

		*(pData + i) = E ^ (lastE + ENCODE_VALUE + i + 1);
		lastE = *(pData + i);
	}

}





bool CSerializeBuffer::Decode()
{
	// 1. CheckCode 확인
	BYTE code = pBuffer[CODE_INDEX];
	if (code != BUFF_CHECKCODE)
		return false;



	// 2. 복호화
	BYTE p = 0;
	BYTE lastP = 0;
	BYTE lastE = 0;
	BYTE randomKey = pBuffer[RANDOMKEY_INDEX];


	int repeatTime = usingDataSize + 1;		// CheckSum부터 마지막 Data까지
	for (int i = 0; i < repeatTime; ++i)
	{
		p = pBuffer[i + 4] ^ (lastE + ENCODE_VALUE + i + 1);
		lastE = pBuffer[i + 4];

		pBuffer[i + 4] = p ^ (lastP + randomKey + i + 1);
		lastP = p;
	}


	// 3. CheckSum 구하기
	BYTE checkSum = 0;
	for (int i = 0; i < usingDataSize; ++i)
	{
		checkSum += pBuffer[PAYLOAD_INDEX + i];
	}
	checkSum %= 256;


	// 4. CheckSum 확인
	BYTE buffCheckSum = pBuffer[CHECKSUM_INDEX];
	if (buffCheckSum != checkSum)
	{
		return false;
	}

	InterlockedExchange(&encodeFlag, 0);

	return true;
}








void CSerializeBuffer::Init()
{
	m_bHeaderSize5Byte = false;

	m_usingCnt = 0;
	rear = 5;
	front = 5;

	usingDataSize = 0;
	encodeFlag = 0;
	bufferSize = DEFAULT_MAX_BUFFSIZE - 5;
	disconnectFlag = false;
}








int CSerializeBuffer::GetData(char* pDest, int size)
{
	if (size <= 0 || pDest == nullptr)
		return -1;
	if (usingDataSize < size)
		return -1;


	memcpy_s(pDest, size, &pBuffer[front], size);
	front += size;
	usingDataSize -= size;

	return size;
}




int CSerializeBuffer::PutData(char* pDest, int size)
{
	if (size <= 0 || pDest == nullptr)
		CCrashDump::Crash();

	if ((bufferSize - rear) < size)
		CCrashDump::Crash();

	memcpy_s(&pBuffer[rear], size, pDest, size);
	rear += size;
	usingDataSize += size;

	return size;
}





int CSerializeBuffer::PutWCHARData(WCHAR* pDest, int size)
{
	if (size <= 0 || pDest == nullptr)
		return -1;
	if ((bufferSize - rear) < size)
		return -1;


	wmemcpy_s((WCHAR*)&pBuffer[rear], size, pDest, size);
	rear += size;
	usingDataSize += size;

	return size;
}





char* CSerializeBuffer::GetHeaderPtr()
{
	if (m_bHeaderSize5Byte)
		return pBuffer;
	else
		return &pBuffer[3];
}




void CSerializeBuffer::PutHeaderData(WORD* pHeader, bool bHeaderSize5Byte)
{
	m_bHeaderSize5Byte = bHeaderSize5Byte;


	if (bHeaderSize5Byte)
	{
		UINT checkSum = 0;
		for (int i = 0; i < usingDataSize; ++i)
		{
			checkSum += (BYTE)pBuffer[PAYLOAD_INDEX + i];
		}
		checkSum %= 256;

		HeaderOfSerializeBuffer* header = (HeaderOfSerializeBuffer*)pBuffer;

		header->code = BUFF_CHECKCODE;
		header->len = usingDataSize;
		header->randomKey = (rand() % 256);
		header->checkSum = (BYTE)checkSum;
	}
	else
	{
		memcpy_s(&pBuffer[3], 2, pHeader, 2);
	}
}
