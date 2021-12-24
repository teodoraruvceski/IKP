#include "process.h"

char* Menu() {
	char option;
	char message[MESSAGE_LEN];
	while (true) {	
		printf("Odaberite opciju:\n");
		printf("1. Posalji podatke.\n");
		printf("2. Povuci podatke sa replike.\n");
		printf("3. Ugasi proces.\n");
		option = _getch();
		switch (option) {
			case '1':
				printf("Unesite poruku: ");
				scanf("%s", message,2);
				return message;
			case '2':
				strcpy(message, "get_data_from_replica");
				return message;
			case '3':
				strcpy(message, "turn_off");
				return message;
			default:
				printf("Pogresan unos opcije.");
				break;			
		}
	}
	return message;
}

void SendData(int serviceId, void* data, int dataSize) {}

void RecieveData(void* data, int dataSize) {}

void RegisterService() {
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
	short serviceId;
	printf("Unesite id procesa: ");
	scanf_s("%d", &serviceId);
	serviceId =(short) htons(serviceId);  //obavezna funkcija htons() jer cemo slati podatak tipa short 
	//getchar();    //pokupiti enter karakter iz bafera tastature
	// Slanje pripremljene poruke zapisane unutar strukture studentInfo
	//prosledjujemo adresu promenljive student u memoriji, jer se na toj adresi nalaze podaci koje saljemo
	//kao i velicinu te strukture (jer je to duzina poruke u bajtima)
	iResult = send(connectSocket, (char*)&serviceId, (short)sizeof(short), 0);
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
		char message[MESSAGE_LEN];
		strcpy(message,Menu());//Dobavljanje komande koju saljemo replicator1
		if (strcmp(message, "get_data_from_replica")==0) {
			iResult = send(connectSocket, message, strlen(message), 0);
			// Check result of send function
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return;
			}
			printf("Process waititng respons from replicator.");
			iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
			dataBuffer[iResult] = '\0';
			printf("Process received from server: ");
			char* message = dataBuffer;
			printf("OVDJE ISPISATI PODATKE KOJE JE VRATIO REPLICATOR");
		}
		else if(strcmp(message, "turn_off")==0){
			break;
		}
		else{
			iResult = send(connectSocket, message, sizeof(message), 0);
			// Check result of send function
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return;
			}
			printf("bytes sent %d\n", iResult);
			printf("Message %s sucssesfuly sent to replicator.",message);
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
}