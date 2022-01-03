#include "ReplicatorSecHeader.h"

//DWORD WINAPI ListenForRegistrations(LPVOID lpParams)

void ListenForReplicator1Registrations(RingBuffer* storingBuffer,RingBufferRetrieved * retrievingBuffer,CRITICAL_SECTION* cs, ThreadArgs* threadArgs2)
{
	//clientConnection clientConnections[NUMOF_THREADS];
	DWORD ListenForReplicator1ThreadID[NUMOF_THREADS];
	HANDLE hListenForReplicator1Thread[NUMOF_THREADS];
	
	ThreadArgs threadArgs[NUMOF_THREADS];

	int threadNum = 0;
	//hListenForRegistrationsThread = CreateThread(NULL, 0, &ListenForRegistrationsThread, &listenSocket, 0, &ListenForRegistrationsThreadID);
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
				threadArgs[lastIndex].clientSocket = clientSockets[lastIndex];
				threadArgs[lastIndex].storingBuffer = storingBuffer;
				threadArgs[lastIndex].retrievingBuffer = retrievingBuffer;
				threadArgs[lastIndex].cs = cs;
				hListenForReplicator1Thread[threadNum] = CreateThread(NULL, 0, &ListenForReplicator1Thread, &threadArgs[lastIndex], 0, &ListenForReplicator1ThreadID[threadNum]);
				threadNum++;
				lastIndex++;
				if (threadNum == NUMOF_THREADS)
					break;
			}
		}
	}
	/// ////////////////////////////////////////////////////////////////////////////////////////
	DWORD SendToReplicator1ThreadID[NUMOF_THREADS_SENDING];
	HANDLE hSendToReplicator1Thread[NUMOF_THREADS_SENDING];
	SOCKET connectSocket[NUMOF_THREADS_SENDING];
	
	printf("Konektovanje na rep1 \n");
	for (int i = 0;i < NUMOF_THREADS_SENDING;i++) {
		connectSocket[i] = INVALID_SOCKET;
	}
	int numOfConnected = 0;
	// Variable used to store function return value
	iResult;
	// Buffer we will use to store message
	///////////////////////////////////////////////////////////////////////////////////char dataBuffer[BUFFER_SIZE];
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
		hSendToReplicator1Thread[numOfConnected] = CreateThread(NULL, 0, &SendToReplicator1Thread, &threadArgs2[numOfConnected], 0, &SendToReplicator1ThreadID[numOfConnected]);

		numOfConnected++;
	}
	//for (int i = 0;i < threadNum;i++)
	//	CloseHandle(hListenForReplicator1Thread[i]);
	////Close listen and accepted sockets
	//closesocket(listenSocket);

	//// Deinitialize WSA library
	//WSACleanup();
	Sleep(5000);
	printf("Pokrenute sve niti za komunikaciju sa replikatorom 1.\n Im out!\n");
}

