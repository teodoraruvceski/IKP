#include "ReplicatorSecHeader.h"

void ConnectWithReplicator1(RingBuffer* storingBuffer,RingBufferRetrieved * retrievingBuffer,CRITICAL_SECTION* cs, CRITICAL_SECTION* cs2, ThreadArgs* threadArgs2, int replics[],
	DWORD ListenForReplicator1ThreadID[NUMOF_THREADS], HANDLE hListenForReplicator1Thread[NUMOF_THREADS],DWORD SendToReplicator1ThreadID[NUMOF_THREADS_SENDING],
HANDLE hSendToReplicator1Thread[NUMOF_THREADS_SENDING], bool* end)
{
	
	ThreadArgs threadArgs[NUMOF_THREADS];

	int threadNum = 0;
	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;
	// Sockets used for communication with client
	SOCKET clientSockets[NUMOF_THREADS];
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
		return;
	}

	// Initialize serverAddress structure used by bind
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port

	//initialise all client_socket[] to 0 so not checked
	memset(clientSockets, 0, NUMOF_THREADS * sizeof(SOCKET));

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
		// initialize socket set
		FD_ZERO(&readfds);

		// add server's socket and clients' sockets to set
		if (lastIndex != NUMOF_THREADS)
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
				threadArgs[lastIndex].clientSocket = clientSockets[lastIndex];
				threadArgs[lastIndex].storingBuffer = storingBuffer;
				threadArgs[lastIndex].retrievingBuffer = retrievingBuffer;
				threadArgs[lastIndex].cs = cs;
				threadArgs[lastIndex].cs2 = cs2;
				threadArgs[lastIndex].replics = replics;
				threadArgs[lastIndex].end = end;
				hListenForReplicator1Thread[threadNum] = CreateThread(NULL, 0, &ListenForReplicator1Thread, &threadArgs[lastIndex], 0, &ListenForReplicator1ThreadID[threadNum]);
				threadNum++;
				lastIndex++;
				if (threadNum == NUMOF_THREADS)
					break;
			}
		}
	}
	/// ////////////////////////////////////////////////////////////////////////////////////////
	
	SOCKET connectSocket[NUMOF_THREADS_SENDING];
	
	for (int i = 0;i < NUMOF_THREADS_SENDING;i++) {
		connectSocket[i] = INVALID_SOCKET;
	}
	int numOfConnected = 0;
	
	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData2;
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
			printf("socket %d failed with error: %ld\n", i, WSAGetLastError());
			WSACleanup();
			return;
		}
		// Create and initialize address structure
		sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;								// IPv4 protocol
		serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
		serverAddress.sin_port = htons(SERVER_PORT_REP1);					// server port
		// Connect to server specified in serverAddress and socket connectSocket
		iResult = connect(connectSocket[i], (SOCKADDR*)&serverAddress, sizeof(serverAddress));
		if (iResult == SOCKET_ERROR)
		{
			printf("Socket %d unable to connect to server.\n", i);
			closesocket(connectSocket[i]);
			WSACleanup();
			return;
		}
		threadArgs2[numOfConnected].clientSocket = connectSocket[numOfConnected];
		threadArgs2[numOfConnected].storingBuffer = storingBuffer;
		threadArgs2[numOfConnected].retrievingBuffer = retrievingBuffer;
		threadArgs2[numOfConnected].cs = cs;
		threadArgs2[numOfConnected].cs2 = cs2;
		threadArgs2[numOfConnected].end = end;
		hSendToReplicator1Thread[numOfConnected] = CreateThread(NULL, 0, &SendToReplicator1Thread, &threadArgs2[numOfConnected], 0, &SendToReplicator1ThreadID[numOfConnected]);

		numOfConnected++;
	}
	Sleep(100);
	printf("\n----------------------------------CONNECTIONS WITH REPLICATOR1 CREATED----------------------------------\n");
}

