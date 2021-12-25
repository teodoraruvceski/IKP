#include "process.h"

// TCP client that use non-blocking sockets
void StartProcess() {
	// Unos potrebnih podataka koji ce se poslati serveru	
	RegisterService();
}
int main()
{
	StartProcess();
	printf("Process sucssesfuly closed.\n");
	printf("Press any key to close program . . .\n");
	_getch();
	return 0;
}
