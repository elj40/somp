#ifndef UTILS_H
#define UTILS_H

#define EPSILON (1.0/2048.0)
#define ArrayCount(array) (sizeof(array)/sizeof(array[0])) // NOTE: this only works in same scope as when the array was made

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
int nearlyEqual(float a, float b);
void printInt(const void * i);

// Generic linked list operations
void LL_push(LL_Node ** head, void * data);
LL_Node * LL_remove(LL_Node ** head, void * data);
void LL_print(LL_Node * head, void (*printFunc)(const void *) );
void LL_free(LL_Node * head);

#endif // UTILS_H
