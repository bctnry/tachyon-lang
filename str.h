#ifndef TACHYON__BASIS_STRING
#define TACHYON__BASIS_STRING

#include <stddef.h>
#include <stdbool.h>

typedef struct String {
	char* base;
	_Bool allocated;
	size_t len;
} String;

// Create a string object from static character array.
String* String_NewFromStatic(char* s);

// Create a string object from allocated character array.
// This function takes the ownership from its caller; the caller
// should not free the array.
String* String_NewFromAllocated(char* s);

// Creates a new string object that combines the two string object.
// This function will NOT free any of its arguments.
String* String_Append(String* s1, String* s2);

// Creates a new string object that appends multiple string together.
// This function will NOT free any of its arguments.
String* String_Join(String** s, size_t slen, String* sep);

// Creates a new string object from a substring of an existing
// string object. This function will NOT free any of its arguments.
String* String_SubString(String* s, size_t start, size_t end);

// Dispose a string object and free all the related memory.
void String_Dispose(String* s);

// Convert a string object to a c-style zero-terminated string.
char* String_ToCharArray(String* s);

#endif
