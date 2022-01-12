#include "ReplicatorPrimHeader.h"


void ConnectWithReplicator2(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, CRITICAL_SECTION* cs2, SOCKET* clientSockets, SOCKET* connectSocket) {

	// Socket used to communicate with server
	DWORD ConnectWithReplicator2ThreadID[NUMOF_THREADS_SENDING];
	HANDLE hConnectWithReplicator2Thread[NUMOF_THREADS_SENDING];
	ThreadArgs args[NUMOF_THREADS_RECV];
	
	for (int i = 0;i < NUMOF_THREADS_SENDING;i++) {
		connectSocket[i] = INVALID_SOCKET;
	}
	int numOfConnected = 0;
	// Variable used to store function return value
	int iResult;
	// Buffer we will use to store message
	char dataBuffer[BUFFER_SIZE];
	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return;
	}
	// create a socket
	for (int i = 0;i < NUMOF_THREADS_SENDING;i++) {
		connectSocket[i] = socket(AF_INET,
			SOCK_STREAM,
			IPPROTO_TCP);
		if (connectSocket[i] == INVALID_SOCKET)
		{
			printf("socket %d failed with error: %ld\n",i, WSAGetLastError());
			WSACleanup(); 
			return;
		}
		// Create and initialize address structure
		sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;								// IPv4 protocol
		serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
		serverAddress.sin_port = htons(SERVER_PORT2);					// server port
		// Connect to server specified in serverAddress and socket connectSocket
		iResult = connect(connectSocket[i], (SOCKADDR*)&serverAddress, sizeof(serverAddress));
		if (iResult == SOCKET_ERROR)
		{
			printf("Socket %d unable to connect to server.\n",i);
			closesocket(connectSocket[i]);
			WSACleanup();
			return;
		}
		//ThreadArgs threadArgs;
		args[numOfConnected].clientSocket = connectSocket[numOfConnected];
		args[numOfConnected].storingBuffer = storingBuffer;
		args[numOfConnected].retrievingBuffer = retrievingBuffer;
		args[numOfConnected].cs = cs;
		args[numOfConnected].cs2 = cs2;

		hConnectWithReplicator2Thread[numOfConnected] = CreateThread(NULL, 0, &SendToReplicator2Thread, &args[numOfConnected], 0, &ConnectWithReplicator2ThreadID[numOfConnected]);
	
		numOfConnected++;
	}
	///////////////////////////////////////////////////////////////////////////////////
	DWORD ListenForRecvRep2ThreadID[NUMOF_THREADS_RECV];
	HANDLE hListenForRecvRep2Thread[NUMOF_THREADS_RECV];
	ThreadArgs threadArgs[NUMOF_THREADS_RECV];

	int threadNum = 0;

	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;

	// Sockets used for communication with client
	short lastIndex = 0;

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
	serverAddress.sin_port = htons(PORT_RCV_RET_DATA);	// Use specific port

	//initialise all client_socket[] to 0 so not checked
	memset(clientSockets, 0, NUMOF_THREADS_RECV * sizeof(SOCKET));

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
		return;
	}

	// set of socket descriptors
	fd_set readfds;

	// timeout for select function
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	//char* message;
	//struct process newProcess;
	while (true)
	{
		if (lastIndex  == NUMOF_THREADS_RECV)
			break;
		// initialize socket set
		FD_ZERO(&readfds);

		// add server's socket and clients' sockets to set
		if (lastIndex != NUMOF_THREADS_RECV)
		{
			FD_SET(listenSocket, &readfds);
		}

		for (int i = 0; i < lastIndex; i++)
		{
			FD_SET(clientSockets[i], &readfds);
		}

		// wait for events on set
		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			//return 0;
			return;
		}
		else if (selectResult == 0) // timeout expired
		{
			if (_kbhit()) //check if some key is pressed
			{
				getch();
				printf("Stoped\n");
			}
			continue;
		}
		else if (FD_ISSET(listenSocket, &readfds))
		{
			// Struct for information about connected client
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);

			// New connection request is received. Add new socket in array on first free position.
			clientSockets[lastIndex] = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			if (clientSockets[lastIndex] == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("accept failed, because timeout for client request has expired.\n");
				}
				else
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
				}
			}
			else
			{
				if (ioctlsocket(clientSockets[lastIndex], FIONBIO, &mode) != 0)
				{
					printf("ioctlsocket failed with error.");
					continue;
				}
				threadArgs[lastIndex].clientAddr = clientAddr;
				threadArgs[lastIndex].clientSocket = clientSockets[lastIndex];
				threadArgs[lastIndex].storingBuffer = storingBuffer;
				threadArgs[lastIndex].retrievingBuffer = retrievingBuffer;
				threadArgs[lastIndex].cs = cs;
				threadArgs[lastIndex].cs2 = cs2;

				hListenForRecvRep2Thread[threadNum] = CreateThread(NULL, 0, &ReceiveFromReplicator2Thread, &threadArgs[lastIndex], 0, &ListenForRecvRep2ThreadID[threadNum]);
				threadNum++;
				lastIndex++;
			}
		}
	}
	Sleep(100);
	printf("\n----------------------------------CONNECTIONS WITH REPLICATOR2 CREATED----------------------------------\n");

}


void SendData(RingBuffer* storingBuffer, CRITICAL_SECTION* cs, SOCKET* connectSocket) {
	message m;
	char dataBuffer[BUFFER_SIZE];
	m = ringBufGetMessage(storingBuffer, cs);
	if (m.processId == -1)
	{
		Sleep(1000);
		return;
	}

	int iResult = send(*connectSocket, (char*)&m, (short)sizeof(struct message), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error in sending to replicator2: %d\n", WSAGetLastError());
	}
	printf("Sent data to replicator2.\n");
}
DWORD WINAPI SendToReplicator2Thread(LPVOID lpParams) {
	int iResult;
	printf("Thread for sending is connected on replicator2.\n");
	SOCKET connectSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	RingBuffer* storingBuffer= (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	Sleep(3000);

	while (1)
	{
		SendData(storingBuffer, cs, &connectSocket);
	}
	// Shutdown the connection since we're done
	iResult = shutdown(connectSocket, SD_BOTH);
	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return -1;
	}
	Sleep(1000);
	// Close connected socket
	closesocket(connectSocket);
	// Deinitialize WSA library
	WSACleanup();

}


void ReceiveData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs2, SOCKET* connectSocket) {
	retrievedData m;
	char dataBuffer[BUFFER_SIZE];
	int iResult;

	iResult = recv(*connectSocket, dataBuffer, BUFFER_SIZE, 0);
	if (iResult > 0)
	{
		dataBuffer[iResult] = '\0';
		m = *(retrievedData*)(dataBuffer);
		printf("Message received from replicator2.\n");
		ringBufPutRetrievedData(retrievingBuffer, cs2, m);
	}
	else if (iResult == 0)
	{
		// connection was closed gracefully
		printf("Connection with client closed.\n");
	}
}

DWORD WINAPI ReceiveFromReplicator2Thread(LPVOID lpParams) {
	int iResult;
	retrievedData m;
	printf("Thread for receiving waiting for messages from replicator2.\n");
	SOCKET connectSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	RingBuffer* storingBuffer = (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs2 = (*(ThreadArgs*)(lpParams)).cs2;
	Sleep(3000);
	char* message;
	while (1)
	{
		ReceiveData(retrievingBuffer, cs2, &connectSocket);
	}
	// Deinitialize WSA library
	WSACleanup();
}
