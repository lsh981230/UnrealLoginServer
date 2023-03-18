#pragma once

#include <list>
#include <Windows.h>
#include "MemoryPool_TLS.h"
#include "StructInfo.h"

#define	ENCODE_VALUE		0x32
#define BUFF_CHECKCODE		0x77
#define CODE_INDEX			0
#define PAYLOAD_LEN_INDEX	1
#define RANDOMKEY_INDEX		3
#define CHECKSUM_INDEX		4
#define PAYLOAD_INDEX		5


#pragma pack(push,1)
struct HeaderOfSerializeBuffer
{
	BYTE code;
	WORD len;
	BYTE randomKey;
	BYTE checkSum;
};
#pragma pack(pop)


class CSerializeBuffer
{
public:
	enum en_MSGBUFFER { DEFAULT_MAX_BUFFSIZE = 2000 };

	virtual ~CSerializeBuffer() 
	{
		delete[] pBuffer;
	}


	// Memory Pool Function
	//---------------------------------------
	static void SetMaxBuffSize(DWORD buffSize) { _maxBuffSize = buffSize; }
	static CSerializeBuffer* Alloc();
	static void Free(CSerializeBuffer* pBuffer);
	static LONG	GetAllocCnt() { return m_bufferPool->GetAllocCnt(); }
	static LONG	GetUsingCnt() { return m_bufferPool->GetUseCount(); }

	static std::list<CSerializeBuffer*> m_list;

	inline void AddUsingCnt() { InterlockedIncrement(&m_usingCnt); }

	// ¾ÏÈ£È­
	//---------------------------------------
	void	Encode();
	bool	Decode();
	WORD	GetPayloadLen(bool b5ByteHeader);


	void	Init();
	inline	void	Release()			{ delete[] pBuffer; }
	inline	int		GetBufferSize()		{ return bufferSize; }
	inline	int		GetUsingDataSize()	{ return usingDataSize; }
	inline	char*	GetBufferRearPtr()	{ return &pBuffer[rear]; }
	inline	char*	GetBufferFrontPtr() { return &pBuffer[front]; }
	inline	char*	GetBufferZeroPtr()	{ return pBuffer; }
	int		MoveRearPos(int size)		{ if (size < 0) return -1; rear += size; usingDataSize += size; return 0;}
	int		MoveFrontPos(int size)		{ if (size < 0) return -1; front += size; usingDataSize -= size; return 0;}
	int		GetData(char* pDest, int size);
	int		PutData(char* pDest, int size);
	int		PutWCHARData(WCHAR* pDest, int size);

	char*	GetHeaderPtr();
	void	PutHeaderData(WORD* pHeader = nullptr, bool bHeaderSize5Byte = true);


	CSerializeBuffer& operator <<(BYTE value) { PutData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator <<(char value) { PutData((char*)&value, sizeof(value)); return *this; }
	//CSerializeBuffer& operator <<(WCHAR* pBuff) { PutWCHARData(pBuff, wcslen(pBuff) * sizeof(WCHAR)); return *this; }

	CSerializeBuffer& operator <<(short value) { PutData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator <<(WORD value) { PutData((char*)&value, sizeof(value)); return *this; }

	CSerializeBuffer& operator <<(int value) { PutData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator <<(DWORD value) { PutData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator <<(DWORD64 value) { PutData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator <<(float value) { PutData((char*)&value, sizeof(value)); return *this; }

	CSerializeBuffer& operator <<(__int64 value) { PutData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator <<(double value) { PutData((char*)&value, sizeof(value)); return *this; }

	CSerializeBuffer& operator >>(BYTE& value) { GetData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator >>(char& value) { GetData((char*)&value, sizeof(value)); return *this; }


	CSerializeBuffer& operator >>(short& value) { GetData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator >>(WORD& value) { GetData((char*)&value, sizeof(value)); return *this; }

	CSerializeBuffer& operator >>(int& value) { GetData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator >>(DWORD& value) { GetData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator >>(float& value) { GetData((char*)&value, sizeof(value)); return *this; }

	CSerializeBuffer& operator >>(__int64& value) { GetData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator >>(DWORD64& value) { GetData((char*)&value, sizeof(value)); return *this; }
	CSerializeBuffer& operator >>(double& value) { GetData((char*)&value, sizeof(value)); return *this; }


	template<size_t N>
	inline CSerializeBuffer& operator >>(WCHAR(&value)[N]) { GetData((char*)value, N * 2); return *this; }

	template<size_t N>
	inline CSerializeBuffer& operator <<(WCHAR(&value)[N]) { PutData((char*)value, N * 2);  return *this; }

	template<size_t N>
	inline CSerializeBuffer& operator >>(char(&value)[N]) { GetData((char*)value, N); return *this; }

	template<size_t N>
	inline CSerializeBuffer& operator <<(char(&value)[N]) { PutData((char*)value, N);  return *this; }

	DWORD encodeFlag;
	bool	disconnectFlag;
private:

	CSerializeBuffer();
	friend class MemoryPool_TLS<CSerializeBuffer>;

private:

	int rear;
	int front;
	DWORD m_usingCnt;


	int bufferSize;
	WORD usingDataSize;

	char* pBuffer;
	bool m_bHeaderSize5Byte;
	bool m_bBufferAlloc = false;

private:

	static DWORD _maxBuffSize;
	static MemoryPool_TLS<CSerializeBuffer> *m_bufferPool;
};

