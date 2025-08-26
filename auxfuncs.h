#ifndef TACHYON__AUXFUNCS
#define TACHYON__AUXFUNCS

#include <stddef.h>
#include <stdint.h>

char* Tachyon_Aux_Int64ToString(int64_t n);
char *Tachyon_Aux_StringJoin(const char **source, size_t source_len,
                             const char *sep);
char *Tachyon_Aux_Substring(const char *source, size_t start, size_t end);

char* Tachyon_Aux_StringCopy(const char* source);

#endif

