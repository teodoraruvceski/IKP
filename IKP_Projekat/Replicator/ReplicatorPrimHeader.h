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
#include "RingBuffer.h"
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 
#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define SERVER_PORT2 27017
#define MAX_CLIENTS 10
#define NUMOF_THREADS 3

//globalna promenljiva buffera
 //RingBuffer* storingBuffer;

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

struct ThreadArgs{
    SOCKET* clientSocket;
    sockaddr_in clientAddr;
    RingBuffer* storingBuffer;
    RingBufferRetrieved* retrievingBuffer;
    CRITICAL_SECTION* cs;
};

//DWORD WINAPI ListenForRegistrations(LPVOID lpParams);
void ListenForRegistrations(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs);
DWORD WINAPI ListenForRegistrationsThread(LPVOID lpParams);

void ConncectWithReplicator2(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs);
DWORD WINAPI ConncectWithReplicator2Thread(LPVOID lpParams);

bool RegisterService(struct process);

void SendData(int serviceId, void* data, int dataSize);

void RecieveData(void* data, int dataSize);


int FindEmptyPort();
void InitializePorts();
void FreePort(int port);

#endif
