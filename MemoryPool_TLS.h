#pragma once

#include <iostream>
#include <Windows.h>

#include <list>
#include "LockFreeFreeList.h"
#include "CCrashDump.h"

#define CHECKCODE	0x36
#define MAX_CHUNK_SIZE	100



template <typename DATA>
class MemoryPool_TLS
{
private:
	struct Chunk;
	struct Node
	{
		Node() {}
		DATA	data;
		Chunk*	pChunk;
		BYTE	checkSum;
	};


	struct Chunk
	{
		Node*	nodeAry;
		LONG	allocCnt;
		LONG	freeCnt;


		Chunk()
		{
			if (MemoryPool_TLS<DATA>::bNodePlacement)
				nodeAry = (Node*)malloc(sizeof(Node) * MAX_CHUNK_SIZE);
			else
				nodeAry = new Node[MAX_CHUNK_SIZE];
		}


		inline void Init()
		{
			allocCnt = 0;
			freeCnt = 0;


			for (int i = 0; i < MAX_CHUNK_SIZE; ++i)
			{
				nodeAry[i].pChunk = this;
				nodeAry[i].checkSum = CHECKCODE;
			}
		}

		inline DATA* Alloc()
		{
			return &nodeAry[allocCnt++].data;
		}
	};

	SRWLOCK lock;


private:
	static bool	bNodePlacement;

public:

	std::list<Node*> m_allocNodeList;


	MemoryPool_TLS(bool bChunkPlacementNew, DWORD TLSIndex, bool NodePlacementNew)
	{
		m_useCount = 0;
		m_tlsIndex = TLSIndex;
		m_bPlacementNew = bChunkPlacementNew;
		m_memoryPool = new LockFreeFreeList<Chunk>(false);

		bNodePlacement = NodePlacementNew;
	}

	~MemoryPool_TLS()
	{
		TlsFree(m_tlsIndex);
	}

	LONG GetUseCount() { return m_useCount; }
	LONG GetAllocCnt() { return m_memoryPool->GetAllocNodeCnt() * MAX_CHUNK_SIZE; }





	DATA*	Alloc()
	{
		InterlockedIncrement(&m_useCount);

		// 1. Thread Local Storage에 m_tlsIndex번째에 있는 pChunk값 가져오기
		Chunk* pChunk = (Chunk*)TlsGetValue(m_tlsIndex);



		// 2. pChunk가 nullptr인 경우엔 Chunk 메모리풀에서 Alloc
		if (pChunk == nullptr)
		{
			pChunk = m_memoryPool->Alloc();


			pChunk->Init();


			TlsSetValue(m_tlsIndex, pChunk);
		}


		// 3. pChunk의 allocCnt + 1이 MAX인 경우
		else if (pChunk->allocCnt + 1 == MAX_CHUNK_SIZE)
		{
			DATA* returnData = (DATA*)&pChunk->nodeAry[pChunk->allocCnt++];
			TlsSetValue(m_tlsIndex, nullptr);

			if (!m_bPlacementNew)
				new (&pChunk->nodeAry[pChunk->allocCnt])Node();

			return returnData;
		}

		Node* returnData = &pChunk->nodeAry[pChunk->allocCnt++];

		if (!m_bPlacementNew)
			new (&pChunk->nodeAry[pChunk->allocCnt])Node();

		return (DATA*)returnData;
	}






	void	Free(DATA* pData)
	{
		Node* pNode = (Node*)pData;

		if (pNode->checkSum != CHECKCODE)
		{
			CCrashDump::Crash();
		}


		InterlockedDecrement(&m_useCount);

		/*AcquireSRWLockExclusive(&lock);
		auto iterEnd = m_allocNodeList.end();
		for (auto iter = m_allocNodeList.begin(); iter != iterEnd; ++iter)
		{
			if (*iter == (Node*)pData)
			{
				m_allocNodeList.erase(iter);
				break;
			}
		}
		ReleaseSRWLockExclusive(&lock);*/

		if (InterlockedIncrement(&pNode->pChunk->freeCnt) == MAX_CHUNK_SIZE)
		{
			m_memoryPool->Free(pNode->pChunk);
		}
	}







private:
	DWORD	 m_tlsIndex;
	LockFreeFreeList<Chunk>* m_memoryPool;
	LONG	 m_useCount;


	bool	 m_bPlacementNew;

};


template <typename DATA>
bool MemoryPool_TLS<DATA>::bNodePlacement = false;