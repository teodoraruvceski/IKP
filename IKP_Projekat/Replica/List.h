#pragma once
#include "Replica.h"
struct listItem {
    char text[1024];
    short processId;
    struct listItem* next;
};

void init_list(listItem** head);

void add_to_list(listItem* m, listItem** head,int *count);

listItem* create_new_item(char text[], short id);

void destroy_list(listItem** head);
struct retrievedData *retrieve(listItem** head, int* count);