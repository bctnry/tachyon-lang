#include "tachyon.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arraylist.h"
#include "hashtable.h"
#include "auxfuncs.h"
#include "str.h"

void Tachyon__dispose_base(TachyonDatum* d) {
	if (d == NULL) { return; }
	return free(d);
}

void Tachyon_Dispose(TachyonDatum* d) {
	if (d == NULL) { return; }
	switch (d->type) {
	case TACHYON_INT: break;
	case TACHYON_WORD: free(d->v.strval); break;
	case TACHYON_BOOL: break;
	case TACHYON_LIST: {
		for (size_t i = 0; i < ArrayList_Length(d->v.listval.base); i++) {
			TachyonDatum* rr = d->v.listval.base->base[i];
			Tachyon_Dispose(rr);
		}
		ArrayList_Dispose(d->v.listval.base); break;
	}
	}
	Tachyon__dispose_base(d);
	return;
}

// Create a new empty list object.
TachyonDatum* Tachyon_New_EmptyList() {
	TachyonDatum* res = (TachyonDatum*)calloc(1, sizeof(TachyonDatum));
	res->type = TACHYON_LIST;
	res->v.listval.base = ArrayList_Create(4);
	res->v.listval.sealed = 1;
	return res;
}

// Create a new bool object.
TachyonDatum* Tachyon_New_Bool(_Bool val) {
	TachyonDatum* res = (TachyonDatum*)calloc(1, sizeof(TachyonDatum));
	res->type = TACHYON_BOOL;
	res->v.boolval = val;
	return res;
}

// Create a new word object.  This function does not take ownership of the
// original character array.
TachyonDatum* Tachyon_New_Word(char* val) {
	TachyonDatum* res = (TachyonDatum*)calloc(1, sizeof(TachyonDatum));
	res->type = TACHYON_WORD;
	res->v.strval = String_New(val);
	return res;
}

// Create a new integer object.
TachyonDatum* Tachyon_New_Int(int64_t val) {
	TachyonDatum* res = (TachyonDatum*)calloc(1, sizeof(TachyonDatum));
	res->type = TACHYON_INT;
	res->v.intval = val;
	return res;
}

void Tachyon_List_Append(TachyonDatum* list, TachyonDatum* val) {
	ArrayList_Append(list->v.listval.base, val);
}

TachyonDatum* Tachyon_List_Nth(TachyonDatum* list, size_t n) {
	return ArrayList_Nth(list->v.listval.base, n);
}

// Pretty-print a Tachyon Datum to a string.
// Caller should free the string regardless.
char* Tachyon_DatumToString(TachyonDatum* d) {
	switch (d->type) {
	case TACHYON_BOOL: {
		if (d->v.boolval) {
			char* res = (char*)calloc(5, sizeof(char));
			res[0] = 0;
			strcat(res, "TRUE");
			return res;
		} else {
			char* res = (char*)calloc(6, sizeof(char));
			res[0] = 0;
			strcat(res, "FALSE");
			return res;
		}
	}
	case TACHYON_INT: {
		return Tachyon_Aux_Int64ToString(d->v.intval);
	}
	case TACHYON_WORD: {
		char* r = (char*)calloc(d->v.strval->len+1, sizeof(char));
		r[0] = 0;
		strcat(r, d->v.strval->base);
		return r;
	}
	case TACHYON_LIST: {
		ArrayList* arr = ArrayList_Create(2+d->v.listval.base->len);
		ArrayList_Append(arr, (void*)"[");
		for (size_t i = 0; i < ArrayList_Length(d->v.listval.base); i++) {
			TachyonDatum* r = ArrayList_Nth(d->v.listval.base, i);
			char* rstr = Tachyon_DatumToString(r);
			ArrayList_Append(arr, rstr);
		}
		ArrayList_Append(arr, (void*)"]");
		char* res = Tachyon_Aux_StringJoin((const char**)arr->base, arr->len, " ");
		for (size_t i = 1; i < ArrayList_Length(d->v.listval.base) + 1; i++) {
			free(arr->base[i]);
		}
		ArrayList_Dispose(arr);
		return res;
	}
	}
}

