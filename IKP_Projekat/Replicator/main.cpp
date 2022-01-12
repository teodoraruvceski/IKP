#include "ReplicatorPrimHeader.h"

int main()
{
	SOCKET clientSocketsRep2[NUMOF_THREADS_RECV];//sockets for listening to rep2
	SOCKET connectSocketRep2[NUMOF_THREADS_SENDING];//sockets for sending data to rep2
	SOCKET clientSocketsProcess[MAX_CLIENTS]; //sockets for listening to processes

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

	DWORD ListenForRegistrationsID;
	HANDLE hListenForRegistrations;

	ConnectWithReplicator2(&storingBuffer, &retrievingBuffer,&cs, &cs2, clientSocketsRep2, connectSocketRep2);
	ListenForRegistrations(&storingBuffer, &retrievingBuffer, &cs, &cs2, clientSocketsProcess);
	getch();
	return 0;
}
