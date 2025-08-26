#include "str.h"
#include <stdlib.h>
#include <string.h>

String* String_NewFromStatic(char* s) {
	String* res = (String*)calloc(1, sizeof(String));
	res->base = s;
	res->allocated = false;
	res->len = strlen(s);
	return res;
}

String* String_NewFromAllocated(char* s) {
	String* res = (String*)calloc(1, sizeof(String));
	res->base = s;
	res->allocated = true;
	res->len = strlen(s);
	return res;
}

String* String_Append(String* s1, String* s2) {
	size_t r = s1->len + s2->len + 1;
	char* rr = (char*)calloc(r, sizeof(char));
	rr[0] = 0;
	strcat(rr, s1->base);
	strcat(rr, s2->base);
	return String_NewFromAllocated(rr);
}

String* String_Join(String** s, size_t slen, String* sep) {
	if (slen == 0) { return NULL; }
	size_t sl = s[0]->len;
	for (size_t i = 1; i < slen; i++) {
		sl += sep->len;
		sl += s[i]->len;
	}
	sl += 1;
	char* rr = (char*)calloc(sl, sizeof(char));
	rr[0] = 0;
	strcat(rr, s[0]->base);
	for (size_t i = 1; i < slen; i++) {
		strcat(rr, sep->base);
		strcat(rr, s[i]->base);
	}
	return String_NewFromAllocated(rr);
}

String* String_SubString(String* s, size_t start, size_t end) {
	if (end <= start) { return NULL; }
	if (start >= s->len) { return NULL; }
	char* res = (char*)calloc(end-start+1, sizeof(char));
	res[end-start] = 0;
	for (size_t i = 0; i < end-start; i++) {
		res[i] = s->base[start+i];
	}
	return String_NewFromAllocated(res);
}

void String_Dispose(String *s) {
	if (s->allocated) { free(s->base); }
	free(s);
}

char* String_ToCharArray(String* s) {
	return s->base;
}

