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

// Free all memory used by the array list.
// If any of the elements used memory allocated w/ malloc/calloc,
// the caller should free them as well.
void ArrayList_Dispose(ArrayList *l);

void ArrayList_ForEach(ArrayList* l, void(*f)(size_t i, void* elem));
void *ArrayList_Nth(ArrayList *l, size_t n);
void *ArrayList_Pop(ArrayList *l);
void* ArrayList_Head(ArrayList *l);
size_t ArrayList_Length(ArrayList *l);

#endif

