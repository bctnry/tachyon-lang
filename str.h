#ifndef TACHYON__BASIS_STRING
#define TACHYON__BASIS_STRING

#include <stddef.h>
#include <stdbool.h>

typedef struct String {
	char* base;
	size_t len;
} String;

// Create a string object from a character array.
// This function creates a clone of the character array. If the
// caller has acquired the memory by using malloc/calloc or other
// means, they own the memory and should free them when appropriate.
String *String_New(const char *s);

// Creates a new string object that combines the two string object.
// This function will NOT dispose any of its arguments.
String* String_Append(String* s1, String* s2);

// Creates a new string object that appends multiple string together.
// This function will NOT dispose any of its arguments.
String* String_Join(String** s, size_t slen, String* sep);

// Creates a new string object from a substring of an existing
// string object. This function will NOT dispose any of its arguments.
String* String_SubString(String* s, size_t start, size_t end);

// Dispose a string object and free all the related memory.
void String_Dispose(String* s);

// Convert a string object to a c-style zero-terminated string.
// This would allocate a new memory block, whose full ownership is
// given to the caller. Freeing the string object will not free
// this memory block; the caller is responsible for freeing it.
char* String_CloneToCharArray(String* s);

#endif
