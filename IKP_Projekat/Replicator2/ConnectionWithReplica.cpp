#include "ReplicatorSecHeader.h"

//this function waiting new replica connections, adn starting new thread for each connection
void ListenForReplica(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, CRITICAL_SECTION* cs2, SOCKET* clientSocketsReplica,
	DWORD ListenForReplicaThreadID[MAX_CLIENTS],HANDLE hListenForReplicaThread[MAX_CLIENTS], bool* end)
{
	ThreadArgs threadArgs[MAX_CLIENTS];

	int threadNum = 0;

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
		if (_kbhit())
		{
			printf("Press any key to end thread...\n");
			(*end) = true;
			_getch();
			break;
		}
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
				threadArgs[lastIndex].clientAddr = clientAddr;
				threadArgs[lastIndex].clientSocket = clientSocketsReplica[lastIndex];
				threadArgs[lastIndex].storingBuffer = storingBuffer;
				threadArgs[lastIndex].retrievingBuffer = retrievingBuffer;
				threadArgs[lastIndex].cs = cs;
				threadArgs[lastIndex].cs2 = cs2;
				threadArgs[lastIndex].end = end;

				char args[8];

				hListenForReplicaThread[threadNum] = CreateThread(NULL, 0, &ConnectionWithReplicaThread, &threadArgs[lastIndex], 0, &ListenForReplicaThreadID[threadNum]);
				threadNum++;
				lastIndex++;
			}
		}
	}
	for (int i = 0; i < MAX_CLIENTS; i++) {
		try {
			closesocket(clientSocketsReplica[i]); //	pitanje : da li treba da se zatvaraju soketi od replica 
		}
		catch (...) {}
		
	}
	closesocket(listenSocket);
	WSACleanup();
}

//this functions accept first message from replica
void RegisterReplica(SOCKET* clientSocket, bool* flag, short* processId) {
	char dataBuffer[BUFFER_SIZE];
	message* pointer;

	int iResult = recv(*clientSocket, dataBuffer, BUFFER_SIZE, 0);
	if (iResult > 0)
	{
		dataBuffer[iResult] = '\0';
		pointer = (message*)dataBuffer;
		*processId = ntohs(pointer->processId);
		printf("Replica with Id: %d sent registration message.\n", *processId);
		*flag = true;
	}
	else if (iResult == 0)
	{
		Sleep(1000);
	}
	else
	{
		//there was an error during recv
		printf("recv failed with error: %d RECIVING MESSAGES\n", WSAGetLastError());
		Sleep(3000);
	}
}

//this function sending request for retreiving data and waiting response from replica
void MessageForRetreivingData(SOCKET * clientSocket,struct message* mess, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs2, short* processId) {
	char dataBuffer[BUFFER_SIZE];
	retrievedData data;

	int iResult = send(*clientSocket, (char*)mess, (short)sizeof(struct message), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(*clientSocket);
		WSACleanup();
		return;
	}
	printf("Message for retreiving data successfully sent to replica. Total bytes: %ld\n", mess->text, iResult);
	while (1)
	{
		iResult = recv(*clientSocket, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0)
		{
			dataBuffer[iResult] = '\0';
			data = *(retrievedData*)dataBuffer;
			printf("Replica sent : id=%d,text=%s\n", data.processId, data.data);
			ringBufPutRetrievedData(retrievingBuffer, cs2, data);
			printf("Replica %d retrieved data for process.\n", *processId);
			break;
		}
		else if (iResult == 0)
		{
			Sleep(1000);
		}
		else
		{
			//there was an error during recv
			printf("recv failed with error: %d RECIVING MESSAGES\n", WSAGetLastError());
			Sleep(1000);
		}
	}
}

//this function sending messages for storing
void MessageForStoring(SOCKET* clientSocket,struct message *mess) {
	int iResult = send(*clientSocket, (char*)mess, (short)sizeof(struct message), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(*clientSocket);
		WSACleanup();
		return;
	}
	printf("Message with data: %s for replica successfully sent. Total bytes: %ld\n", mess->text, iResult);
}

//this function trying to get data for replica and calling other functions
DWORD WINAPI ConnectionWithReplicaThread(LPVOID lpParams)
{
	SOCKET clientSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	sockaddr_in clientAddr = (*(ThreadArgs*)(lpParams)).clientAddr;
	RingBuffer* storingBuffer = (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	CRITICAL_SECTION* cs2 = (*(ThreadArgs*)(lpParams)).cs2;
	bool *end= (*(ThreadArgs*)(lpParams)).end;

	short processId=-1;//var for replica Id
	char* newAddr = inet_ntoa(clientAddr.sin_addr);
	printf("New replica request accepted  . Replica address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	message mess;
	retrievedData *data;
	char dataBuffer[BUFFER_SIZE];
	int iResult;
	bool flag = false;
	
	while (!(*end))
	{
		if (flag == false) {
			RegisterReplica(&clientSocket, &flag, &processId);
		}
		else {
			//checking is something in buffer has id of thread
			mess = ringBufReadMessage(storingBuffer, cs);
			if (mess.processId == -1 || mess.processId != processId)
			{
				Sleep(1000);
				continue;
			}
			mess = ringBufGetMessage(storingBuffer, cs);
			if (strcmp(mess.text, "get_data_from_replica") == 0) {
				//MessageForRetreivingData(&clientSocket, &mess, retrievingBuffer, cs2, &processId);	
				////////////////////////////////////////////////////////////////////////////////////
				int iResult = send(clientSocket, (char*)&mess, (short)sizeof(struct message), 0);
				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					//printf("send failed with error: %d\n", WSAGetLastError());
					//closesocket(clientSocket);
					//WSACleanup();
					//return;
				}
				printf("Message for retreiving data successfully sent to replica. Total bytes: %ld\n", mess.text, iResult);
				while (1)
				{
					iResult = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);
					if (iResult > 0)
					{
						dataBuffer[iResult] = '\0';
						data = (retrievedData*)dataBuffer;
						printf("Replica sent : id=%d,text=%s\n", data->processId, data->data);
						ringBufPutRetrievedData(retrievingBuffer, cs2, *data);
						printf("Replica %d retrieved data for process.\n", data->processId);
						break;
					}
					else if (iResult == 0)
					{
						Sleep(1000);
					}
					else
					{
						//there was an error during recv
						printf("recv failed with error: %d RECIVING MESSAGES\n", WSAGetLastError());
						Sleep(1000);
					}
				}
			}
			else {
				MessageForStoring(&clientSocket,&mess);
			}
		}
	}
	// Deinitialize WSA library
	WSACleanup();
}