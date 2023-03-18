#pragma once

#include <Windows.h>

#include "LockFreeFreeList.h"





template <typename DATA>
class LockFreeStack
{
private:
	struct Node
	{
		Node*   pNext;
		DATA    data;
	};


	struct MyTop
	{
		Node*   pNode = nullptr;
		LONG64  nodeID;
	};


public:
	LockFreeStack()
	{
		m_stackCount = 0;

		m_top = new MyTop;
		m_top->nodeID = 0;
	}

	~LockFreeStack()
	{

	}



	void Free_Push(DATA data)
	{
		Node* pNewNode = nodeFreeList.Alloc();
		pNewNode->data = data;


		Node* pCopyTopNode = nullptr;


		do
		{
			pCopyTopNode = m_top->pNode;

			pNewNode->pNext = pCopyTopNode;
		} while ((LONG64)pCopyTopNode != InterlockedCompareExchange64((volatile LONG64*)&m_top->pNode, (LONG64)pNewNode, (LONG64)pCopyTopNode));



		InterlockedIncrement64(&m_stackCount);
	}





	bool Alloc_Pop(DATA* returnData)
	{

		if (InterlockedDecrement64(&m_stackCount) < 0)
		{
			InterlockedIncrement64(&m_stackCount);
			return false;
		}


		MyTop pTopCopy;
		Node* pSecondNode;



		do
		{
			pTopCopy = *m_top;

			pSecondNode = m_top->pNode->pNext;
		} while (!InterlockedCompareExchange128((volatile LONG64*)m_top, (LONG64)m_top->nodeID + 1, (LONG64)pSecondNode, (LONG64*)&pTopCopy));

		// 문제 상황 : *m_top의 값을 pTopCopy로 대입하는 과정이, 해당 변수들의 주소에 접근하여 1바이트씩 반복하며 복사해주는 과정이다.
		//            그런데 반복하며 복사해주는 과정에서 중간에 m_top의 값이 다른 쓰레드에 의해 바뀌면 값이 복사되다가 중간부터 다른 값이 복사가 되는 현상을 겪는다.
		//			  이 현상에서 우연히 nodeID와 pNode의 주소값이 같다면? 




		*returnData = pTopCopy.pNode->data;
		nodeFreeList.Free(pTopCopy.pNode);
		pTopCopy.pNode = nullptr;

		return true;
	}




	LONG	 GetAllocNodeCnt()	{ return nodeFreeList.GetAllocNodeCnt(); }
	LONG64   GetStackCount()	{ return m_stackCount; }
	void	 ReleaseEveryNode();


private:

	MyTop		*m_top;


	volatile LONG64		m_stackCount;

	LockFreeFreeList<Node> nodeFreeList;
};





template<typename DATA>
inline void LockFreeStack<DATA>::ReleaseEveryNode()
{
	DATA deleteData;


	LONG64 stackCnt = m_stackCount;
	for (LONG64 i = 0; i < stackCnt; ++i)
	{
		Alloc_Pop(&deleteData);
	}
}