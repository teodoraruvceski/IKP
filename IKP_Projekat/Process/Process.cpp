#include "process.h"

//menu for user. user can chose 3 options
void Menu(struct message* messageForRepl) {
	char option;
	char* message = (char*)malloc(MESSAGE_LEN);
	while (true) {	
		printf("\nOdaberite opciju:\n");
		printf("1. Posalji podatke.\n");
		printf("2. Povuci podatke sa replike.\n");
		printf("3. Ugasi proces.\n");
		option = _getch();
		switch (option) {
			case '1':				
				printf("Unesite poruku: \n");
				scanf("%s", message); 				
				strcpy(messageForRepl->text, message);
				free(message);
				return;
			case '2':
				strcpy(messageForRepl->text, "get_data_from_replica");
				free(message);
				return;
			case '3':
				strcpy(messageForRepl->text, "turn_off");
				free(message);
				return;;
			default:
				printf("Pogresan unos opcije.\n");
				break;			
		}
	}
}
//function for sending data for storing at raplica
void SendData(SOCKET* connectSocket, struct message* messageForRepl) {
	int iResult = send(*connectSocket, (char*)messageForRepl, (int)sizeof(message), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(*connectSocket);
		WSACleanup();
		return;
	}
	printf("Message %s sucssesfuly sent to replicator.\n", messageForRepl->text);
}
//function for retreiving data from replica
void RecieveData(SOCKET* connectSocket, struct message* messageForRepl) {
	char dataBuffer[BUFFER_SIZE];
	int iResult = send(*connectSocket, (char*)messageForRepl, (int)sizeof(message), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(*connectSocket);
		WSACleanup();
		return;
	}
	printf("Process waititng respons from replicator.\n");
	iResult = recv(*connectSocket, dataBuffer, BUFFER_SIZE, 0);
	dataBuffer[iResult] = '\0';
	printf("Process received from replicator1: \n");
	char* message = dataBuffer;
	retrievedData data = *(retrievedData*)(message);
	data.dataCount = ntohs(data.dataCount);
	char delim[] = "\n";
	char* ptr = strtok(data.data, delim);
	printf("Data:\n");
	while (ptr != NULL)
	{
		printf("%s\n", ptr);
		ptr = strtok(NULL, delim);
	}
}
//function for registration and calling process operations 
void RegisterService(short *serviceId) {
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
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return;
	}
	//short serviceId;
	do{
		printf("\nUnesite id procesa(id mora biti veci od 0): ");
		scanf("%d", serviceId);
	} while (*serviceId <= 0);

	*serviceId = htons(*serviceId);
	iResult = send(connectSocket, (char*)serviceId, (short)sizeof(short), 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return;
	}
	printf("Registration message with ID successfully sent. Total bytes: %ld\n", iResult);
	while (true)
	{
		int option;
		struct message messageForRepl;

		//Menu for process
		Menu(&messageForRepl);
		messageForRepl.serviceId = *serviceId;

		if (strcmp(messageForRepl.text, "get_data_from_replica")==0) {
			RecieveData(&connectSocket, &messageForRepl);
		}
		else if(strcmp(messageForRepl.text, "turn_off")==0){
			
			break;
		}
		else{
			SendData(&connectSocket, &messageForRepl);
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
	// Close connected socket
	closesocket(connectSocket);
	// Deinitialize WSA library
	WSACleanup();
	return;
}