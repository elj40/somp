#ifndef UTILS_H
#define UTILS_H
/*
* Filename:	utils.h
* Date:		14/12/2024 
* Name:		EL Joubert
*
* Some simple quality of life functions
*/

#define EPSILON (1.0/2048.0)
#define ArrayCount(array) (sizeof(array)/sizeof(array[0])) // NOTE: this only works in same scope as when the array was made

#include <assert.h>
#define DEFAULT_DA_CAPACITY 5
#define DynamicArrayAppend(da, item) \
    do { \
        if ((da)->count >= (da)->capacity) \
        { \
            if ((da)->capacity == 0) (da)->capacity = DEFAULT_DA_CAPACITY; \
            else (da)->capacity *= 2; \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof((da)->items[0])); \
            assert((da)->items != NULL); \
        } \
        (da)->items[(da)->count] = item; \
        (da)->count++; \
    } while(0); 

#define shift_array(arr, start, end) do { \
    for (int shift_array_index = start; shift_array_index < end; shift_array_index++) { \
        (arr)[shift_array_index] = (arr)[shift_array_index+1]; \
    } \
} while (0); \

#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#define MAX(a, b) ((a) > (b)) ? (a) : (b)

struct LL_Node
{
	void * data;
	struct LL_Node * next;
};

typedef struct LL_Node LL_Node;

// Array operations
int ArrayMax(int nums[], int n);
float ArrayMaxf(float nums[], int n);

// Misc
int nearly_equal(float a, float b);
void printInt(const void * i);
void line_from_points(float * m, float * c, float ax, float ay, float bx, float by);

// Generic linked list operations
void LL_push(LL_Node ** head, void * data);
LL_Node * LL_remove(LL_Node ** head, void * data);
void LL_print(LL_Node * head, void (*printFunc)(const void *) );
void LL_free(LL_Node * head);

#ifdef UTILS_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

float lerp(float a, float b, float t)
{
    return a*(1-t) + b*t;
}
void line_from_points(float * m, float * c, float ax, float ay, float bx, float by)
{
    if (m != NULL) *m = (by-ay)/(bx-ax);
    if (c != NULL) *c = -((by-ay)/(bx-ax))*ax + ay;
};
int nearly_equal(float a, float b)
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


#endif // UTILS_IMPLEMENTATION

#endif // UTILS_H
