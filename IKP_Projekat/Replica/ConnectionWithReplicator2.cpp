#include "Replica.h"


SOCKET clientSocketsProcess;

void ListenForReplicator2() {
	DWORD ListenForRegistrationsThreadID;
	HANDLE hListenForRegistrationsThread;
	ThreadArgs threadArgs;

	int threadNum = 0;
	//hListenForRegistrationsThread = CreateThread(NULL, 0, &ListenForRegistrationsThread, &listenSocket, 0, &ListenForRegistrationsThreadID);

	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;

	// Sockets used for communication with client

	short lastIndex = 0;

	// Variable used to store function return value
	int iResult;

	// Buffer used for storing incoming data
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		//return 1;
		return;
	}

	// Initialize serverAddress structure used by bind
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port

	//initialise all client_socket[] to 0 so not checked
	memset(&clientSocketsProcess, 0, sizeof(SOCKET));   ///provjeriti treba li adresa soketa

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address family
		SOCK_STREAM,  // Stream socket
		IPPROTO_TCP); // TCP protocol

	// Check if socket is successfully created
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		//return 1;
		return;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		//return 1;
		return;
	}

	//// All connections are by default accepted by protocol stek if socket is in listening mode.
	//// With SO_CONDITIONAL_ACCEPT parameter set to true, connections will not be accepted by default
	bool bOptVal = true;
	int bOptLen = sizeof(bool);
	iResult = setsockopt(listenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) {
		printf("setsockopt for SO_CONDITIONAL_ACCEPT failed with error: %u\n", WSAGetLastError());
	}

	unsigned long  mode = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &mode) != 0)
		printf("ioctlsocket failed with error.");

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		//return 1;
		return;
	}

	printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

	// set of socket descriptors
	fd_set readfds;

	// timeout for select function
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	//char* message;
	//struct process newProcess;
	//while (true)
	//{
	//	// initialize socket set
	//	FD_ZERO(&readfds);

	//	// add server's socket and clients' sockets to set
	//	if (lastIndex != MAX_CLIENTS)
	//	{
	//		FD_SET(listenSocket, &readfds);
	//	}

	//	for (int i = 0; i < lastIndex; i++)
	//	{
	//		FD_SET(clientSocketsProcess[i], &readfds);
	//	}

	//	// wait for events on set
	//	int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

	//	if (selectResult == SOCKET_ERROR)
	//	{
	//		printf("Select failed with error: %d\n", WSAGetLastError());
	//		closesocket(listenSocket);
	//		WSACleanup();
	//		//return 0;
	//		return;
	//	}
	//	else if (selectResult == 0) // timeout expired
	//	{
	//		if (_kbhit()) //check if some key is pressed
	//		{
	//			getch();
	//			printf("Primena racunarskih mreza u infrstrukturnim sistemima 2019/2020\n");
	//		}
	//		continue;
	//	}
	//	else if (FD_ISSET(listenSocket, &readfds))
	//	{
	//		// Struct for information about connected client
	//		sockaddr_in clientAddr;
	//		int clientAddrSize = sizeof(struct sockaddr_in);

	//		// New connection request is received. Add new socket in array on first free position.
	//		clientSocketsProcess[lastIndex] = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

	//		if (clientSocketsProcess[lastIndex] == INVALID_SOCKET)
	//		{
	//			if (WSAGetLastError() == WSAECONNRESET)
	//			{
	//				printf("accept failed, because timeout for client request has expired.\n");
	//			}
	//			else
	//			{
	//				printf("accept failed with error: %d\n", WSAGetLastError());
	//			}
	//		}
	//		else
	//		{

	//			if (ioctlsocket(clientSocketsProcess[lastIndex], FIONBIO, &mode) != 0)
	//			{
	//				printf("ioctlsocket failed with error.");
	//				continue;
	//			}
	//			threadArgs.clientAddr = clientAddr; //mpoguca greska da se prepise nove=a preko stare strukture
	//			threadArgs.clientSocket = lastIndex;
	//			//threadArgs.storingBuffer = storingBuffer;
	//			//threadArgs.retrievingBuffer = retrievingBuffer;
	//			threadArgs.cs = &cs;
	//			char args[8];
	//			//memset(args, (int)&cc, 4);
	//			//memset(args+4, (int)storingBuffer, 4);

	//			hListenForRegistrationsThread[threadNum] = CreateThread(NULL, 0, &ListenForRegistrationsThread, &threadArgs[lastIndex], 0, &ListenForRegistrationsThreadID[threadNum]);
	//			threadNum++;
	//			lastIndex++;
	//		}
	//	}
	//}
	//for (int i = 0;i < threadNum;i++)
	//	CloseHandle(hListenForRegistrationsThread[i]);
	////Close listen and accepted sockets
	//closesocket(listenSocket);

	//// Deinitialize WSA library
	//WSACleanup();
}

DWORD WINAPI ListenForReplicator2Thread(LPVOID lpParams) {

	return 0;
}