#include "ReplicatorSecHeader.h"


int main()
{
	SOCKET clientSocketsReplica[NUMOF_THREADS];
	 CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	RingBuffer storingBuffer;
	RingBufferRetrieved retrievingBuffer;

	retrievingBuffer.head = 0;
	retrievingBuffer.tail = 0;
	retrievingBuffer.count = 0;

	storingBuffer.head = 0;
	storingBuffer.tail = 0;
	storingBuffer.count = 0;

	ListenForReplicator1Registrations(&storingBuffer,&retrievingBuffer,&cs);
	ListenForReplica(&storingBuffer, &retrievingBuffer, &cs, clientSocketsReplica);
	_getch();
	return 0;
}

