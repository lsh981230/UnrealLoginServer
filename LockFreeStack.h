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

		// ���� ��Ȳ : *m_top�� ���� pTopCopy�� �����ϴ� ������, �ش� �������� �ּҿ� �����Ͽ� 1����Ʈ�� �ݺ��ϸ� �������ִ� �����̴�.
		//            �׷��� �ݺ��ϸ� �������ִ� �������� �߰��� m_top�� ���� �ٸ� �����忡 ���� �ٲ�� ���� ����Ǵٰ� �߰����� �ٸ� ���� ���簡 �Ǵ� ������ �޴´�.
		//			  �� ���󿡼� �쿬�� nodeID�� pNode�� �ּҰ��� ���ٸ�? 




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