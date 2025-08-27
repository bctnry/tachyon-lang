#include "str.h"
#include <stdlib.h>
#include <string.h>

String* String_New(const char* s) {
	String* res = (String*)calloc(1, sizeof(String));
	size_t l = strlen(s);
	char* base = (char*)calloc(l, sizeof(char));
	base[0] = 0;
	strcat(base, s);
	res->base = base;
	res->len = l;
	return res;
}

String* String__newWithOwnedBuffer(char* s, size_t l) {
	String* res = (String*)calloc(1, sizeof(String));
	res->base = s;
	res->len = l;
	return res;
}

String* String_Append(String* s1, String* s2) {
	size_t r = s1->len + s2->len + 1;
	char* rr = (char*)calloc(r, sizeof(char));
	rr[0] = 0;
	strcat(rr, s1->base);
	strcat(rr, s2->base);
	return String__newWithOwnedBuffer(rr, r);
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
	return String__newWithOwnedBuffer(rr, sl);
}

String* String_SubString(String* s, size_t start, size_t end) {
	if (end <= start) { return NULL; }
	if (start >= s->len) { return NULL; }
	size_t len = end-start;
	char* res = (char*)calloc(len+1, sizeof(char));
	res[end-start] = 0;
	for (size_t i = 0; i < len; i++) {
		res[i] = s->base[start+i];
	}
	return String__newWithOwnedBuffer(res, len);
}

void String_Dispose(String *s) {
	free(s->base);
	free(s);
}

char* String_CloneToCharArray(String* s) {
	char* res = (char*)calloc(s->len, sizeof(char));
	res[0] = 0;
	strcat(res, s->base);
	return res;
}