DWORD WINAPI ListenForReplicator1Thread(LPVOID lpParams)
{
	printf("Nit kreirana kao server, slusa poruke od rep1.\n");
	SOCKET clientSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	sockaddr_in clientAddr = (*(ThreadArgs*)(lpParams)).clientAddr;
	RingBuffer* storingBuffer = (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	// Sockets used for communication with client
	
	char* message;
	struct process newProcess;
	
	while (1)
	{
		char dataBuffer[BUFFER_SIZE];
		int iResult;
		char verb[7] = "runas\0";
		char location[53] = "D:\tea\Fax\4.godina\1.semestar\ikp\IKP\IKP_Projekat\Replica\0";
		char id[4];
		
		iResult = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0)
		{			
			dataBuffer[iResult] = '\0';
			printf("Message received from client\n");
			struct message* message2 = (struct message*)dataBuffer;
			if (strcmp(message2->text, "REGISTRATION") == 0) {
				itoa((int)(message2->processId), id, 10);
				printf("Recieved REGISTRATION message from REP1.\n");
				//code for creating NEW INSTANCE <3
				/*ShellExecuteA(
					GetDesktopWindow(),
					"open",
					"D:\\tea\\Fax\\4.godina\\1.semestar\\ikp\\IKP\\IKP_Projekat\\x64\\Debug\\Replica.exe",
					id,
					"D:\\tea\\Fax\\4.godina\\1.semestar\\ikp\\IKP\\IKP_Projekat\\x64\\Debug",
					SW_SHOWDEFAULT
				);*/

				/*ShellExecuteA(
					NULL,
					NULL,
					"D:\\tea\\Fax\\4.godina\\1.semestar\\ikp\\IKP\\IKP_Projekat\\x64\\Debug\\Replica.exe",
					id,
					"D:\\tea\\Fax\\4.godina\\1.semestar\\ikp\\IKP\\IKP_Projekat",
					SW_SHOWNORMAL
				);*/

				/*char* my_args[3];
				char name[] = "D:\\tea\\Fax\\4.godina\\1.semestar\\ikp\\IKP\\IKP_Projekat\\x64\\Debug\\Replica.exe";
				my_args[0] = name;
				my_args[1] = id;
				my_args[2] = NULL;
				spawnl(P_WAIT, "Replica.exe", name,(char*)(message2->processId),NULL);*/

				/*char child2[50];
				strcpy(child2, "Replica.exe ");
				strcat(child2, id);
				system(child2);*/

				//PVOID OldValue = nullptr;
				//Wow64DisableWow64FsRedirection(&OldValue);
				//ShellExecute(NULL, L"open",L"D:\\tea\\Fax\\4.godina\\1.semestar\\ikp\\IKP\\IKP_Projekat\\x64\\Debug\\Replica.exe",NULL, id, SW_RESTORE);

				SHELLEXECUTEINFO ExecuteInfo;
				LPCWSTR mode = L"open";
				LPCWSTR params = (LPCWSTR)id;
				memset(&ExecuteInfo, 0, sizeof(ExecuteInfo));

				ExecuteInfo.cbSize = sizeof(ExecuteInfo);
				ExecuteInfo.fMask = 0;
				ExecuteInfo.hwnd = 0;
				ExecuteInfo.lpVerb = mode;                      // Operation to perform
				ExecuteInfo.lpFile = L"D:\\tea\\Fax\\4.godina\\1.semestar\\ikp\\IKP\\IKP_Projekat\\x64\\Debug\\Replica.exe"; //"c:\\windows\\notepad.exe";  // Application name
				ExecuteInfo.lpParameters = params;           // Additional parameters
				ExecuteInfo.lpDirectory = 0;                           // Default directory
				ExecuteInfo.nShow = SW_SHOWNORMAL;
				ExecuteInfo.hInstApp = 0;

				ShellExecuteEx(&ExecuteInfo);
			}
			else {
				printf("Recieved message from REP1, and stored in storingBuffer.\n");
				ringBufPutMessage(storingBuffer, cs,*message2);
			}
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
	return 0;
}
DWORD WINAPI SendToReplicator1Thread(LPVOID lpParams) {
	int iResult;
	retrievedData m;
	printf("Nit konektovana na replicator1 kao klijent.\n");
	SOCKET connectSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	RingBuffer* storingBuffer = ((ThreadArgs*)(lpParams))->storingBuffer;
	RingBufferRetrieved* retrievingBuffer = ((ThreadArgs*)(lpParams))->retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	Sleep(10000);
	//_getch();
	while (1)
	{
		char dataBuffer[BUFFER_SIZE];
		m = ringBufGetRetrievedData(retrievingBuffer, cs);
		if (m.processId == -1)
		{
			Sleep(3000);
			continue;
		}
		
		printf("Checking for messages to send...\n");
		iResult = send(connectSocket, (char*)&m, (short)sizeof(retrievedData), 0);
		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			//closesocket(connectSocket);
			//WSACleanup();
			break;
		}
		printf("Sent data to replicator1.\n");
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