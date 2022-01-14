#pragma once
#ifndef REPLICATORSEC_H_   
#define REPLICATORSEC_H_
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <shellapi.h>
#include <process.h>

#include <iostream> 
#include "RingBuffer.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 
#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27017           /// replicator2 port for 3 threads connection from replicator1
#define SERVER_PORT_REP1 27018      /// replicator1 port for 3 threads connections from replicator2
#define REPLICA_LISTEN_PORT 27019   /// replicator2 port for replicas
#define MAX_CLIENTS 10
#define NUMOF_THREADS 3
#define NUMOF_THREADS_SENDING 3

struct ThreadArgs {
    SOCKET clientSocket;
    sockaddr_in clientAddr;
    RingBuffer* storingBuffer;
    RingBufferRetrieved* retrievingBuffer;
    CRITICAL_SECTION* cs;
    CRITICAL_SECTION* cs2;
    int *replics;
    bool* end;
};

struct process {
    char ipAddr[15];
    int port;
    int id;
};

//functions for connection with replicator1
void ConnectWithReplicator1(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, CRITICAL_SECTION* cs2, ThreadArgs* threadArgs2, int replics[],
    DWORD ListenForReplicator1ThreadID[NUMOF_THREADS], HANDLE hListenForReplicator1Thread[NUMOF_THREADS], DWORD SendToReplicator1ThreadID[NUMOF_THREADS_SENDING],
    HANDLE hSendToReplicator1Thread[NUMOF_THREADS_SENDING],bool *end);
//sending to replicator1
DWORD WINAPI SendToReplicator1Thread(LPVOID lpParams);
void SendData(RingBufferRetrieved* retrievingBuffer, SOCKET* connectSocket, CRITICAL_SECTION* cs2);
//receiving from replicator1
DWORD WINAPI ListenForReplicator1Thread(LPVOID lpParams);
void ReceiveData(RingBuffer* storingBuffer, CRITICAL_SECTION* cs, SOCKET* clientSocket, int* replics);

//functions for connection with replicas
void ListenForReplica(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, CRITICAL_SECTION* cs2, SOCKET* clientSocketsReplica,
    DWORD ListenForReplicaThreadID[MAX_CLIENTS], HANDLE hListenForReplicaThread[MAX_CLIENTS] , bool* end);
DWORD WINAPI ConnectionWithReplicaThread(LPVOID lpParams);
void RegisterReplica(SOCKET* clientSocket, bool* flag, short* processId);
void MessageForStoring(SOCKET* clientSocket, struct message* mess);
void MessageForRetreivingData(SOCKET* clientSocket, struct message* mess, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs2, short* processId);

//functions for record replicas
void InitReplicaArray(int array[]);
bool TryAddReplica(int array[], int id);
void PrintReplicas(int* replicas);

#endif
