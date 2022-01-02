//#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>

#include "ReplicatorPrimHeader.h"
// TCP server that use non-blocking sockets
//extern RingBuffer* storingBuffer;
//extern RingBufferRetrieved* retrievingBuffer;
//extern CRITICAL_SECTION cs;
int main()
{
	SOCKET clientSocketsRep2[NUMOF_THREADS_RECV];//sockets for listening to rep2
	SOCKET connectSocketRep2[NUMOF_THREADS_SENDING];//sockets for sending data to rep2
	SOCKET clientSocketsProcess[MAX_CLIENTS]; //sockets for listening to processes

	RingBuffer storingBuffer;
	RingBufferRetrieved retrievingBuffer;
	 CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	//RingBuffer storingBuffer;
	//RingBufferRetrieved retrievingBuffer;

	retrievingBuffer.head = 0;
	retrievingBuffer.tail = 0;
	retrievingBuffer.count = 0;

	storingBuffer.head = 0;
	storingBuffer.tail = 0;
	storingBuffer.count = 0;
	/*ThreadArgs threadArgs;
	threadArgs.storingBuffer = &storingBuffer;
	threadArgs.retrievingBuffer = &retrievingBuffer;*/

	InitializePorts();//dodao da bi mogli da imamo 10 portova struct{brporta , indikator je li zauzet}
	DWORD ListenForRegistrationsID;
	HANDLE hListenForRegistrations;

	//hListenForRegistrations = CreateThread(NULL, 0, &ListenForRegistrations, NULL, 0, &ListenForRegistrationsID);
	ConncectWithReplicator2(&storingBuffer, &retrievingBuffer,&cs, clientSocketsRep2, connectSocketRep2);
	ListenForRegistrations(&storingBuffer, &retrievingBuffer, &cs, clientSocketsProcess);
	_getch();

	//CloseHandle(hListenForRegistrations);

	return 0;
}
