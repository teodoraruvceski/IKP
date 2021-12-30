#include "ReplicatorPrimHeader.h"



//extern RingBuffer* storingBuffer;
//extern RingBufferRetrieved* retrievingBuffer;
//extern CRITICAL_SECTION cs;


void ConncectWithReplicator2(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, SOCKET* clientSockets, SOCKET* connectSocket) {

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
		hConnectWithReplicator2Thread[numOfConnected] = CreateThread(NULL, 0, &SendToReplicator2Thread, &args[numOfConnected], 0, &ConnectWithReplicator2ThreadID[numOfConnected]);
		
		numOfConnected++;
	}
	///////////////////////////////////////////////////////////////////////////////////
	//SOCKET serverSocket_recv[NUMOF_THREADS_RECV];

	//printf("Ocekivanje komenkcije...\n");
	DWORD ListenForRecvRep2ThreadID[NUMOF_THREADS_RECV];
	HANDLE hListenForRecvRep2Thread[NUMOF_THREADS_RECV];
	ThreadArgs threadArgs[NUMOF_THREADS_RECV];

	int threadNum = 0;
	//hListenForRegistrationsThread = CreateThread(NULL, 0, &ListenForRegistrationsThread, &listenSocket, 0, &ListenForRegistrationsThreadID);

	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;

	// Sockets used for communication with client
	short lastIndex = 0;

	// Variable used to store function return value
    iResult;

	// Buffer used for storing incoming data
	//dataBuffer[BUFFER_SIZE];

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
	serverAddress.sin_port = htons(SERVER_PORT2_2);	// Use specific port

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
	//printf("\nBajnding...\n");
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
				_getch();
				printf("Primena racunarskih mreza u infrstrukturnim sistemima 2019/2020\n");
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
				threadArgs[lastIndex].clientAddr = clientAddr; //mpoguca greska da se prepise nove=a preko stare strukture
				threadArgs[lastIndex].clientSocket = clientSockets[lastIndex];
				threadArgs[lastIndex].storingBuffer = storingBuffer;
				threadArgs[lastIndex].retrievingBuffer = retrievingBuffer;
				threadArgs[lastIndex].cs = cs;
				//char args[8];

				hListenForRecvRep2Thread[threadNum] = CreateThread(NULL, 0, &ReccvFromReplicator2Thread, &threadArgs[lastIndex], 0, &ListenForRecvRep2ThreadID[threadNum]);
				threadNum++;
				lastIndex++;
			}
		}
	}
	//for (int i = 0;i < threadNum;i++)
	//	CloseHandle(hListenForRecvRep2Thread[i]);
	////Close listen and accepted sockets
	//closesocket(listenSocket);

	// Deinitialize WSA library
	//WSACleanup();
}


DWORD WINAPI SendToReplicator2Thread(LPVOID lpParams) {
	int iResult;
	message m;
	SOCKET connectSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	printf("Nit konektovana na replicator2. \n");

	//int socket = (*(ThreadArgs*)(lpParams)).clientSocket;
	RingBuffer* storingBuffer= (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	Sleep(3000);
	
	while (1)
	{
		char dataBuffer[BUFFER_SIZE];
		//printBuffer(storingBuffer,cs);
		m = ringBufGetMessage(storingBuffer,cs);//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<OVDJE I DOLJE DA MI GRESKU NE JAVLJA
		//m.processId = -1;//<<<<<<<<<<<<<<
		if (m.processId == -1)
		{
			Sleep(3000);
			continue;
		}
		printf("Checking for messages to send...\n");
		iResult = send(connectSocket, (char*)&m, (short)sizeof(struct message), 0);
		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error in sending to REP2: %d\n", WSAGetLastError());
			//closesocket(connectSocket);
			//WSACleanup();
			//break;
		}
		printf("Sent data to replicator2.");
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
DWORD WINAPI ReccvFromReplicator2Thread(LPVOID lpParams) {
	int iResult;
	retrievedData m;
	//SOCKET clientSocket = *(*(ThreadArgs*)(lpParams)).clientSocket;
	SOCKET socket= (*(ThreadArgs*)(lpParams)).clientSocket;
	RingBuffer* storingBuffer = (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	Sleep(3000);
	char* message;
	while (1)
	{
		char dataBuffer[BUFFER_SIZE];
		int iResult;
		
		
		iResult = recv(socket, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0)
		{
			dataBuffer[iResult] = '\0';
			m = *(retrievedData*)(dataBuffer);
			printf("Message received from client.\n");
			ringBufPutRetrievedData(retrievingBuffer,cs,m); //zakomentarisao sam buffer da bi mogo pokrenuti niti
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			//closesocket(clientSocket);
		}
		else
		{
			//there was an error during recv
			//printf("recv failed with error: %d RECIVING MESSAGES\n", WSAGetLastError());
			//closesocket(clientSocket);
		}
		//Close listen and accepted sockets
		//closesocket(listenSocket);
	}
	// Deinitialize WSA library
	WSACleanup();

}
