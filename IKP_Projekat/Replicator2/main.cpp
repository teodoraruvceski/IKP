#include "ReplicatorSecHeader.h"


int main()
{
	bool end = false;
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

	DWORD ListenForReplicator1ThreadID[NUMOF_THREADS];
	HANDLE hListenForReplicator1Thread[NUMOF_THREADS];

	DWORD SendToReplicator1ThreadID[NUMOF_THREADS_SENDING];
	HANDLE hSendToReplicator1Thread[NUMOF_THREADS_SENDING];

	DWORD ListenForReplicaThreadID[MAX_CLIENTS];
	HANDLE hListenForReplicaThread[MAX_CLIENTS];



	ConnectWithReplicator1(&storingBuffer,&retrievingBuffer,&cs,&cs2,threadArgs2,replics, ListenForReplicator1ThreadID, hListenForReplicator1Thread, SendToReplicator1ThreadID, hSendToReplicator1Thread,&end);
	ListenForReplica(&storingBuffer, &retrievingBuffer, &cs,&cs2, clientSocketsReplica, ListenForReplicaThreadID, hListenForReplicaThread,&end);
	printf("Threads ended.");
	printf("Closing Handles...");
	//getch();
	for (int i = 0; i < NUMOF_THREADS; i++)
	{
		TerminateThread(hListenForReplicator1Thread[i], 0);
		CloseHandle(hListenForReplicator1Thread[i]);
	}
	for (int i = 0; i < NUMOF_THREADS_SENDING; i++)
	{
		TerminateThread(hSendToReplicator1Thread[i], 0);
		CloseHandle(hSendToReplicator1Thread[i]);
	}
	for (int i = 0; i < 1; i++)
	{
		try {
			TerminateThread(hListenForReplicaThread[i], 0);
			CloseHandle(hListenForReplicaThread[i]);
		}
		catch (...) {}
		
	}
	printf("Replicator2 gracefully shut down.");
	return 0;
}

