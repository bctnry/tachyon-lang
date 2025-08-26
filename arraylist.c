#include "arraylist.h"
#include <stddef.h>
#include <stdlib.h>


ArrayList* ArrayList_Create(size_t cap) {
	ArrayList* res = (ArrayList*)calloc(1, sizeof(ArrayList));
	res->base = (void**)calloc(cap, sizeof(void*));
	res->cap = cap;
	res->len = 0;
	return res;
}

void ArrayList_Append(ArrayList* l, void* elem) {
	if (l->len + 1 >= l->cap) {
		l->base = realloc(l->base, l->cap*2*sizeof(void*));
		l->cap *= 2;
	}
	l->base[l->len] = elem;
	l->len++;
}

void ArrayList_Dispose(ArrayList* l) {
	free(l->base);
	free(l);
}

void ArrayList_ForEach(ArrayList* l, void(*f)(size_t i, void* elem)) {
	for (size_t i = 0; i < l->len; i++) {
		f(i, l->base[i]);
	}
}

void *ArrayList_Nth(ArrayList *l, size_t n) { return l->base[n]; }

// Pop the first element from the array list. This function will not
// free the memory used by the element.
void* ArrayList_Pop(ArrayList *l) {
	void* res = l->base[l->len-1];
	l->len -= 1;
	return res;
}
void* ArrayList_Head(ArrayList *l) {
	if (l->len <= 0) { return NULL; }
	return l->base[l->len-1];
}
size_t ArrayList_Length(ArrayList *l) { return l->len; }

