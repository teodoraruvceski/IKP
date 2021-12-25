#include "ReplicatorSecHeader.h"

struct message ringBufGetMessage(RingBuffer* apBuffer) {
	int index;
	index = apBuffer->head;
	apBuffer->head = (apBuffer->head + 1) % RING_SIZE;
	return apBuffer->data[index];
}
void ringBufPutMessage(RingBuffer* apBuffer, struct message m) {
	apBuffer->data[apBuffer->tail] = m;
	apBuffer->tail = (apBuffer->tail + 1) % RING_SIZE;
}

void printBuffer(RingBuffer* apBuffe) {
	for (int i = 0;i < RING_SIZE;i++) {
		printf("Message: %s, service ID: %d.\n", *apBuffe->data->text, apBuffe->data->serviceId);
	}
}