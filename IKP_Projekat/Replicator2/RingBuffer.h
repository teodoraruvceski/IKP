#pragma once
// Kruzni bafer - FIFO 
#define RING_SIZE 50
#define BUFFER_SIZE 4096
#define TEXT_SIZE 2048


struct message {
	char text[TEXT_SIZE];
	short processId;
};

struct retrievedData {
	char data[TEXT_SIZE];
	short processId;
	int dataCount;
};
struct RingBufferRetrieved {
	unsigned int tail;
	unsigned int head;
	int count;
	struct retrievedData data[RING_SIZE];
};
struct RingBuffer {
	unsigned int tail;
	unsigned int head;
	int count;
	struct message data[RING_SIZE];
};
// operations for ringBuffer
struct message ringBufGetMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs);
bool ringBufPutMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs, struct message m);
void printBuffer(RingBuffer* storingBuffer, CRITICAL_SECTION* cs);
struct message ringBufReadMessage(RingBuffer* storingBuffer, CRITICAL_SECTION* cs);

struct retrievedData ringBufGetRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs);
bool ringBufPutRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, struct retrievedData d);
void printBufferRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs);
struct retrievedData ringBufReadRetrievedData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs);