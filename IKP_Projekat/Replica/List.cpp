#include "List.h"

void init_list(listItem** head) {
    *head = NULL;
}

void add_to_list(listItem* m, listItem** head,int* count) {
    if (*head == NULL) { // list is empty
        *head = m;
        (*count)++;
        return;
    }

    add_to_list(m, &((*head)->next),count);
}

listItem* create_new_item(char text[],  short id) {
    listItem* m = (listItem*)malloc(sizeof(listItem));
    if (m == NULL) {
        printf("Not enough RAM!\n");
        exit(21);
    }

    strcpy(m->text, text);
    m->processId = id;

    m->next = NULL;

    return m;
}

struct retrievedData *retrieve(listItem** head,int *count)
{
    //struct retrievedData* ret=malloc(sizeof(struct retrievedData));
    struct retrievedData ret = { (char**)malloc(2048 * sizeof(char*)),0,0};
    if (*head == NULL)
    {
        ret.processId = -1;
        ret.dataCount = 0;
        return &ret;
    }
    ret.processId = (*head)->processId;
    ret.dataCount = *count;
   /* for (int i = 0;i < *count;i++)
    {
        ret.data[i] = (*head)->text;
        printf("Text: %s\n", ret.data[i]);
        *head = (*head)->next;
    }*/
    return &ret;
}

void destroy_list(listItem** head) {
    if (*head != NULL) {
        destroy_list(&((*head)->next));
        free(*head);
        *head = NULL;
    }
}