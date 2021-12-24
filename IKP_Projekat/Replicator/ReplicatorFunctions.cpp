#include "ReplicatorPrimHeader.h"

bool RegisterService(struct process newProcess) {
	if (find(newProcess.id) == NULL) {
		insertFirst(newProcess.id, newProcess.ipAddr, newProcess.port);
		return true;
	}
	else
	{
		if (update(newProcess.id, newProcess.ipAddr, newProcess.port))
		{
			printf("Updated.");
			return true;
		}
	}
	return false;
}


void InitializePorts() {
	for (int i = 0;i < MAX_CLIENTS;i++) {
		struct port p;
		p.val = 27000 + i;
		p.ind = false;
		ports[i] = p;
	}
}

int FindEmptyPort() {
	for (int i = 0;i < MAX_CLIENTS;i++) {
		if (!ports[i].ind)
			return ports[i].val;
	}
	return -1;
}

void FreePort(int port) {
	for (int i = 0;i < MAX_CLIENTS;i++) {
		if (ports[i].val == port)
			ports[i].ind = false;
	}
}
