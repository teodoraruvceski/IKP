#include "ReplicatorSecHeader.h"

void ListenForReplica(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, SOCKET* clientSocketsReplica)
{

	DWORD ListenForReplicaThreadID[MAX_CLIENTS];
	HANDLE hListenForReplicaThread[MAX_CLIENTS];
	ThreadArgs threadArgs[MAX_CLIENTS];

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
	serverAddress.sin_port = htons(REPLICA_LISTEN_PORT);	// Use specific port

	//initialise all client_socket[] to 0 so not checked
	memset(clientSocketsReplica, 0, MAX_CLIENTS * sizeof(SOCKET));

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

	printf("Replicator2 socket is set to listening mode. Waiting for new Replica connection requests.\n");

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
		if (lastIndex != MAX_CLIENTS)
		{
			FD_SET(listenSocket, &readfds);
		}

		for (int i = 0; i < lastIndex; i++)
		{
			FD_SET(clientSocketsReplica[i], &readfds);
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
			//printf("Timeout expired\n");
			continue;
		}
		else if (FD_ISSET(listenSocket, &readfds))
		{
			// Struct for information about connected client
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);

			// New connection request is received. Add new socket in array on first free position.
			clientSocketsReplica[lastIndex] = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			if (clientSocketsReplica[lastIndex] == INVALID_SOCKET)
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

				if (ioctlsocket(clientSocketsReplica[lastIndex], FIONBIO, &mode) != 0)
				{
					printf("ioctlsocket failed with error.");
					continue;
				}
				printf("Accepted replica connection!\n");
				threadArgs[lastIndex].clientAddr = clientAddr; //mpoguca greska da se prepise nove=a preko stare strukture
				threadArgs[lastIndex].clientSocket = clientSocketsReplica[lastIndex];
				threadArgs[lastIndex].storingBuffer = storingBuffer;
				threadArgs[lastIndex].retrievingBuffer = retrievingBuffer;
				threadArgs[lastIndex].cs = cs;
				char args[8];


				hListenForReplicaThread[threadNum] = CreateThread(NULL, 0, &SendToReplica, &threadArgs[lastIndex], 0, &ListenForReplicaThreadID[threadNum]);
				threadNum++;
				lastIndex++;
			}
		}
	}
	/*for (int i = 0;i < threadNum;i++)
		CloseHandle(hListenForRegistrationsThread[i]);
	closesocket(listenSocket);

	WSACleanup();*/
}

DWORD WINAPI SendToReplica(LPVOID lpParams)
{
	SOCKET clientSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	sockaddr_in clientAddr = (*(ThreadArgs*)(lpParams)).clientAddr;
	RingBuffer* storingBuffer = (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	// Sockets used for communication with client
	short pId=-1; //tu cuvamo id replice koja se javila
	short processId;
	char* newAddr = inet_ntoa(clientAddr.sin_addr);
	printf("New client request accepted . Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	message m;
	message* pointer;
	retrievedData data;
	char dataBuffer[BUFFER_SIZE];
	int iResult;
	bool flag = false;
	
	while (1)
	{
		if (flag == false) {
			printf("Waiting for replica to say Hi...\n");
			iResult = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);
			if (iResult > 0)
			{
				dataBuffer[iResult] = '\0';
				pointer = (message*)dataBuffer;
				pId = ntohs(pointer->processId);
				printf("Replica %d saying hello.\n",pId);
				flag = true;
			}
			else if (iResult == 0)
			{
				//Sleep(10);
			}
			else
			{
				//there was an error during recv
				printf("recv failed with error: %d RECIVING MESSAGES\n", WSAGetLastError());
				Sleep(3000);
				//closesocket(clientSocket);
			}
		}
		else {
			m = ringBufReadMessage(storingBuffer, cs);
			if (m.processId == -1 || m.processId != pId)
			{
				Sleep(1000);
				continue;
			}
			m = ringBufGetMessage(storingBuffer, cs);
			if (strcmp(m.text, "get_data_from_replica") == 0) {
				iResult = send(clientSocket, (char*)&m, (short)sizeof(struct message), 0);
				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(clientSocket);
					WSACleanup();
					return -1;
				}
				printf("Message with data: %s for replica successfully sent. Total bytes: %ld\n", m.text, iResult);
				//cekanje povratnih podataka
				iResult = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);
				if (iResult > 0)
				{
					dataBuffer[iResult] = '\0';
					data = *(retrievedData*)dataBuffer;//provjeriti povratni tip podataka nisam podesavao
					ringBufPutRetrievedData(retrievingBuffer,cs,data);
					printf("Replica %d retrieved data for process.\n", pId);
				}
				else if (iResult == 0)
				{
					Sleep(10);
				}
				else
				{
					//there was an error during recv
					printf("recv failed with error: %d RECIVING MESSAGES\n", WSAGetLastError());
					closesocket(clientSocket);
				}
			}
			else {
				iResult = send(clientSocket, (char*)&m, (short)sizeof(struct message), 0);
				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(clientSocket);
					WSACleanup();
					return -1;
				}
				printf("Message with data: %s for replica successfully sent. Total bytes: %ld\n", m.text, iResult);
			}
		}
	}
	// Deinitialize WSA library
	WSACleanup();

}