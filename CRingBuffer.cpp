#include <iostream>
#include "CRingBuffer.h"



CRingBuffer::CRingBuffer()
{
	buff = new char[1000];
	buffSize = 1000;

	front = rear = 0;
}

CRingBuffer::CRingBuffer(int _buffSize)
{
	buff = new char[_buffSize];
	buffSize = _buffSize;

	front = rear = 0;
}


CRingBuffer::~CRingBuffer()
{
	delete[] buff;
}

int CRingBuffer::GetBuffSize()
{
	return buffSize;
}



int CRingBuffer::GetUsingSize()
{
	int Rear = rear;
	int Front = front;


	if (Rear > Front)
		return (Rear - Front);

	else if (Rear < Front)
		return (buffSize - (Front - Rear));

	else
		return 0;
}

int CRingBuffer::GetUsingSize(int Rear, int Front)
{
	if (Rear > Front)
		return (Rear - Front);

	else if (Rear < Front)
		return (buffSize - (Front - Rear));

	else
		return 0;
}

int CRingBuffer::GetAvailableSize()
{
	return (buffSize - GetUsingSize() - 1);
}

int CRingBuffer::Getfront2endSize()
{
	return (buffSize - front);
}

int CRingBuffer::Getfront2LastDataSize()
{
	int Rear = rear;
	int Front = front;
	int usingSize = GetUsingSize(Rear,Front);


	if (usingSize < (buffSize - Front))
		return Rear - Front;

	else if (usingSize >= (buffSize - Front))
		return buffSize - Front;
}

int CRingBuffer::Getrear2endSize()
{
	return (buffSize - rear);
}

bool CRingBuffer::Enqueue(char * pDataBuff, int size)
{
	int rear2EndSize = Getrear2endSize();
	int Rear = rear;


	if (GetAvailableSize() < size)
		return false;

	if (rear2EndSize >= size)
		memcpy_s(&buff[Rear], rear2EndSize, pDataBuff, size);

	else
	{
		memcpy_s(&buff[Rear], rear2EndSize, pDataBuff, rear2EndSize);
		memcpy_s(&buff[0], GetAvailableSize(), &pDataBuff[rear2EndSize], size - rear2EndSize);
	}

	rear = (Rear + size) % buffSize;


	return true;
}



bool CRingBuffer::Dequeue(char * pDequeueBuff, int size)
{
	int Front = front;
	int front2EndSize = buffSize - Front;


	if (GetUsingSize() < size)
		return false;



	if (front2EndSize >= size)
		memcpy_s(pDequeueBuff, size, &buff[Front], size);

	else
	{
		memcpy_s(pDequeueBuff, size, &buff[Front], front2EndSize);
		memcpy_s(&pDequeueBuff[front2EndSize], size - front2EndSize, &buff[0], size - front2EndSize);
	}


	front = (Front + size) % buffSize;

	return true;
}



bool CRingBuffer::Peek(char * pPeekBuff, int size, int peekIndex)
{
	int Front = (front + peekIndex) % buffSize;
	int front2EndSize = (buffSize - Front);


	if (GetUsingSize() < size + peekIndex)
		return false;



	if (front2EndSize >= size)
		memcpy_s(pPeekBuff, size, &buff[Front], size);

	else
	{
		memcpy_s(pPeekBuff, size, &buff[Front], front2EndSize);
		memcpy_s(&pPeekBuff[front2EndSize], size - front2EndSize, &buff[0], size - front2EndSize);
	}


	return true;
}



void CRingBuffer::MoveRear(int size)
{
	rear = (rear + size) % buffSize;
}

void CRingBuffer::MoveFront(int size)
{
	front = (front + size) % buffSize;
}

void CRingBuffer::ClearBuffer()
{
	rear = front;
}

char * CRingBuffer::GetFrontPtr()
{
	return &buff[front];
}

char * CRingBuffer::GetRearPtr()
{
	return &buff[rear];
}

