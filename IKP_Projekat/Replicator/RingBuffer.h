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
struct message ringBufGetMessage(RingBuffer* apBuffer);
void ringBufPutMessage(RingBuffer* apBuffer, struct message m);
void printBuffer(RingBuffer apBuffer);
struct message ringBufReadMessage(RingBuffer* apBuffer);

struct retrievedData ringBufGetRetrievedData(RingBufferRetrieved* apBuffer);
void ringBufPutRetrievedData(RingBufferRetrieved* apBuffer, struct retrievedData d);
void printBufferRetrievedData(RingBufferRetrieved apBuffer);
struct retrievedData ringBufReadRetrievedData(RingBufferRetrieved* apBuffer);