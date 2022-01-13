#include "Replica.h"


int main(int argc, char* argv[]) {
	short pId = atoi(argv[1]);//var for saving process id
	//short pId = 2;
	ConnectToReplicator2(pId);
	return 0;
}