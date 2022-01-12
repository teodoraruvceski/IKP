#pragma once
#ifndef REPLICA_H_   
#define REPLICA_H_
#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"
#include <iostream> 
#include "List.h"
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 
#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27019 /// port replikatora2 na koj i se konektuju replike
#define BUFFER_SIZE 4096

#define MAX_CLIENTS 20
#define NUMOF_THREADS 1
#define TEXT_SIZE 2048

struct message {
	char text[TEXT_SIZE];
	short processId;
};

struct retrievedData {
	char data[TEXT_SIZE];
	short processId;
	int dataCount;
};


struct ThreadArgs {
    int clientSocket;
    sockaddr_in clientAddr;
    CRITICAL_SECTION* cs;
};

void ConnectToReplicator2(short id);
void RetreiveData(SOCKET* connectSocket, struct listItem* head, int* count, struct message* m);
#endif
