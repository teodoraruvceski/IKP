#include "Replica.h"

void RetreiveData(SOCKET* connectSocket,struct listItem* head,int* count,struct message* m) {
	listItem* pom = head;
	//retrieve data from storage
	printf("Retrieving data:\n");
	struct retrievedData data;
	struct retrievedData retrievedData = *retrieve(&pom, count);

	if (retrievedData.processId == -1)
	{
		retrievedData.processId = m->processId;
	}
	printf("%s\n", retrievedData.data);

	retrievedData.dataCount = htons(retrievedData.dataCount);
	int iResult = send(*connectSocket, (char*)&retrievedData, (short)sizeof(struct retrievedData), 0);
	//Check result of send function
	if (iResult == SOCKET_ERROR)
	{

		printf("send failed with error: %d\n", WSAGetLastError());
		Sleep(1000);
		closesocket(*connectSocket);
		WSACleanup();
		return;
	}
	printf("Message with retrieved data successfully sent. Total bytes: %ld\n", iResult);
}

void StoreData(listItem* head, int* count,struct message* m) {
	listItem* item = create_new_item(m->text, m->processId);
	printf("Storing data: %s.\n", item->text, item->processId);
	add_to_list(item, &head, count);
}

void ConnectToReplicator2(short id) {
	short pId = id;//var for saving process id
	printf("Replica for process with Id: %d\n", pId);
	printf("\n-----------------------------------REPLICA-----------------------------------\n");
	int count = 0;
	listItem* head;
	init_list(&head);
	// Socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;
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
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port
	// Connect to server specified in serverAddress and socket connectSocket
	message m;
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));

	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		_getch();
		closesocket(connectSocket);
		WSACleanup();
		return;
		Sleep(2000);
	}
	m.processId = htons(pId);
	iResult = send(connectSocket, (char*)&m, (int)sizeof(struct message), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return;
	}
	printf("Hello message to replicator2 with your id successfully sent. Total bytes: %ld\n", iResult);

	while (1)
	{
		iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0)
		{
			dataBuffer[iResult] = '\0';
			m = *(message*)(dataBuffer);
			printf("Message received from replicator2: %s\n",m.text);
			if (strcmp(m.text, "get_data_from_replica") == 0)
			{
				RetreiveData(&connectSocket,head,&count,&m);
			}
			else
			{
				StoreData(head, &count, &m);
			}
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with replicator2 closed.\n");
			Sleep(1000);
			closesocket(connectSocket);
		}
	}

	// Shutdown the connection since we're done
	iResult = shutdown(connectSocket, SD_BOTH);
	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return;
	}
	Sleep(1000);
	// Close connected socket
	closesocket(connectSocket);
	// Deinitialize WSA library
	WSACleanup();
	return ;
	
}