TachyonDatum* Tachyon_EnvPage_Lookup(TachyonEnvPage* l, char* str) {
	for (size_t i = 0; i < l->len; i++) {
		HashTable* p = l->base[i];
		TachyonDatum* r = HashTable_Get(p, str);
		if (r != NULL) { return r; }
	}
	return NULL;
}
void Tachyon_EnvPage_Enter(TachyonEnvPage* l) {
	HashTable* p = HashTable_Create(16);
	ArrayList_Append(l, p);
}
void Tachyon_EnvPage__free_elem(const char* k, void* v) {
	Tachyon_Dispose((TachyonDatum*)v);
}
void Tachyon_EnvPage_Leave(TachyonEnvPage* l) {
	HashTable* r = ArrayList_Pop(l);
	HashTable_ForEach(r, Tachyon_EnvPage__free_elem);
	HashTable_Dispose(r);
}
void Tachyon_Env_Enter(TachyonEnv *env) {
	Tachyon_EnvPage_Enter(env->env_page);
}
void Tachyon_Env_Leave(TachyonEnv* env) {
	Tachyon_EnvPage_Leave(env->env_page);
}
void Tachyon_EnvPage_Insert(TachyonEnvPage* l, char* key, TachyonDatum* val) {
	HashTable* head = ArrayList_Head(l);
	HashTable_Put(head, key, val);
}
void Tachyon_EnvPage__free_page(size_t i, void* v) {
	HashTable_ForEach(v, Tachyon_EnvPage__free_elem);
	HashTable_Dispose(v);
}
void Tachyon_EnvPage_Dispose(TachyonEnvPage* l) {
	ArrayList_ForEach(l, Tachyon_EnvPage__free_page);
}

TachyonEnv* Tachyon_Env_New() {
	TachyonEnv* res = (TachyonEnv*)calloc(1, sizeof(TachyonEnv));
	TachyonEnvPage* envstack = ArrayList_Create(4);
	Tachyon_EnvPage_Enter(envstack);
	res->state = TACHYON_STATE_NORMAL;
	res->escape_count = 0;
	res->env_page = envstack;
	res->stack = ArrayList_Create(4);
	return res;
}
void Tachyon_Env_Insert(TachyonEnv* env, char* key, TachyonDatum* val) {
	Tachyon_EnvPage_Insert(env->env_page, key, val);
}
TachyonDatum* Tachyon_Env_Lookup(TachyonEnv* env, char* key) {
	return Tachyon_EnvPage_Lookup(env->env_page, key);
}
HashTable* Tachyon_Env_CurrentEnvPage(TachyonEnv* env) {
	return ArrayList_Head(env->env_page);
}
					
void Tachyon_Env_Dispose(TachyonEnv* env) {
	Tachyon_EnvPage_Dispose(env->env_page);
	ArrayList_Dispose(env->stack);
	free(env);
}
TachyonDatum* Tachyon_Env_StackTop(TachyonEnv* env) {
	return ArrayList_Head(env->stack);
}
TachyonDatum* Tachyon_Env_NthFromTop(TachyonEnv* env, size_t n) {
	return ArrayList_Nth(env->stack, ArrayList_Length(env->stack)-(n+1));
}
void Tachyon_Env_PushStack(TachyonEnv* env, TachyonDatum* d) {
	ArrayList_Append(env->stack, d);
}
TachyonDatum* Tachyon_Env_PopStack(TachyonEnv* env) {
	return ArrayList_Pop(env->stack);
}

void dump_stack(TachyonEnv* env) {
	for (size_t i = 0; i < env->stack->len; i++) {
		char* s = Tachyon_DatumToString(env->stack->base[i]);
		printf("%s\n", s);
		free(s);
	}
}

void dump_env_2(const char* k, void* v) {
	char* r = Tachyon_DatumToString(v);
	printf("<%s:%s>\n", k, r);
	free(r);
}
void dump_env_1(size_t i, void* elem) {
	HashTable_ForEach(elem, dump_env_2);
}
void dump_env(TachyonEnvPage *env) { ArrayList_ForEach(env, dump_env_1); }

HashTable *tachyon_primitive_table = NULL;

void Tachyon_Initialize() {
	tachyon_primitive_table = HashTable_Create(16);
}
void Tachyon_Teardown() {
	HashTable_Dispose(tachyon_primitive_table);
}
void Tachyon_RegisterPrimitive(char *key, void (*f)(TachyonEnv *env)) {
	HashTable_Put(tachyon_primitive_table, key, f);
}
void* Tachyon__primitive_lookup(const char* key) {
	if (tachyon_primitive_table == NULL) { return NULL; }
	return HashTable_Get(tachyon_primitive_table, key);
}

