#pragma once

#include <Windows.h>

#include "LockFreeFreeList.h"
#include "CCrashDump.h"



template <typename DATA>
class LockFreeQueue
{
private:
	struct Node
	{
		Node* pNext;
		DATA data;
	};



public:
	struct MyNode
	{
		Node*   pNode;
		LONG64   nodeID;
	};

	LockFreeQueue()
	{
		Node* dummyRearNode = nodeList.Alloc();
		dummyRearNode->pNext = nullptr;


		m_rear = new MyNode;
		m_rear->pNode = dummyRearNode;
		m_rear->nodeID = 0;

		m_front = new MyNode;
		m_front->pNode = dummyRearNode;
		m_front->nodeID = 0;

		m_queueSize = 0;
	}



	bool Peek(DATA* pData, DWORD index)
	{
		if (m_front->pNode == nullptr)
			CCrashDump::Crash();



		Node* returnDataNode = m_front->pNode;
		for (DWORD i = 0; i <= index; ++i)
		{
			if (returnDataNode->pNext == nullptr)
				return false;


			returnDataNode = returnDataNode->pNext;
		}


		if (returnDataNode == nullptr)
			CCrashDump::Crash();



		*pData = returnDataNode->data;

		return true;
	}




	void Enqueue(DATA data)
	{
		Node* newNode = nodeList.Alloc();


		newNode->pNext = nullptr;
		newNode->data = data;



		MyNode copyRear;
		copyRear.pNode = nullptr;



		for (;;)
		{
			copyRear = *m_rear;

			// Rear 밀기
			if (copyRear.pNode->pNext != nullptr)
			{
				InterlockedCompareExchange128((LONG64*)m_rear, m_rear->nodeID + 1, (LONG64)copyRear.pNode->pNext, (LONG64*)&copyRear);
				continue;
			}


			// CAS 1 : Copy Rear->pNext가 nullptr이라면 자기 노드를 넣음
			if (NULL == InterlockedCompareExchange64((LONG64*)&copyRear.pNode->pNext, (LONG64)newNode, NULL))
				break;
		}

		// Rear 밀기 (CAS 2)
		InterlockedCompareExchange128((LONG64*)m_rear, m_rear->nodeID + 1, (LONG64)newNode, (LONG64*)&copyRear);




		InterlockedIncrement(&m_queueSize);
	}








	bool Dequeue(DATA* pData)
	{
		if (InterlockedDecrement(&m_queueSize) < 0)
		{
			InterlockedIncrement(&m_queueSize);
			return false;
		}


		MyNode copyFront;
		copyFront.pNode = nullptr;

		MyNode copyRear;
		copyRear.pNode = nullptr;





		for (;;)
		{
			// 1. Front Node 널 체크
			if (m_front->pNode->pNext == nullptr)
				continue;
			


			// 2. Rear 밀기
			copyRear = *m_rear;
			if (copyRear.pNode->pNext != nullptr)
			{
				InterlockedCompareExchange128((LONG64*)m_rear, m_rear->nodeID + 1, (LONG64)copyRear.pNode->pNext, (LONG64*)&copyRear);
			}




			// 3. Front 값 복사
			copyFront = *m_front;
			if (copyFront.pNode->pNext == nullptr)
				continue;




			// 5. Front 밀기
			if (InterlockedCompareExchange128((LONG64*)m_front, (LONG64)copyFront.nodeID + 1, (LONG64)m_front->pNode->pNext, (LONG64*)&copyFront))
				break;

		}



		*pData = copyFront.pNode->pNext->data;

		nodeList.Free(copyFront.pNode);




		return true;
	}


	void ClearQueue()
	{
		while (m_queueSize != 0)
		{
			DATA returnData;

			Dequeue(&returnData);
		}
	}


	LONG GetQueueSize() { return m_queueSize; }


	private:
		LockFreeFreeList<Node> nodeList;
		MyNode*   m_front;
		MyNode*   m_rear;


		LONG   m_queueSize;
};