#pragma once
// Kruzni bafer - FIFO 
#define RING_SIZE 50
#define BUFFER_SIZE 4096

struct message {
	char text[BUFFER_SIZE];
	short serviceId;
};

struct RingBuffer {
	unsigned int tail;
	unsigned int head;
	struct message data[RING_SIZE];
};
// Operacije za rad sa kruznim baferom 
struct message ringBufGetMessage(RingBuffer* apBuffer);
void ringBufPutMessage(RingBuffer* apBuffer, struct message m);
void printBuffer();