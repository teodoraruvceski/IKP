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
#include "RingBuffer.h"
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 
#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1" //localhost
#define SERVER_PORT 27016        /// replicator1 port for processes
#define SERVER_PORT2 27017       /// replicator2 port for 3 threads connection from replicator1
#define PORT_RCV_RET_DATA 27018  /// replicator1 port for 3 threads connections from replicator2
#define MAX_CLIENTS 2
#define NUMOF_THREADS_SENDING 3
#define NUMOF_THREADS_RECV 3

struct process {
    char ipAddr[15];
    int port;
    int id;
};

//struct for passing data to thread(thread function)
struct ThreadArgs{
    SOCKET clientSocket;
    sockaddr_in clientAddr;
    RingBuffer* storingBuffer;
    RingBufferRetrieved* retrievingBuffer;
    CRITICAL_SECTION* cs;
    CRITICAL_SECTION* cs2;
    int id;
    bool* end;
};
//functions for connection with processes
void ListenForRegistrations(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs,
    CRITICAL_SECTION* cs2, SOCKET* clientSocketsProcess, DWORD ListenForRegistrationsThreadID[MAX_CLIENTS],
    HANDLE hListenForRegistrationsThread[MAX_CLIENTS], bool* end);
DWORD WINAPI ListenForRegistrationsThread(LPVOID lpParams);
void RegisterProcess(SOCKET* clientSocket, bool* flag, short* processId, struct process* newProcess, struct message* newMessage, RingBuffer* storingBuffer, CRITICAL_SECTION* cs, bool* end);
void MessageForStoring(RingBuffer* storingBuffer, CRITICAL_SECTION* cs, struct message* newMessage);
void MessageForRetreivingData(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs, CRITICAL_SECTION* cs2, struct message* newMessage, SOCKET* clientSocket);

//functions for connections with replicator2
void ConnectWithReplicator2(RingBuffer* storingBuffer, RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs,
    CRITICAL_SECTION* cs2, SOCKET* clientSockets, SOCKET* connectSocket,
    DWORD ConnectWithReplicator2ThreadID[NUMOF_THREADS_SENDING], HANDLE hConnectWithReplicator2Thread[NUMOF_THREADS_SENDING],
    DWORD ListenForRecvRep2ThreadID[NUMOF_THREADS_RECV], HANDLE hListenForRecvRep2Thread[NUMOF_THREADS_RECV],bool*end);
//sending to replicator2
DWORD WINAPI SendToReplicator2Thread(LPVOID lpParams);
void SendData(RingBuffer* storingBuffer, CRITICAL_SECTION* cs, SOCKET* connectSocket);
//receiving from replicator2
DWORD WINAPI ReceiveFromReplicator2Thread(LPVOID lpParams);
void ReceiveData(RingBufferRetrieved* retrievingBuffer, CRITICAL_SECTION* cs2, SOCKET* connectSocket);

#endif
