#include "ReplicatorSecHeader.h"


int main()
{
	int replics[MAX_CLIENTS];
	InitReplicaArray(replics);//niz koji cuva sve id replika stavljamo na -1;
	SOCKET clientSocketsReplica[NUMOF_THREADS];
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	RingBuffer storingBuffer;
	RingBufferRetrieved retrievingBuffer;
	ThreadArgs threadArgs2[NUMOF_THREADS_SENDING];
	retrievingBuffer.head = 0;
	retrievingBuffer.tail = 0;
	retrievingBuffer.count = 0;


	storingBuffer.head = 0;
	storingBuffer.tail = 0;
	storingBuffer.count = 0;

	ListenForReplicator1Registrations(&storingBuffer,&retrievingBuffer,&cs,threadArgs2,replics);
	ListenForReplica(&storingBuffer, &retrievingBuffer, &cs, clientSocketsReplica);
	_getch();
	return 0;
}