void Tachyon_ExecList(TachyonEnv *env, TachyonDatum *d);

_Bool Tachyon__resolve_to_bool(TachyonEnv* env, TachyonDatum* d) {
	if (d == NULL) { return 0; }
	switch (d->type) {
	case TACHYON_LIST: {
		Tachyon_ExecList(env, d);
		return Tachyon__resolve_to_bool(env, Tachyon_Env_StackTop(env));
	}
	case TACHYON_INT:
		return d->v.intval > 0;
	case TACHYON_BOOL:
		return d->v.boolval;
	case TACHYON_WORD:
		return d->v.strval->len > 0;
	}
}

// Execute a word with specified environment.
// All memory used by the datum will not be freed - the caller needs
// to free it themselves.
void Tachyon_Exec(TachyonEnv* env, TachyonDatum* d) {
	if (env->state == TACHYON_STATE_ESCAPE) {
		switch (d->type) {
		case TACHYON_WORD: {
			if (strcmp(d->v.strval->base, "]") == 0) {
				TachyonDatum* h = Tachyon_Env_StackTop(env);
				h->v.listval.sealed = 1;
				size_t stklen = ArrayList_Length(env->stack);
				if (stklen >= 2) {
					TachyonDatum* h2 = Tachyon_Env_NthFromTop(env, 1);
					if (h2->type == TACHYON_LIST && !h2->v.listval.sealed) {
						Tachyon_Env_PopStack(env);
						Tachyon_List_Append(h2, h);
					}
				}
				env->escape_count -= 1;
				
				if (env->escape_count <= 0) {
					env->state = TACHYON_STATE_NORMAL;
				}
			} else if (strcmp(d->v.strval->base, "[") == 0) {
				TachyonDatum* l = Tachyon_New_EmptyList();
				l->v.listval.sealed = 0;
				Tachyon_Env_PushStack(env, l);
				env->escape_count += 1;
			} else {
				TachyonDatum* l = Tachyon_Env_StackTop(env);
				ArrayList_Append(l->v.listval.base, d);
			}
			break;
		}
		default: {
			TachyonDatum* l = Tachyon_Env_StackTop(env);
			ArrayList_Append(l->v.listval.base, d);
			break;
		}
		}
		return;
	}
	switch (d->type) {
	case TACHYON_WORD: {
		if (strcmp(d->v.strval->base, "[") == 0) {
			TachyonDatum* l = Tachyon_New_EmptyList();
			l->v.listval.sealed = 0;
			Tachyon_Env_PushStack(env, l);
			env->escape_count += 1;
			env->state = TACHYON_STATE_ESCAPE;
		} else if (strcmp(d->v.strval->base, "]") == 0) {
			// TODO: panic: not in escape mode.
		} else if (strcmp(d->v.strval->base, "#") == 0) {
			TachyonDatum* value_datum = Tachyon_Env_PopStack(env);
			TachyonDatum* name_datum = Tachyon_Env_PopStack(env);
			Tachyon_Env_Insert(env, name_datum->v.strval->base, value_datum);
		} else if (strcmp(d->v.strval->base, "$") == 0) {
			TachyonDatum* name = Tachyon_Env_PopStack(env);
			TachyonDatum* l = Tachyon_Env_Lookup(env, name->v.strval->base);
			Tachyon_Env_PushStack(env, l);
		} else if (strcmp(d->v.strval->base, "begin") == 0) {
			Tachyon_Env_Enter(env);
		} else if (strcmp(d->v.strval->base, "end") == 0) {
			Tachyon_Env_Leave(env);
		} else if (strcmp(d->v.strval->base, "TRUE") == 0) {
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(1));
		} else if (strcmp(d->v.strval->base, "FALSE") == 0) {
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(0));
		} else if (strcmp(d->v.strval->base, "drop") == 0) {
			Tachyon_Dispose(Tachyon_Env_PopStack(env));
		} else if (strcmp(d->v.strval->base, "swap") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, a);
			Tachyon_Env_PushStack(env, b);
		} else if (strcmp(d->v.strval->base, "dup") == 0) {
			Tachyon_Env_PushStack(env, Tachyon_Env_StackTop(env));
		} else if (strcmp(d->v.strval->base, "over") == 0) {
			Tachyon_Env_PushStack(env, Tachyon_Env_NthFromTop(env, 1));
		} else if (strcmp(d->v.strval->base, "rot3") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			TachyonDatum* c = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, b);
			Tachyon_Env_PushStack(env, a);
			Tachyon_Env_PushStack(env, c);
		} else if (strcmp(d->v.strval->base, "inc") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Int(a->v.intval+1));
		} else if (strcmp(d->v.strval->base, "dec") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Int(a->v.intval-1));
		} else if (strcmp(d->v.strval->base, "+") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Int(a->v.intval+b->v.intval));
			Tachyon_Dispose(a);
			Tachyon_Dispose(b);
		} else if (strcmp(d->v.strval->base, "*") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Int(a->v.intval*b->v.intval));
		} else if (strcmp(d->v.strval->base, "-") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Int(b->v.intval-a->v.intval));
		} else if (strcmp(d->v.strval->base, "/") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Int(b->v.intval/(a->v.intval)));
		} else if (strcmp(d->v.strval->base, "MOD") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Int(b->v.intval%(a->v.intval)));
		} else if (strcmp(d->v.strval->base, "<=") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(b->v.intval<=a->v.intval));
		} else if (strcmp(d->v.strval->base, ">=") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(b->v.intval>=a->v.intval));
		} else if (strcmp(d->v.strval->base, "==") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(a->v.intval==b->v.intval));
		} else if (strcmp(d->v.strval->base, "!=") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(a->v.intval!=b->v.intval));
		} else if (strcmp(d->v.strval->base, "AND") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(a->v.boolval&&b->v.boolval));
		} else if (strcmp(d->v.strval->base, "OR") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			TachyonDatum* b = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(a->v.boolval||b->v.boolval));
		} else if (strcmp(d->v.strval->base, "NOT") == 0) {
			TachyonDatum* a = Tachyon_Env_PopStack(env);
			Tachyon_Env_PushStack(env, Tachyon_New_Bool(!a->v.boolval));
		} else if (strcmp(d->v.strval->base, "if") == 0) {
			TachyonDatum* elsec = Tachyon_Env_PopStack(env);
			TachyonDatum* thenc = Tachyon_Env_PopStack(env);
			TachyonDatum* cond = Tachyon_Env_PopStack(env);
			Tachyon_ExecList(env, cond->v.boolval? thenc : elsec);
		} else if (strcmp(d->v.strval->base, "while") == 0) {
			TachyonDatum* body = Tachyon_Env_PopStack(env);
			TachyonDatum* cond = Tachyon_Env_PopStack(env);
			while (Tachyon__resolve_to_bool(env, cond)) {
				Tachyon_ExecList(env, body);
			}
		} else if (strcmp(d->v.strval->base, "dstk") == 0) {
			printf("exec: dstk\n");
			dump_stack(env);
		} else if (strcmp(d->v.strval->base, "denv") == 0) {
			printf("exec: denv\n");
			dump_env(env->env_page);
		} else if (d->v.strval->base[0] == '\'') {
			String* s = String_SubString(d->v.strval, 1, d->v.strval->len);
			TachyonDatum *a = Tachyon_New_Word(s->base);
			Tachyon_Env_PushStack(env, a);
			String_Dispose(s);
		} else {
			void(*f)(TachyonEnv*env) = Tachyon__primitive_lookup(d->v.strval->base);
			if (f != NULL) {
				f(env);
			} else {
				TachyonDatum* dd = Tachyon_Env_Lookup(env, d->v.strval->base);
				if (dd == NULL) { break; }
				Tachyon_ExecList(env, dd);
			}
		}
		break;
	}
	default: {
		Tachyon_Env_PushStack(env, d);
		break;
	}
	}
}

void Tachyon_ExecList(TachyonEnv* env, TachyonDatum* d) {
	for (size_t i = 0; i < d->v.listval.base->len; i++) {
		char* r = Tachyon_DatumToString(d->v.listval.base->base[i]);
		Tachyon_Exec(env, d->v.listval.base->base[i]);
		free(r);
	}
}

