#pragma once


#include <Windows.h>





template <typename DATA>
class LockFreeFreeList
{
private:


	struct Node
	{
		DATA    data;
		Node*   pNext;
	};


	struct MyTop
	{
		Node*   pNode = nullptr;
		LONG64  nodeID;
	};




public:
	LockFreeFreeList(bool bPlacementNew = true)
	{
		m_useCount = 0;
		m_stackCount = 0;
		m_allocNodeCnt = 0;

		m_top = new MyTop;
		m_top->nodeID = 0;


		m_bPlacementNew = bPlacementNew;
	}

	~LockFreeFreeList()
	{
		DeleteAll();
	}



	void Free(DATA* pData)
	{
		Node* pPushNode = (Node*)pData;


		if (pPushNode == nullptr)
			return;



		Node* pCopyTopNode = nullptr;


		do
		{
			pCopyTopNode = m_top->pNode;
			pPushNode->pNext = pCopyTopNode;
		} while ((LONG64)pCopyTopNode != InterlockedCompareExchange64((volatile LONG64*)&m_top->pNode, (LONG64)pPushNode, (LONG64)pCopyTopNode));



		InterlockedIncrement(&m_stackCount);
		InterlockedDecrement(&m_useCount);
	}




	DATA* Alloc()
	{
		if (InterlockedDecrement(&m_stackCount) < 0)
		{
			InterlockedIncrement(&m_stackCount);
			InterlockedIncrement(&m_useCount);



			return CreateNode();
		}




		MyTop pTopCopy;



		do
		{
			pTopCopy = *m_top;

		} while (!InterlockedCompareExchange128((LONG64*)m_top, m_top->nodeID + 1, (LONG64)pTopCopy.pNode->pNext, (LONG64*)&pTopCopy));


		InterlockedIncrement(&m_useCount);


		return &pTopCopy.pNode->data;
	}




	DATA*	 CreateNode()
	{
		Node* pNewNode;
		if (m_bPlacementNew)
			pNewNode = (Node*)malloc(sizeof(Node));
		else
			pNewNode = new Node();

		pNewNode->pNext = nullptr;


		InterlockedIncrement(&m_allocNodeCnt);

		return (DATA*)pNewNode;
	}


	LONG	GetStackCnt() { return m_stackCount; }
	LONG64  GetTopID() { return m_top->nodeID; }
	LONG	GetAllocNodeCnt() { return m_allocNodeCnt; }
	LONG    GetUseCount() { return m_useCount; }
	void	DeleteAll()
	{
		Node* pNextNode = nullptr;
		Node* pDeleteNode = m_top->pNode;


		LONG deleteSize = m_allocNodeCnt - m_useCount;

		for (LONG i = 0; i < deleteSize; ++i)
		{
			pNextNode = pDeleteNode->pNext;

			if (pNextNode == nullptr)
				break;

			delete pDeleteNode;

			pDeleteNode = pNextNode;
		}
	}


private:

	MyTop		*m_top;
	volatile LONG			m_stackCount;


	LONG			m_allocNodeCnt;
	LONG			m_useCount;

	bool   m_bPlacementNew;
};