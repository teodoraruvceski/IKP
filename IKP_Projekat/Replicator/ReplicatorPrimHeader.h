#pragma once
#ifndef REPLICATORPRIM_H_   
#define REPLICATORPRIM_H_
#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"

#include <iostream> 
#include "Map.h"
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 
#pragma pack(1)


#define SERVER_PORT 27016
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10

struct port {
    int val;
    bool ind;
};

static struct port ports[MAX_CLIENTS];


struct process {
    char ipAddr[15];
    int port;
    int id;
};

struct clientConnection{
    SOCKET clientSocket;
    sockaddr_in clientAddr;
};

//DWORD WINAPI ListenForRegistrations(LPVOID lpParams);
void ListenForRegistrations();
DWORD WINAPI ListenForRegistrationsThread(LPVOID lpParams);

bool RegisterService(struct process);

void SendData(int serviceId, void* data, int dataSize);

void RecieveData(void* data, int dataSize);


int FindEmptyPort();
void InitializePorts();
void FreePort(int port);

#endif
