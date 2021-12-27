#include "ReplicatorSecHeader.h"


int main()
{

	InitializeCriticalSection(&cs);
	RingBuffer storingBuffer;
	RingBufferRetrieved retrievingBuffer;

	retrievingBuffer.head = 0;
	retrievingBuffer.tail = 0;
	retrievingBuffer.count = 0;

	storingBuffer.head = 0;
	storingBuffer.tail = 0;
	storingBuffer.count = 0;

	ListenForReplicator1Registrations();
	_getch();
	return 0;
}

