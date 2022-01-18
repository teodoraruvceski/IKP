#include "ReplicatorPrimHeader.h"

int main()
{
	SOCKET clientSocketsRep2[NUMOF_THREADS_RECV];//sockets for listening to rep2
	SOCKET connectSocketRep2[NUMOF_THREADS_SENDING];//sockets for sending data to rep2
	SOCKET clientSocketsProcess[MAX_CLIENTS]; //sockets for listening to processes

	bool endReplicator1 = false;
	
	RingBuffer storingBuffer;
	RingBufferRetrieved retrievingBuffer;
	CRITICAL_SECTION cs;
	CRITICAL_SECTION cs2;

	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&cs2);

	retrievingBuffer.head = 0;
	retrievingBuffer.tail = 0;
	retrievingBuffer.count = 0;

	storingBuffer.head = 0;
	storingBuffer.tail = 0;
	storingBuffer.count = 0;

	DWORD ListenForRegistrationsThreadID[MAX_CLIENTS];
	HANDLE hListenForRegistrationsThread[MAX_CLIENTS];

	DWORD ConnectWithReplicator2ThreadID[NUMOF_THREADS_SENDING];
	HANDLE hConnectWithReplicator2Thread[NUMOF_THREADS_SENDING];

	DWORD ListenForRegistrationsID;
	HANDLE hListenForRegistrations;

	DWORD ListenForRecvRep2ThreadID[NUMOF_THREADS_RECV];
	HANDLE hListenForRecvRep2Thread[NUMOF_THREADS_RECV];

	ConnectWithReplicator2(&storingBuffer, &retrievingBuffer,&cs, &cs2, clientSocketsRep2, connectSocketRep2, 
		ConnectWithReplicator2ThreadID, hConnectWithReplicator2Thread, ListenForRecvRep2ThreadID, hListenForRecvRep2Thread,&endReplicator1);
	ListenForRegistrations(&storingBuffer, &retrievingBuffer, &cs, &cs2, clientSocketsProcess, ListenForRegistrationsThreadID, hListenForRegistrationsThread, &endReplicator1);
	printf("====================Threads ended=======================");
	printf("Closing Handles...");
	for (int i = 0; i < NUMOF_THREADS_SENDING; i++)
	{
		CloseHandle(hConnectWithReplicator2Thread[i]);
	}
	for (int i = 0; i < NUMOF_THREADS_RECV; i++)
	{
		CloseHandle(hListenForRecvRep2Thread[i]);
	}
	/*for (int i = 0; i < 1; i++)
	{
		try {
			CloseHandle(hListenForRegistrationsThread[i]);
		}
		catch (...) {}
		
	}*/
	printf("Replicator1 gracefully shut down.");
	return 0;
}
