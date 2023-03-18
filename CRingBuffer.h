#pragma once
class CRingBuffer
{
public:
	CRingBuffer();
	CRingBuffer(int buffSize);
	~CRingBuffer();

	int GetBuffSize();
	int GetUsingSize();
	int GetUsingSize(int Rear, int Front);
	int GetAvailableSize();

	int Getfront2endSize();
	int Getfront2LastDataSize();
	int Getrear2endSize();
	int GetZero2rearSize() { return rear; }


	bool Enqueue(char* pEnqueueBuff, int size);
	bool Dequeue(char* pDequeueBuff, int size);
	bool Peek(char* pDequeueBuff, int size, int peekIndex = 0);

	void MoveRear(int size);
	void MoveFront(int size);

	void ClearBuffer();

	char* GetFrontPtr();
	char* GetRearPtr();
	char* GetBufferPtr() { return buff; }

private:
	char* buff;
	int front;
	int rear;
	int buffSize;
};

