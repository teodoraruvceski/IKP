#include "ReplicatorSecHeader.h"

//DWORD WINAPI ListenForRegistrations(LPVOID lpParams)
void ListenForReplicator1Registrations()
{
	DWORD ListenForReplicator1ThreadID[NUMOF_THREADS];
	HANDLE hListenForReplicator1Thread[NUMOF_THREADS];
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
				getch();
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
				struct clientConnection cc; //mpoguca greska da se prepise nove=a preko stare strukture
				cc.clientSocket = clientSockets[lastIndex];
				cc.clientAddr = clientAddr;
				hListenForReplicator1Thread[threadNum] = CreateThread(NULL, 0, &ListenForReplicator1Thread, &cc, 0, &ListenForReplicator1ThreadID[threadNum]);
				threadNum++;
				lastIndex++;
			}
		}
	}
	for (int i = 0;i < threadNum;i++)
		CloseHandle(hListenForReplicator1Thread[i]);
	//Close listen and accepted sockets
	closesocket(listenSocket);

	// Deinitialize WSA library
	WSACleanup();
}

DWORD WINAPI ListenForReplicator1Thread(LPVOID lpParams)
{

	SOCKET clientSocket = (*(clientConnection*)(lpParams)).clientSocket;
	sockaddr_in clientAddr = (*(clientConnection*)(lpParams)).clientAddr;
	// Sockets used for communication with client

	bool flag = false;
	char* message;
	struct process newProcess;

	char* newAddr = inet_ntoa(clientAddr.sin_addr);
	strcpy(newProcess.ipAddr, newAddr);
	newProcess.port = ntohs(clientAddr.sin_port);
	printf("New replicator1 connection request accepted . Replicator1 connection address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

	while (1)
	{
		_getch();
		//char dataBuffer[BUFFER_SIZE];
		//int iResult;
		//if (flag == false)
		//{
		//	iResult = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);
		//	if (iResult > 0)
		//	{
		//		dataBuffer[iResult] = '\0';
		//		//printf("Message received from client:\n");
		//		message = dataBuffer;
		//		short* pom = ((short*)(message));
		//		newProcess.id = ntohs(*pom);
		//		printf("Registration message %d \n", newProcess.id);
		//		flag = true;
		//		printf("_______________________________  \n");
		//		int newPort = RegisterService(newProcess); //cuvanje 
		//		if (iResult == SOCKET_ERROR)
		//		{
		//			printf("send failed with error: %d\n", WSAGetLastError());
		//			closesocket(clientSocket);
		//			WSACleanup();
		//			return 0;
		//			//return;
		//		}
		//		printf("Response successfully sent. Total bytes: %ld\n", iResult);
		//		continue;
		//	}
		//	else if (iResult == 0)
		//	{
		//		// connection was closed gracefully
		//		printf("Connection with client closed.\n");
		//		closesocket(clientSocket);
		//	}
		//	else
		//	{
		//		// there was an error during recv
		//		//printf("recv failed with error: %d RECIVING ID\n", WSAGetLastError());
		//		//closesocket(clientSocket);
		//	}
		//}
		//iResult = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);
		//if (iResult > 0)
		//{
		//	dataBuffer[iResult] = '\0';
		//	printf("Message received from client\n");
		//	struct message* message2 = (struct message*)dataBuffer;
		//	message2->serviceId = ntohs(message2->serviceId);
		//	if (strcmp(message2->text, "get_data_from_replica") == 0) {



		//		//iResult = send(clientSocket, (char*)&newPort, (int)sizeof(int), 0);
		//		//if (iResult == SOCKET_ERROR)
		//		//{
		//		//	printf("send failed with error: %d\n", WSAGetLastError());
		//		//	closesocket(clientSocket);
		//		//	WSACleanup();
		//		//	return 0;
		//		//	//return;
		//		//}
		//		//printf("Response successfully sent. Total bytes: %ld\n", iResult);
		//	}
		//	else {
		//		printf("\nPoruka %s  id= %d", message2->text, message2->serviceId);
		//	}
		//}
		//else if (iResult == 0)
		//{
		//	// connection was closed gracefully
		//	printf("Connection with client closed.\n");
		//	closesocket(clientSocket);
		//}
		//else
		//{
		//	//there was an error during recv
		//	//printf("recv failed with error: %d RECIVING MESSAGES\n", WSAGetLastError());
		//	//closesocket(clientSocket);
		//}
		////Close listen and accepted sockets
		////closesocket(listenSocket);


	}
	// Deinitialize WSA library
	WSACleanup();

}