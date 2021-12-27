#include "ReplicatorPrimHeader.h"


void ConncectWithReplicator2(RingBuffer* storingBuffer,RingBufferRetrieved* retrievingBuffer,CRITICAL_SECTION *cs) {
	// Socket used to communicate with server
	DWORD ConncectWithReplicator2ThreadID[NUMOF_THREADS];
	HANDLE hConncectWithReplicator2Thread[NUMOF_THREADS];

	SOCKET connectSocket[NUMOF_THREADS];
	for (int i = 0;i < NUMOF_THREADS;i++) {
		connectSocket[i] = INVALID_SOCKET;
	}
	int numOfConnected = 0;
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
	for (int i = 0;i < NUMOF_THREADS;i++) {
		connectSocket[i] = socket(AF_INET,
			SOCK_STREAM,
			IPPROTO_TCP);
		if (connectSocket[i] == INVALID_SOCKET)
		{
			printf("socket %d failed with error: %ld\n",i, WSAGetLastError());
			WSACleanup(); 
			return;
		}
		// Create and initialize address structure
		sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;								// IPv4 protocol
		serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
		serverAddress.sin_port = htons(SERVER_PORT2);					// server port
		// Connect to server specified in serverAddress and socket connectSocket
		iResult = connect(connectSocket[i], (SOCKADDR*)&serverAddress, sizeof(serverAddress));
		if (iResult == SOCKET_ERROR)
		{
			printf("Socket %d unable to connect to server.\n",i);
			closesocket(connectSocket[i]);
			WSACleanup();
			return;
		}
		ThreadArgs threadArgs;
		threadArgs.clientSocket = &connectSocket[numOfConnected];
		threadArgs.storingBuffer = storingBuffer;
		threadArgs.retrievingBuffer = retrievingBuffer;
		threadArgs.cs = cs;
		hConncectWithReplicator2Thread[numOfConnected] = CreateThread(NULL, 0, &ConncectWithReplicator2Thread, &threadArgs, 0, &ConncectWithReplicator2ThreadID[numOfConnected]);
		
		numOfConnected++;
	}
}


DWORD WINAPI ConncectWithReplicator2Thread(LPVOID lpParams) {
	int iResult;
	message m;
	printf("Nit konektovana na replicator2.\n");
	SOCKET connectSocket = *(*(ThreadArgs*)(lpParams)).clientSocket;
	RingBuffer* storingBuffer= (*(ThreadArgs*)(lpParams)).storingBuffer;
	RingBufferRetrieved* retrievingBuffer = (*(ThreadArgs*)(lpParams)).retrievingBuffer;
	CRITICAL_SECTION* cs = (*(ThreadArgs*)(lpParams)).cs;
	Sleep(3000);
	
	while (1)
	{
		char dataBuffer[BUFFER_SIZE];
		m = ringBufGetMessage(storingBuffer, cs);
		printf("Cheking for messages to send...\n");
		Sleep(10000);
		if (m.processId == -1)
			continue;
		iResult = send(connectSocket, (char*)&m, (short)sizeof(message), 0);
		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			break;
		}
		if (strcmp(m.text, "get_data_from_replica") == 0)
		{
			
			printf("Sent request for retrieving data. Process waiting for response from replicator2.");
			iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
			dataBuffer[iResult] = '\0';
			printf("Process received from server: ");
			retrievedData response = *(retrievedData*)dataBuffer;
			//printf("OVDJE ISPISATI PODATKE KOJE JE VRATIO REPLICATOR");//ne znam kako **char da ispisem :)))
			ringBufPutRetrievedData(retrievingBuffer, response,cs);
			printBufferRetrievedData(*retrievingBuffer,cs);
			
		}
		else
		{
			printf("Sent data to replicator2.");
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


	//int iResult;
	//short serviceId;
	//printf("Unesite id procesa: ");
	//scanf_s("%d", &serviceId);
	//serviceId = (short)htons(serviceId);  //obavezna funkcija htons() jer cemo slati podatak tipa short 
	////getchar();    //pokupiti enter karakter iz bafera tastature
	//// Slanje pripremljene poruke zapisane unutar strukture studentInfo
	////prosledjujemo adresu promenljive student u memoriji, jer se na toj adresi nalaze podaci koje saljemo
	////kao i velicinu te strukture (jer je to duzina poruke u bajtima)
	//iResult = send(connectSocket, (char*)&serviceId, (short)sizeof(short), 0);
	//// Check result of send function
	//if (iResult == SOCKET_ERROR)
	//{
	//	printf("send failed with error: %d\n", WSAGetLastError());
	//	closesocket(connectSocket);
	//	WSACleanup();
	//	return;
	//}
	//printf("Registration message with ID successfully sent. Total bytes: %ld\n", iResult);
	//while (true)
	//{
	//	int option;
	//	struct message messageForRepl;

	//	//char message[MESSAGE_LEN];
	//	strcpy(messageForRepl.text, Menu());//Dobavljanje komande koju saljemo replicator1
	//	messageForRepl.serviceId = serviceId;
	//	//messageForRepl.serviceId = htons(messageForRepl.serviceId); vec je gore uradjen htons nad serviceId

	//	//printf("ID... id %d", messageForRepl.serviceId);
	//	if (strcmp(messageForRepl.text, "get_data_from_replica") == 0) {
	//		iResult = send(connectSocket, (char*)&messageForRepl, (int)sizeof(messageForRepl), 0);
	//		// Check result of send function
	//		if (iResult == SOCKET_ERROR)
	//		{
	//			printf("send failed with error: %d\n", WSAGetLastError());
	//			closesocket(connectSocket);
	//			WSACleanup();
	//			return;
	//		}
	//		printf("Process waititng respons from replicator.");
	//		iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
	//		dataBuffer[iResult] = '\0';
	//		printf("Process received from server: ");
	//		char* message = dataBuffer;
	//		printf("OVDJE ISPISATI PODATKE KOJE JE VRATIO REPLICATOR");
	//	}
	//	else if (strcmp(messageForRepl.text, "turn_off") == 0) {
	//		break;
	//	}
	//	else {
	//		iResult = send(connectSocket, (char*)&messageForRepl, (int)sizeof(struct message), 0);
	//		// Check result of send function
	//		if (iResult == SOCKET_ERROR)
	//		{
	//			printf("send failed with error: %d\n", WSAGetLastError());
	//			closesocket(connectSocket);
	//			WSACleanup();
	//			return;
	//		}
	//		printf("Message %s sucssesfuly sent to replicator. ID = %d", messageForRepl.text, messageForRepl.serviceId);
	//	}
	//}
	
}
