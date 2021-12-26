#include "ReplicatorPrimHeader.h"

struct message ringBufGetMessage(RingBuffer* apBuffer, CRITICAL_SECTION* cs) {
	message ret;
	EnterCriticalSection(cs);
	if (apBuffer->head == apBuffer->tail)
	{
		LeaveCriticalSection(cs);
		ret.processId = -1;
		return ret;
	}
	int index;
	index = apBuffer->head;
	apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
	ret= apBuffer->data[index];
	LeaveCriticalSection(cs);
	return ret;
}
struct message ringBufReadMessage(RingBuffer* apBuffer, CRITICAL_SECTION* cs) {
	message ret;
	EnterCriticalSection(cs);
	int index = apBuffer->head;
	ret= apBuffer->data[index];
	LeaveCriticalSection(cs);
	return ret;
}
void ringBufPutMessage(RingBuffer* apBuffer, struct message m, CRITICAL_SECTION* cs) {
	EnterCriticalSection(cs);
	if ((apBuffer->tail + 1) % RING_SIZE == apBuffer->tail)
		apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
	apBuffer->data[apBuffer->tail] = m;
	apBuffer->tail = (apBuffer->tail + 1) % RING_SIZE;
	LeaveCriticalSection(cs);
}

void printBuffer(RingBuffer apBuffer, CRITICAL_SECTION* cs) {
	EnterCriticalSection(cs);
	for (int i = apBuffer.head;i < apBuffer.tail;i++) {
		printf("Message: %s, ProcessId: %d.\n", apBuffer.data[i].text, apBuffer.data[i].processId);
	}
	LeaveCriticalSection(cs);
}


struct retrievedData ringBufGetRetrievedData(RingBufferRetrieved* apBuffer, CRITICAL_SECTION* cs)
{
	retrievedData ret;
	EnterCriticalSection(cs);
	if (apBuffer->head == apBuffer->tail)
	{
		ret.processId = -1;
		return ret;
	}
	int index;
	index = apBuffer->head;
	apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
	ret= apBuffer->data[index];
	return ret;
	LeaveCriticalSection(cs);
}
void ringBufPutRetrievedData(RingBufferRetrieved* apBuffer, struct retrievedData d, CRITICAL_SECTION* cs)
{
	EnterCriticalSection(cs);
	apBuffer->data[apBuffer->tail] = d;
	if((apBuffer->tail + 1) % RING_SIZE==apBuffer->head)
		apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
	apBuffer->tail = (apBuffer->tail + 1) % RING_SIZE;
	LeaveCriticalSection(cs);
}
void printBufferRetrievedData(RingBufferRetrieved apBuffer, CRITICAL_SECTION* cs)
{
	EnterCriticalSection(cs);
	for (int i = apBuffer.head;i < apBuffer.tail;i++) {
		printf("ProcessId: %d.\nData:\n", apBuffer.data[i].processId);
		for (int j = 0;j < apBuffer.data[i].dataCount;j++)
		{
			printf("%s\n", apBuffer.data[i].data[j]);
		}
	}
	LeaveCriticalSection(cs);
}
struct retrievedData ringBufReadRetrievedData(RingBufferRetrieved* apBuffer, CRITICAL_SECTION* cs)
{
	retrievedData ret;
	EnterCriticalSection(cs);
	int index = apBuffer->head;
	ret= apBuffer->data[index];
	LeaveCriticalSection(cs);
	return ret;
}
