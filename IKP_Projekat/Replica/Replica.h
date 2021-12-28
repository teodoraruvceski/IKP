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
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 
#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27019 /// port replikatora1 na koj i se konektuju procesi
#define BUFFER_SIZE 4096

static CRITICAL_SECTION cs;


struct ThreadArgs {
    int clientSocket;
    sockaddr_in clientAddr;
    //RingBuffer* storingBuffer;
   // RingBufferRetrieved* retrievingBuffer;
    CRITICAL_SECTION* cs;
};

void ListenForReplicator2();
DWORD WINAPI ListenForReplicator2Thread(LPVOID lpParams);

#endif
