#include "ReplicatorSecHeader.h"

void InitReplicaArray(int array[]) {
	for (int i = 0;i < MAX_CLIENTS;i++) {
		array[i] = -1;
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
			if (array[i] == -1)
				array[i] = id;
		}
	}
	return ind;
}
