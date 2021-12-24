#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node {
    char ipAddr[15];
    int port;
    int key;
    struct node* next;
};

static struct node* head = NULL;
static struct node* current = NULL;

//display the list
void printList();

//insert link at the first location
void insertFirst(int key, char* ipAddr, int port);

//delete first item
struct node* deleteFirst();

//is list empty
bool isEmpty();

int length();

//find a link with given key
struct node* find(int key);

//delete a link with given key
struct node* deletel(int key);

void sort();

void reverse(struct node** head_ref);

bool update(int key, char* ipAddr, int port);