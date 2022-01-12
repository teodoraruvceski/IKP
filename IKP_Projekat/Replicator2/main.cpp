#include "ReplicatorSecHeader.h"


int main()
{
	int replics[MAX_CLIENTS];
	InitReplicaArray(replics);
	SOCKET clientSocketsReplica[NUMOF_THREADS];

	CRITICAL_SECTION cs;
	CRITICAL_SECTION cs2;
	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&cs2);

	RingBuffer storingBuffer;
	RingBufferRetrieved retrievingBuffer;
	ThreadArgs threadArgs2[NUMOF_THREADS_SENDING];

	retrievingBuffer.head = 0;
	retrievingBuffer.tail = 0;
	retrievingBuffer.count = 0;

	storingBuffer.head = 0;
	storingBuffer.tail = 0;
	storingBuffer.count = 0;

	ConnectWithReplicator1(&storingBuffer,&retrievingBuffer,&cs,&cs2,threadArgs2,replics);
	ListenForReplica(&storingBuffer, &retrievingBuffer, &cs,&cs2, clientSocketsReplica);
	getch();
	return 0;
}

