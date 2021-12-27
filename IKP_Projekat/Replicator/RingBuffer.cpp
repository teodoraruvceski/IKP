#include "ReplicatorPrimHeader.h"

struct message ringBufGetMessage(RingBuffer* apBuffer) {
	message ret;
	EnterCriticalSection(&cs);
	if (apBuffer->count==0)
	{
		ret.processId = -1;
	}
	else
	{
		int index;
		index = apBuffer->head;
		apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
		ret = apBuffer->data[index];
		apBuffer->count = apBuffer->count - 1;
	}
	LeaveCriticalSection(&cs);
	return ret;
}
struct message ringBufReadMessage(RingBuffer* apBuffer) {
	message ret;
	EnterCriticalSection(&cs);
	if (apBuffer->count == 0)
	{
		ret.processId = -1;
	}
	else
	{
		int index = apBuffer->head;
		ret = apBuffer->data[index];
	}
	LeaveCriticalSection(&cs);
	return ret;
}
bool ringBufPutMessage(RingBuffer* apBuffer, struct message m) {
	EnterCriticalSection(&cs);
	bool ret;
	if (apBuffer->count<BUFFER_SIZE)
	{
		apBuffer->data[apBuffer->tail] = m;
		apBuffer->tail = (apBuffer->tail + 1) % RING_SIZE;
		apBuffer->count = apBuffer->count + 1;
		ret = true;
	}
	else
	{
		ret = false;
	}
	LeaveCriticalSection(&cs);
	return true;
}

void printBuffer(RingBuffer *apBuffer) {
	EnterCriticalSection(&cs);
	for (int i = apBuffer->head;i < apBuffer->tail;i++) {
		printf("Message: %s, ProcessId: %d.\n", apBuffer->data[i].text, apBuffer->data[i].processId);
	}
	LeaveCriticalSection(&cs);
}


struct retrievedData ringBufGetRetrievedData(RingBufferRetrieved* apBuffer)
{
	retrievedData ret;
	EnterCriticalSection(&cs);
	if (apBuffer->count==0)
	{
		ret.processId = -1;
	}
	else
	{
		int index;
		index = apBuffer->head;
		apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
		ret = apBuffer->data[index];
		apBuffer->count = apBuffer->count - 1;
	}
	LeaveCriticalSection(&cs);
	return ret;
	
}
bool ringBufPutRetrievedData(RingBufferRetrieved* apBuffer, struct retrievedData d)
{
	bool ret;
	EnterCriticalSection(&cs);
	if (apBuffer->count==BUFFER_SIZE)
	{
		ret = false;
	}
	else
	{
		apBuffer->data[apBuffer->tail] = d;
		apBuffer->tail = (apBuffer->tail + 1) % RING_SIZE;
		apBuffer->count = apBuffer->count + 1;
		ret = true;
	}
	
	LeaveCriticalSection(&cs);
	return true;
}
void printBufferRetrievedData(RingBufferRetrieved* apBuffer)
{
	EnterCriticalSection(&cs);
	for (int i = apBuffer->head;i < apBuffer->tail;i++) {
		printf("ProcessId: %d.\nData:\n", apBuffer->data[i].processId);
		for (int j = 0;j < apBuffer->data[i].dataCount;j++)
		{
			printf("%s\n", apBuffer->data[i].data[j]);
		}
	}
	LeaveCriticalSection(&cs);
}
struct retrievedData ringBufReadRetrievedData(RingBufferRetrieved* apBuffer)
{
	retrievedData ret;
	EnterCriticalSection(&cs);
	int index = apBuffer->head;
	ret= apBuffer->data[index];
	LeaveCriticalSection(&cs);
	return ret;
}
