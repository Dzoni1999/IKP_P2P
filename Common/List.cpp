#include "List.h"


void initlist(List* ilist) {
	ilist->head = 0;
}

void insertfront(List* ilist, UserData val) {
	ListItem* newitem;
	newitem = (ListItem*)malloc(sizeof(ListItem));
	newitem->next = ilist->head;
	newitem->data = val;
	ilist->head = newitem;
}

void insertback(List* ilist, UserData val) {
	ListItem* ptr;
	ListItem* newitem;
	newitem = (ListItem*)malloc(sizeof(ListItem));
	newitem->data = val;
	newitem->next = 0;
	if (!ilist->head) {
		ilist->head = newitem;
		return;
	}
	ptr = ilist->head;
	while (ptr->next)
	{
		ptr = ptr->next;
	}
	ptr->next = newitem;
}

int length(List ilist) { /* returns list length */
	ListItem* ptr;
	int count = 1;
	if (!ilist.head) return 0;
	ptr = ilist.head;
	while (ptr->next) {
		ptr = ptr->next;
		count++;
	}
	return count;
}

void destroy(List* ilist) { /* deletes list */
	ListItem* ptr1,
		* ptr2;
	if (!ilist->head) return; /* nothing to destroy */
	ptr1 = ilist->head; /* destroy one by one */
	while (ptr1) {
		ptr2 = ptr1;
		ptr1 = ptr1->next;
		free(ptr2);
	}
	ilist->head = 0;
}

void setitem(List* ilist, int n, UserData val) {
	/* modifies a value*/
	/* assume length is at least n long */
	ListItem* ptr;
	int count = 0;
	if (!ilist->head) return;
	ptr = ilist->head;
	for (count = 0; count < n; count++)
	{
		if (ptr) ptr = ptr->next;
		else return;
	}
	if (ptr)
		ptr->data = val;
}

UserData getitem(List ilist, int n) {
	/* returns a list value,
	* assume length is at least n long */
	ListItem* ptr;
	int count = 0;
	UserData retval = {};
	retval.id = -1;
	if (!ilist.head) return retval;
	ptr = ilist.head;
	if (n == 0) return ptr->data;
	while (ptr->next) {
		ptr = ptr->next;
		count++;
		if (n == count)
			return (ptr->data);
	}
	return retval;
}

UserData getitembyid(List ilist, int id)
{
	ListItem* ptr;
	int count = 0;
	UserData retval = {};
	retval.id = -1;
	if (!ilist.head) return retval;
	ptr = ilist.head;
	if (ptr->data.id == id) return ptr->data;
	while (ptr->next) {
		ptr = ptr->next;
		if (ptr->data.id == id)
			return (ptr->data);
	}
	return retval;
}

void displaylist(List* list) {
	List* temp;
	if (list == NULL)
	{
		printf(" List is empty.");
	}
	else
	{
		temp = list;
		while (temp->head != NULL)
		{
			printf("%d ", temp->head->data);       // prints the data of current node
			temp->head = temp->head->next;         // advances the position of current node
		}
	}
}
