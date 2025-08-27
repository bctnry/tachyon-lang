#ifndef TACHYON__BASIS_ARRAYLIST
#define TACHYON__BASIS_ARRAYLIST

#include <stddef.h>

typedef struct ArrayList {
	void** base;
	size_t cap;
	size_t len;
} ArrayList;

// Create a new array list.
ArrayList *ArrayList_Create(size_t cap);

// Push a new element at the end of the array list.
void ArrayList_Append(ArrayList *l, void *elem);

// Free the memory used by the array list itself.
// The array list does not know the proper way of freeing the element,
// which means that the caller needs to free each of the values themselves.
void ArrayList_Dispose(ArrayList *l);

// For each elements of the array list, executes function `f`.
// `f` MUST NOT take the ownership of the elements.
void ArrayList_ForEach(ArrayList *l, void (*f)(size_t i, void *elem));

// Returns the nth element of the array list.
void *ArrayList_Nth(ArrayList *l, size_t n);

// Remove the element from the top of the array list. Returns NULL
// if the list is empty.
void *ArrayList_Pop(ArrayList *l);

// Return the element from the top of the array list without removing it.
// Returns NULL if the list is empty.
void *ArrayList_Head(ArrayList *l);

// Returns the length of the array list.
size_t ArrayList_Length(ArrayList *l);

#endif

