#include "ReplicatorSecHeader.h"

struct message ringBufGetMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs) {
	message ret;
	//EnterCriticalSection(cs);
	if (storingBuffer->count == 0)
	{
		ret.processId = -1;
	}
	else
	{
		int index;
		index = storingBuffer->head;
		storingBuffer->head = (storingBuffer->head + 1) % RING_SIZE;
		ret = storingBuffer->data[index];
		storingBuffer->count = storingBuffer->count - 1;
	}
	//LeaveCriticalSection(cs);
	return ret;
}
struct message ringBufReadMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs) {
	message ret;
	//EnterCriticalSection(cs);
	if (storingBuffer->count == 0)
	{
		ret.processId = -1;
	}
	else
	{
		int index = storingBuffer->head;
		ret = storingBuffer->data[index];
	}
	//LeaveCriticalSection(cs);
	return ret;
}
bool ringBufPutMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs, struct message m) {
	//EnterCriticalSection(cs);
	bool ret;
	if (storingBuffer->count < BUFFER_SIZE)
	{
		storingBuffer->data[storingBuffer->tail] = m;
		storingBuffer->tail = (storingBuffer->tail + 1) % RING_SIZE;
		storingBuffer->count = storingBuffer->count + 1;
		ret = true;
	}
	else
	{
		ret = false;
	}
	//LeaveCriticalSection(cs);
	return true;
}

void printBuffer(RingBuffer* storingBuffer, CRITICAL_SECTION* cs) {
	//EnterCriticalSection(cs);
	for (int i = storingBuffer->head;i < storingBuffer->tail;i++) {
		printf("Message: %s, ProcessId: %d.\n", storingBuffer->data[i].text, storingBuffer->data[i].processId);
	}
	//LeaveCriticalSection(cs);
}


struct retrievedData ringBufGetRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs)
{
	retrievedData ret;
	//EnterCriticalSection(cs);
	if (retrievingBuffer->count == 0)
	{
		ret.processId = -1;
	}
	else
	{
		int index;
		index = retrievingBuffer->head;
		retrievingBuffer->head = (retrievingBuffer->head + 1) % RING_SIZE;
		ret = retrievingBuffer->data[index];
		retrievingBuffer->count = retrievingBuffer->count - 1;
	}
	//LeaveCriticalSection(cs);
	return ret;

}
bool ringBufPutRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, struct retrievedData d)
{
	bool ret;
	//EnterCriticalSection(cs);
	if (retrievingBuffer->count == BUFFER_SIZE)
	{
		ret = false;
	}
	else
	{
		retrievingBuffer->data[retrievingBuffer->tail] = d;
		retrievingBuffer->tail = (retrievingBuffer->tail + 1) % RING_SIZE;
		retrievingBuffer->count = retrievingBuffer->count + 1;
		ret = true;
	}

	//LeaveCriticalSection(cs);
	return true;
}
void printBufferRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs)
{
	//EnterCriticalSection(cs);
	for (int i = retrievingBuffer->head;i < retrievingBuffer->tail;i++) {
		printf("ProcessId: %d.\nData:\n", retrievingBuffer->data[i].processId);
		for (int j = 0;j < retrievingBuffer->data[i].dataCount;j++)
		{
			printf("%s\n", retrievingBuffer->data[i].data[j]);
		}
	}
	//LeaveCriticalSection(cs);
}
struct retrievedData ringBufReadRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs)
{
	retrievedData ret;
	//EnterCriticalSection(cs);
	int index = retrievingBuffer->head;
	ret = retrievingBuffer->data[index];
	//LeaveCriticalSection(cs);
	return ret;
}
