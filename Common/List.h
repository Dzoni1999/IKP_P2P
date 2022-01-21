#pragma once
#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

typedef struct user_data {
	SOCKET socket;
	int id;
	char name[25];
	unsigned short listen_port;
	unsigned long ip_address;
}UserData;

typedef struct listitem {
	user_data data;
	struct listitem* next;
}ListItem;

typedef struct list {
	listitem* head;
}List;

void initlist(List*); /* initialize an empty list */
void insertfront(List*, UserData val); /* insert val at front */
void insertback(List*, UserData val); /* insert val at back */
int length(List); /* returns list length */
void destroy(List*); /* deletes list */
void setitem(List*, int n, UserData val);/* modifies item at n to val*/
UserData getitem(List, int n); /* returns value at n*/
UserData getitembyid(List, int id);
void displaylist(List*); /* displays the entire list*/

