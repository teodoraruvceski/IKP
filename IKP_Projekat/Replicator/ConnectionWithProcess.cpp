#include "ReplicatorPrimHeader.h"

int pBr = -1;
bool ind = false;

void ListenForRegistrations(RingBuffer* storingBuffer,RingBufferRetrieved* retrievingBuffer,CRITICAL_SECTION *cs ,
	CRITICAL_SECTION* cs2,SOCKET* clientSocketsProcess, DWORD ListenForRegistrationsThreadID[MAX_CLIENTS],
HANDLE hListenForRegistrationsThread[MAX_CLIENTS], bool* end)
{
	
	ThreadArgs threadArgs[MAX_CLIENTS];
	int ids[MAX_CLIENTS];
	for (int i = 0;i < MAX_CLIENTS;i++)
		ids[i] = -1;

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
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port

	//initialise all client_socket[] to 0 so not checked
	memset(clientSocketsProcess, 0, MAX_CLIENTS * sizeof(SOCKET));

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

	printf("Replicator1 socket is set to listening mode. Waiting for new Process connection requests.\n");

	// set of socket descriptors
	fd_set readfds;

	// timeout for select function
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	//char* message;
	//struct process newProcess;
	ind = false;
	while (true)
	{
		//if key is hit return from function
		if (_kbhit())
		{
			_getch();
			(*end) = true;
			break;
		}
		// initialize socket set
		FD_ZERO(&readfds);

		// add server's socket and clients' sockets to set
		if (threadNum != MAX_CLIENTS)
		{
			FD_SET(listenSocket, &readfds);
		}

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if(ids[i]==0)
				FD_SET(clientSocketsProcess[i], &readfds);
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
			continue;
		}
		else if (FD_ISSET(listenSocket, &readfds))
		{
			// Struct for information about connected client
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);
			for (int i = 0; i < MAX_CLIENTS;i++) {
				if (ids[i] == -1) {
					lastIndex = i;
					ids[i] = 0;
					break;
				}
			}
			// New connection request is received. Add new socket in array on first free position.
			clientSocketsProcess[lastIndex] = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			if (clientSocketsProcess[lastIndex] == INVALID_SOCKET)
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

				if (ioctlsocket(clientSocketsProcess[lastIndex], FIONBIO, &mode) != 0)
				{
					printf("ioctlsocket failed with error.\n");
					continue;
				}
				
				threadArgs[lastIndex].clientAddr= clientAddr; //mpoguca greska da se prepise nove=a preko stare strukture
				threadArgs[lastIndex].clientSocket =clientSocketsProcess[lastIndex];
				threadArgs[lastIndex].storingBuffer = storingBuffer;
				threadArgs[lastIndex].retrievingBuffer = retrievingBuffer;
				threadArgs[lastIndex].cs = cs;
				threadArgs[lastIndex].cs2 = cs2;
				threadArgs[lastIndex].id = lastIndex;
				threadArgs[lastIndex].end = end;

				hListenForRegistrationsThread[lastIndex] = CreateThread(NULL, 0, &ListenForRegistrationsThread, &threadArgs[lastIndex], 0, &ListenForRegistrationsThreadID[lastIndex]);
				threadNum++;
			}
		}
		else {
			Sleep(5);
			if (ind) {
				closesocket(clientSocketsProcess[pBr]);
				Sleep(100);

				ind = false;
				printf("Closing thread with connection %d \n",pBr);

				// sort array and clean last place
				CloseHandle(hListenForRegistrationsThread[pBr]);
				clientSocketsProcess[pBr] = 0;
				ids[pBr] = -1;
				pBr = -1;
				threadNum--;
			}
		}
	}
	for (int i = 0;i < MAX_CLIENTS;i++) {
		if (ids[i] == 0) {
			closesocket(clientSocketsProcess[i]);
		}
	}
	closesocket(listenSocket);
	WSACleanup();
}

