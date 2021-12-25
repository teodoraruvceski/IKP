#pragma once
#ifndef PROCESS_H_   
#define PROCESS_H_
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)
#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 4096
#define MESSAGE_LEN 40

struct process {
    char ipAddr[15];
    int port;
    int id;
};

struct message {
    char text[MESSAGE_LEN];
    short serviceId;
};

void RegisterService();

void SendData(int serviceId, void* data, int dataSize);

void RecieveData(void* data, int dataSize);

char* Menu();
//void StartProcess();

#endif
