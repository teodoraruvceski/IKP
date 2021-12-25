#include "ReplicatorPrimHeader.h"

struct message ringBufGetMessage(RingBuffer* apBuffer) {
	int index;
	index = apBuffer->head;
	apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
	return apBuffer->data[index];
}
struct message ringBufReadMessage(RingBuffer* apBuffer) {
	int index = apBuffer->head;
	return apBuffer->data[index];
}
void ringBufPutMessage(RingBuffer* apBuffer, struct message m) {
	apBuffer->data[apBuffer->tail] = m;
	apBuffer->tail = (apBuffer->tail + 1) % RING_SIZE;
}

void printBuffer(RingBuffer apBuffer) {
	for (int i = 0;i < apBuffer.tail-apBuffer.head;i++) {
		printf("Message: %s, ProcessId: %d.\n", apBuffer.data[i].text, apBuffer.data[i].processId);
	}
}


struct retrievedData ringBufGetRetrievedData(RingBufferRetrieved* apBuffer)
{
	int index;
	index = apBuffer->head;
	apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
	return apBuffer->data[index];
}
void ringBufPutRetrievedData(RingBufferRetrieved* apBuffer, struct retrievedData d)
{
	apBuffer->data[apBuffer->tail] = d;
	apBuffer->tail = (apBuffer->tail + 1) % RING_SIZE;
}
void printBufferRetrievedData(RingBufferRetrieved apBuffer)
{
	
}
struct retrievedData ringBufReadRetrievedDataID(RingBufferRetrieved* apBuffer)
{
	int index = apBuffer->head;
	return apBuffer->data[index];
}