void ReceiveData(RingBuffer* storingBuffer, CRITICAL_SECTION* cs, SOCKET* clientSocket,int* replics) {
	char dataBuffer[BUFFER_SIZE];
	char id[4];

	int iResult = recv(*clientSocket, dataBuffer, BUFFER_SIZE, 0);
	if (iResult > 0)
	{
		dataBuffer[iResult] = '\0';
		struct message* message2 = (struct message*)dataBuffer;
		if (strcmp(message2->text, "REGISTRATION") == 0) {
			itoa((int)(message2->processId), id, 10);
			printf("Recieved REGISTRATION message from replicator1.\n");
			if (TryAddReplica(replics, message2->processId) == true) {
				SHELLEXECUTEINFO ExecuteInfo;
				LPCWSTR mode = L"open";
				LPCWSTR params = (LPCWSTR)id;
				memset(&ExecuteInfo, 0, sizeof(ExecuteInfo));
				ExecuteInfo.cbSize = sizeof(ExecuteInfo);
				ExecuteInfo.fMask = 0;
				ExecuteInfo.hwnd = 0;
				ExecuteInfo.lpVerb = mode;
				ExecuteInfo.lpFile = L"D:\\tea\\Fax\\4.godina\\1.semestar\\ikp\\IKP\\IKP_Projekat\\x64\\Debug\\Replica.exe";
				//ExecuteInfo.lpFile = L"C:\\Users\\Nebojsa\\Desktop\\IKP\\IKP_Projekat\\x64\\Debug\\Replica.exe";
				ExecuteInfo.lpParameters = params;
				ExecuteInfo.lpDirectory = 0;
				ExecuteInfo.nShow = SW_SHOWNORMAL;
				ExecuteInfo.hInstApp = 0;
				ShellExecuteEx(&ExecuteInfo);
			}
			else {
				printf("Replica %d already exist.\n", message2->processId);
			}
		}
		else {
			printf("Recieved message from replicator1, and stored.\n");
			ringBufPutMessage(storingBuffer, cs, *message2);
		}
	}
}

DWORD WINAPI ListenForReplicator1Thread(LPVOID lpParams)
{
	int iResult;
	printf("Thread for receiving waiting for messages from replicator1.\n");
	SOCKET clientSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	sockaddr_in clientAddr = (*(ThreadArgs*)(lpParams)).clientAddr;
	RingBuffer* storingBuffer = (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	int* replics = (*(ThreadArgs*)(lpParams)).replics;
	bool* end= (*(ThreadArgs*)(lpParams)).end;
	
	char* message;
	struct process newProcess;
	
	while (!(*end))
	{
		ReceiveData(storingBuffer,cs,&clientSocket,replics);
	}
	// Deinitialize WSA library
	WSACleanup();
	printf("NIT ZA PRIJEM GOTOVA\n");
	return 0;
}

void SendData(RingBufferRetrieved* retrievingBuffer, SOCKET* connectSocket, CRITICAL_SECTION* cs2) {
	retrievedData m;
	char dataBuffer[BUFFER_SIZE];
	m = ringBufGetRetrievedData(retrievingBuffer, cs2);
	if (m.processId == -1)
	{
		Sleep(1000);
		return;
	}
	int iResult = send(*connectSocket, (char*)&m, (short)sizeof(retrievedData), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
	}
	printf("Sent data to replicator1.\n");
}

DWORD WINAPI SendToReplicator1Thread(LPVOID lpParams) {
	int iResult;
	printf("Thread for sending is connected on replicator1.\n");
	SOCKET connectSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	RingBuffer* storingBuffer = ((ThreadArgs*)(lpParams))->storingBuffer;
	RingBufferRetrieved* retrievingBuffer = ((ThreadArgs*)(lpParams))->retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	CRITICAL_SECTION* cs2 = (*(ThreadArgs*)(lpParams)).cs;
	bool *end= (*(ThreadArgs*)(lpParams)).end;

	Sleep(10000);
	//_getch();
	while (!(*end))
	{
		printf("end:%d\n", (*end));
		SendData(retrievingBuffer,&connectSocket,cs2);
	}
	printf("NIT ZA SLANJE GOTOVA\n");
	// Shutdown the connection since we're done
	iResult = shutdown(connectSocket, SD_BOTH);
	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		/*printf("Nit za Prijem:Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return -1;*/
	}
	Sleep(1000);
	// Close connected socket
	closesocket(connectSocket);
	// Deinitialize WSA library
	WSACleanup();
	printf("NIT ZA SLANJE GOTOVA\n");
	return 0;
}