void RegisterProcess(SOCKET* clientSocket,bool* flag,short* processId,struct process* newProcess, struct message* newMessage, RingBuffer* storingBuffer, CRITICAL_SECTION* cs) {
	char dataBuffer[BUFFER_SIZE];
	char* message_;
	int iResult = recv(*clientSocket, dataBuffer, BUFFER_SIZE, 0);

	if (iResult > 0)
	{
		dataBuffer[iResult] = '\0';
		message_ = dataBuffer;
		
		*processId = *((short*)(message_));
		*processId = ntohs(*processId);
		newProcess->id = *processId;
		newMessage->processId = newProcess->id;
		strcpy(newMessage->text, "REGISTRATION");
		ringBufPutMessage(storingBuffer, cs, *newMessage);

		printf("Process with Id: %d sent registration message.\n", newProcess->id);
		*flag = true;
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(*clientSocket);
			WSACleanup();
			return;
		}
	}
}

void MessageForStoring(RingBuffer* storingBuffer, CRITICAL_SECTION* cs,struct message* newMessage) {
	printf("Storing message %s from process: %d to buffer.\n", newMessage->text, newMessage->processId);
	ringBufPutMessage(storingBuffer, cs, *newMessage);
}

void MessageForRetreivingData(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, CRITICAL_SECTION* cs2, struct message* newMessage, SOCKET* clientSocket) {
	ringBufPutMessage(storingBuffer, cs, *newMessage);
	while (1)
	{
		retrievedData data = ringBufReadRetrievedData(retrievingBuffer, cs2);
		if (data.processId == newMessage->processId)
		{
			data = ringBufGetRetrievedData(retrievingBuffer, cs2);
			int iResult = send(*clientSocket, (char*)&data, (short)sizeof(struct retrievedData), 0);
			// Check result of send function
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error in sending to replicator2: %d\n", WSAGetLastError());
				//closesocket(connectSocket);
				//WSACleanup();
				//break;
			}
			printf("Sent data to process %d.\n",newMessage->processId);
			break;
		}
		else
		{
			Sleep(500);
		}
	}
}

DWORD WINAPI ListenForRegistrationsThread(LPVOID lpParams)
{
	SOCKET clientSocket = (*(ThreadArgs*)(lpParams)).clientSocket;
	sockaddr_in clientAddr = (*(ThreadArgs*)(lpParams)).clientAddr;
	RingBuffer* storingBuffer= (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs= (*(ThreadArgs*)(lpParams)).cs;
	CRITICAL_SECTION* cs2 = (*(ThreadArgs*)(lpParams)).cs2;
	int id = (*(ThreadArgs*)(lpParams)).id;
	bool *end= (*(ThreadArgs*)(lpParams)).end;

	// Sockets used for communication with client
	bool flag = false;
	struct process newProcess;
	short processId;
	char* newAddr = inet_ntoa(clientAddr.sin_addr);
	message newMessage;
	strcpy(newProcess.ipAddr, newAddr);
	newProcess.port = ntohs(clientAddr.sin_port);
	printf("New process request accepted . Process address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	
	while (!(*end))
	{
		char dataBuffer[BUFFER_SIZE];
		int iResult;
		if (flag==false)
		{
			RegisterProcess(&clientSocket,&flag,&processId,&newProcess, &newMessage,storingBuffer,cs);
		}
		else
		{
			iResult = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);
			if (iResult > 0)
			{
				dataBuffer[iResult] = '\0';
				printf("Message received from client.\n");
				strcpy(newMessage.text, dataBuffer);
				newMessage.processId = processId;
				if (strcmp(newMessage.text, "get_data_from_replica") == 0) {
					MessageForRetreivingData(storingBuffer,retrievingBuffer,cs,cs2,&newMessage,&clientSocket); 
				}
				else {
					MessageForStoring(storingBuffer,cs,&newMessage);
				}
			}
			else if (iResult == 0)
			{
				// connection was closed gracefully
				ind = true;
				pBr = id;
			}
		}
	}
	// Deinitialize WSA library
	WSACleanup();
	printf("PROCESS THREAD ENDED\n");
}