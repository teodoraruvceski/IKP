#include "Replica.h"


int main(int argc, char* argv[]) {
	//int pId = atoi(argv[1]);//id procesa <<<<<<<<<<<<<<<
	short pId = 1;
	printf("Id: \n");
	
	int count = 0;
	listItem* head;
	init_list(&head);
	printf("I AM ALIVEEEEEEEEE\n");
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
		return -1;
	}
	// create a socket
	connectSocket = socket(AF_INET,
							SOCK_STREAM,
							IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	
	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port
	// Connect to server specified in serverAddress and socket connectSocket
	message m;
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
		
		//if (iResult == SOCKET_ERROR)
		//{
		//	printf("Unable to connect to server.\n");
		//	//_getch();
		//	//closesocket(connectSocket);
		//	//WSACleanup();
		//	//return -1;
		//	Sleep(2000);
		//}
		//else
		//{
		//	break;
		//}
	
	
	printf("HTONS:%d\n", htons(pId));
	m.processId = htons(pId);
	iResult = send(connectSocket, (char*)&m, (int)sizeof(struct message), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return -1;
	}
	printf("Hello message with your id successfully sent. Total bytes: %ld\n", iResult);

	while (1)
	{
		iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0)
		{
			dataBuffer[iResult] = '\0';
			m = *(message*)(dataBuffer);
			printf("Message received from rep2.\n");
			printf("Message: %s\n", m.text);
			if (strcmp(m.text, "get_data_from_replica") == 0)
			{
				//retrieve data from storage
				printf("Retrieving.\n");
				struct retrievedData retrievedData = retrieve(&head, &count);
				printf("ProcessId: %d\n", retrievedData.processId);
				if (retrievedData.processId == -1)
				{
					retrievedData.processId = m.processId;
				}
				
				iResult = send(connectSocket, (char*)&retrievedData, (short)sizeof(struct retrievedData), 0);
				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					/*closesocket(connectSocket);
					WSACleanup();
					return -1;*/
				}
				printf("Message with retrieved data successfully sent. Total bytes: %ld\n", iResult);
			}
			else
			{
				listItem* item = create_new_item(m.text, m.processId);
				add_to_list(item, &head,&count);
			}
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			Sleep(10000);
			//closesocket(clientSocket);
		}
		else
		{
			//there was an error during recv
			//printf("recv failed with error: %d RECIVING MESSAGES\n", WSAGetLastError());
			//closesocket(clientSocket);
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
		return -1;
	}
	Sleep(1000);
	// Close connected socket
	closesocket(connectSocket);
	// Deinitialize WSA library
	WSACleanup();
	return -1;
}