#include "process.h"

// TCP client that use non-blocking sockets
void StartProcess() {
	// Unos potrebnih podataka koji ce se poslati serveru	
	RegisterService();
}
int main()
{
	StartProcess();
	printf("Process sucssesfuly closed.");
	printf("Press any key to close program.");
	_getch();
	return 0;
}
