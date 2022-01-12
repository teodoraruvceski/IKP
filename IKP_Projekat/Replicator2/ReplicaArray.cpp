#include "ReplicatorSecHeader.h"

void InitReplicaArray(int array[]) {
	for (int i = 0;i < MAX_CLIENTS;i++) {
		array[i] = -1;
	}
}
void PrintReplicas(int* array)
{
	printf("replicas:\n");
	for (int i = 0;i < MAX_CLIENTS;i++)
	{
		printf("%d\n", (*array));
		array++;
	}
}
bool TryAddReplica(int array[],int id) {
	bool ind = true;
	for (int i = 0;i < MAX_CLIENTS;i++) {
		if (array[i] == id)
			ind = false;
	}
	if (ind == true) {
		for (int i = 0;i < MAX_CLIENTS;i++) {
			if (array[i] == -1) {
				array[i] = id;
				break;
			}
		}
	}
	return ind;
}
