#include "process.h"

int main()
{
	//short* serviceId=(short*)malloc(sizeof(short));
	short serviceId;
	RegisterService(&serviceId);
	printf("Process sucssesfuly closed.\n");
	printf("Press any key to close program . . .\n");
	//free(serviceId);
	_getch();
	return 0;
}
