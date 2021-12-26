#pragma once
// Kruzni bafer - FIFO 
#define RING_SIZE 50
#define BUFFER_SIZE 4096


struct message {
	char text[BUFFER_SIZE];
	short processId;
};

struct retrievedData {
	char** data;
	short processId;
	int dataCount;
};
struct RingBufferRetrieved {
	unsigned int tail;
	unsigned int head;
	struct retrievedData data[RING_SIZE];
};
struct RingBuffer {
	unsigned int tail;
	unsigned int head;
	struct message data[RING_SIZE];
};
// Operacije za rad sa kruznim baferom 
struct message ringBufGetMessage(RingBuffer* apBuffer, CRITICAL_SECTION* cs);
void ringBufPutMessage(RingBuffer* apBuffer, struct message m, CRITICAL_SECTION* cs);
void printBuffer(RingBuffer apBuffer, CRITICAL_SECTION* cs);
struct message ringBufReadMessage(RingBuffer* apBuffer, CRITICAL_SECTION* cs);

struct retrievedData ringBufGetRetrievedData(RingBufferRetrieved* apBuffer, CRITICAL_SECTION* cs);
void ringBufPutRetrievedData(RingBufferRetrieved* apBuffer, struct retrievedData d, CRITICAL_SECTION* cs);
void printBufferRetrievedData(RingBufferRetrieved apBuffer, CRITICAL_SECTION* cs);
struct retrievedData ringBufReadRetrievedData(RingBufferRetrieved* apBuffer, CRITICAL_SECTION* cs);