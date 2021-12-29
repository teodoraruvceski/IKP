#include "ReplicatorSecHeader.h"

struct message ringBufGetMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs) {
	message ret;
	RingBuffer* apBuffer = storingBuffer;
	EnterCriticalSection(cs);
	if (apBuffer->count == 0)
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
	LeaveCriticalSection(cs);
	return ret;
}
struct message ringBufReadMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs) {
	message ret;
	RingBuffer* apBuffer = storingBuffer;
	EnterCriticalSection(cs);
	if (apBuffer->count == 0)
	{
		ret.processId = -1;
	}
	else
	{
		int index = apBuffer->head;
		ret = apBuffer->data[index];
	}
	LeaveCriticalSection(cs);
	return ret;
}
bool ringBufPutMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs, struct message m) {
	RingBuffer* apBuffer = storingBuffer;
	EnterCriticalSection(cs);
	bool ret;
	if (apBuffer->count < BUFFER_SIZE)
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
	LeaveCriticalSection(cs);
	return true;
}

void printBuffer(RingBuffer* storingBuffer, CRITICAL_SECTION* cs) {
	RingBuffer* apBuffer = storingBuffer;
	EnterCriticalSection(cs);
	for (int i = apBuffer->head;i < apBuffer->tail;i++) {
		printf("Message: %s, ProcessId: %d.\n", apBuffer->data[i].text, apBuffer->data[i].processId);
	}
	LeaveCriticalSection(cs);
}


struct retrievedData ringBufGetRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs)
{
	retrievedData ret;
	RingBufferRetrieved* apBuffer = retrievingBuffer;
	EnterCriticalSection(cs);
	if (apBuffer->count == 0)
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
	LeaveCriticalSection(cs);
	return ret;

}
bool ringBufPutRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, struct retrievedData d)
{
	bool ret;
	RingBufferRetrieved* apBuffer = retrievingBuffer;
	EnterCriticalSection(cs);
	if (apBuffer->count == BUFFER_SIZE)
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

	LeaveCriticalSection(cs);
	return true;
}
void printBufferRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs)
{
	RingBufferRetrieved* apBuffer = retrievingBuffer;
	EnterCriticalSection(cs);
	for (int i = apBuffer->head;i < apBuffer->tail;i++) {
		printf("ProcessId: %d.\nData:\n", apBuffer->data[i].processId);
		for (int j = 0;j < apBuffer->data[i].dataCount;j++)
		{
			printf("%s\n", apBuffer->data[i].data[j]);
		}
	}
	LeaveCriticalSection(cs);
}
struct retrievedData ringBufReadRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs)
{
	RingBufferRetrieved* apBuffer = retrievingBuffer;
	retrievedData ret;
	EnterCriticalSection(cs);
	int index = apBuffer->head;
	ret = apBuffer->data[index];
	LeaveCriticalSection(cs);
	return ret;
}
