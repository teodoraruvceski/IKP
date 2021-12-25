//#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>

#include "ReplicatorPrimHeader.h"
// TCP server that use non-blocking sockets
int main()
{

	InitializePorts();//dodao da bi mogli da imamo 10 portova struct{brporta , indikator je li zauzet}
	DWORD ListenForRegistrationsID;
	HANDLE hListenForRegistrations;
	//hListenForRegistrations = CreateThread(NULL, 0, &ListenForRegistrations, NULL, 0, &ListenForRegistrationsID);
	ConncectWithReplicator2();
	ListenForRegistrations();
	_getch();

	//CloseHandle(hListenForRegistrations);

	return 0;
}
