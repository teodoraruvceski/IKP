#pragma once
#ifndef REPLICATORSEC_H_   
#define REPLICATORSEC_H_
#define WIN32_LEAN_AND_MEAN
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"

#include <iostream> 
#include "RingBuffer.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 
#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27017
#define SERVER_PORT_REP1 27018
#define REPLICA_LISTEN_PORT 27019
#define MAX_CLIENTS 10
#define NUMOF_THREADS 3
#define NUMOF_THREADS_SENDING 3
struct port {
    int val;
    bool ind;
};


static struct port ports[MAX_CLIENTS];


struct ThreadArgs {
    SOCKET clientSocket;
    sockaddr_in clientAddr;
    RingBuffer* storingBuffer;
    RingBufferRetrieved* retrievingBuffer;
    CRITICAL_SECTION* cs;
};

struct process {
    char ipAddr[15];
    int port;
    int id;
};

struct clientConnection {
    SOCKET clientSocket;
    sockaddr_in clientAddr;
};

//DWORD WINAPI ListenForRegistrations(LPVOID lpParams);
void ListenForReplicator1Registrations(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer,CRITICAL_SECTION* cs);
void ListenForReplica(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, SOCKET* clientSocketsReplica);
DWORD WINAPI ListenForReplicator1Thread(LPVOID lpParams);
DWORD WINAPI SendToReplicator1Thread(LPVOID lpParams);
DWORD WINAPI SendToReplica(LPVOID lpParams);

bool RegisterService(struct process);

void SendData(int serviceId, void* data, int dataSize);

void RecieveData(void* data, int dataSize);


int FindEmptyPort();
void InitializePorts();
void FreePort(int port);

#endif
