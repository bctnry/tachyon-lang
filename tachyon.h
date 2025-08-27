#ifndef TACHYON__MAIN
#define TACHYON__MAIN

#include "arraylist.h"
#include "hashtable.h"
#include "auxfuncs.h"
#include "str.h"


typedef enum TachyonDatumType {
	TACHYON_INT = 1,
	TACHYON_WORD = 2,
	TACHYON_BOOL = 3,
	TACHYON_LIST = 4,
} TachyonDatumType;

typedef struct TachyonDatum {
	TachyonDatumType type;
	union {
		int64_t intval;
		String* strval;
		_Bool boolval;
		struct {
			ArrayList* base;
			_Bool sealed;
		} listval;
	} v;
} TachyonDatum;

// Dispose a TachyonDatum.
void Tachyon_Dispose(TachyonDatum* d);

// Create a new empty list object.
TachyonDatum* Tachyon_New_EmptyList();

// Create a new bool object.
TachyonDatum* Tachyon_New_Bool(_Bool val);

// Create a new word object.  This function does not take ownership of the
// original character array.
TachyonDatum* Tachyon_New_Word(char* val);

// Create a new integer object.
TachyonDatum* Tachyon_New_Int(int64_t val);

// Put a TachyonDatum into a list.
void Tachyon_List_Append(TachyonDatum* list, TachyonDatum* val);

// Retrieve the nth element of a list TachyonDatum (starts from 0)
TachyonDatum* Tachyon_List_Nth(TachyonDatum* list, size_t n);

// Pretty-print a Tachyon Datum to a string. The resulting memory is
// owned by the caller, who thus is responsible for freeing it.
char* Tachyon_DatumToString(TachyonDatum* d);

typedef enum TachyonEnvState {
  TACHYON_STATE_NORMAL = 1,
  TACHYON_STATE_ESCAPE = 2,
} TachyonEnvState;

typedef ArrayList TachyonEnvPage;

TachyonDatum* Tachyon_EnvPage_Lookup(TachyonEnvPage* l, char* str);

typedef struct TachyonEnv {
	TachyonEnvState state;
	int escape_count;
	TachyonEnvPage* env_page;
	ArrayList* stack;
} TachyonEnv;

// Create a new environment.
TachyonEnv *Tachyon_Env_New();

// Create a new environment scope.
void Tachyon_Env_Enter(TachyonEnv *env);

// Leave an environment scope.
void Tachyon_Env_Leave(TachyonEnv *env);

// Insert a word with its definition into the current environment.
void Tachyon_Env_Insert(TachyonEnv *env, char *key, TachyonDatum *val);

// Retrieve a definition w/ the specified name in the current environment.
TachyonDatum *Tachyon_Env_Lookup(TachyonEnv *env, char *key);

// Dispose all memories used by the environment.
void Tachyon_Env_Dispose(TachyonEnv *env);

TachyonDatum* Tachyon_Env_StackTop(TachyonEnv* env);
TachyonDatum *Tachyon_Env_NthFromTop(TachyonEnv *env, size_t n);

// Push the datum to the top of the stack of the specified environment.
void Tachyon_Env_PushStack(TachyonEnv *env, TachyonDatum *d);

// Pop the datum from the top of the stack of the specified environment.
TachyonDatum* Tachyon_Env_PopStack(TachyonEnv* env);

// Execute a list TachyonDatum. Each word within the datum is executed
// in order.
void Tachyon_ExecList(TachyonEnv *env, TachyonDatum *d);

// Execute a word with specified environment.
// All memory used by the datum will not be freed - the caller needs
// to free it themselves.
void Tachyon_Exec(TachyonEnv *env, TachyonDatum *d);

// Initialize the language.
// `Tachyon_RegisterPrimitive` will NOT work if this is not executed before hand.
void Tachyon_Initialize();
void Tachyon_RegisterPrimitive(char *key, void (*f)(TachyonEnv *env));

void Tachyon_Teardown();

#endif

