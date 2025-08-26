#include <stdlib.h>
#include <string.h>
#include "auxfuncs.h"

char* Tachyon_Aux_Int64ToString(int64_t n) {
	_Bool neg = n < 0;
	int64_t subj = neg? -n : n;
	int count = 0;
	if (subj == 0) { count = 1; }
	else {
		while (subj > 0) { count += 1; subj /= 10; }
		if (neg) { count += 1; }
	}
	char* res = (char*)calloc(count+1, sizeof(char));
	// -1234  // count = 5; allocate 6; res[count] = 0
	res[count] = 0;
	subj = neg? -n : n;
	if (subj == 0) { res[0] = '0'; return res; }
	size_t i = count-1;
	while (subj > 0) {
		res[i] = '0' + subj%10;
		subj /= 10;
		i -= 1;
	}
	if (neg) { res[0] = '-'; }
	return res;
}
char* Tachyon_Aux_StringJoin(const char** source, size_t source_len, const char* sep) {
	if (source_len <= 0) { return NULL; }
	size_t r = strlen(source[0]);
	size_t seplen = strlen(sep);
	for (size_t i = 1; i < source_len; i++) {
		r += seplen;
		r += strlen(source[i]);
	}
	r += 1;
	char* res = (char*)calloc(r, sizeof(char));
	res[0] = 0;
	strcat(res, source[0]);
	for (size_t i = 1; i < source_len; i++) {
		strcat(res, sep);
		strcat(res, source[i]);
	}
	return res;
}
char* Tachyon_Aux_Substring(const char* source, size_t start, size_t end) {
	if (end <= start) { return NULL; }
	char* res = (char*)calloc(end-start+1, sizeof(char));
	res[end-start] = 0;
	for (size_t i = 0; i < end-start; i++) {
		res[i] = source[start+i];
	}
	return res;
}
char* Tachyon_Aux_StringCopy(const char* source) {
	char* res = (char*)calloc(strlen(source), sizeof(char));
	res[0] = 0;
	strcat(res, source);
	return res;
}

