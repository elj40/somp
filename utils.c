/*
* Filename:	utils.c
* Date:		14/12/2024 
* Name:		EL Joubert
*
* Some simple quality of life functions
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utils.h"

int ArrayMax(int nums[], int n)
{
	int max = nums[0];
	for (int i = 1; i < n; i++)
	{
		max = (nums[i] > max) ? nums[i] : max;
	}

	return max;
}
float ArrayMaxf(float nums[], int n)
{
	float max = nums[0];
	for (int i = 1; i < n; i++)
	{
		max = (nums[i] > max) ? nums[i] : max;
	}

	return max;
}

int nearlyEqual(float a, float b)
{
	return fabs(a - b) < EPSILON;
}

// Used as a generic print function for printing ints
void printInt(const void * i)
{
	int * n = (int *) i;
	printf("%d", *n);
}


// Linked list functions
void LL_push(LL_Node ** head, void * data)
{
	LL_Node * node = malloc(sizeof(LL_Node));
	node->data = data;
	node->next = NULL; 

	if (*head == NULL)
	{
		*head = node;
		return;
	}

	LL_Node * current = *head;
	while (current->next)
	{
		current = current->next;
	}
	current->next = node;
}

LL_Node * LL_remove(LL_Node ** head, void * data)
{
	LL_Node * p;
	if ((*head)->data == data)
	{
		p = *head;
		*head = (*head)->next;
		return p;
	}

	LL_Node * current = *head;
	while (current->next)
	{
		if (current->next->data == data)
		{
			p = current->next;
			current->next = p->next;
			return p;
		}

		current = current->next;
	}

	return NULL;
}

void LL_print(LL_Node * head, void (*printFunc)(const void *) )
{
	LL_Node * current = head;
	while (current)
	{
		(*printFunc)(current->data);
		printf("->");
		current = current->next;
	}
	printf("NULL\n");
}

void LL_free(LL_Node * head)
{
	LL_Node * current = head;
	LL_Node * next;
	while (current)
	{
		next = current->next;
		free(current);
		current = next;
	}
}


