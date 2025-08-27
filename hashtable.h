#ifndef TACHYON__BASIS_HASHTABLE
#define TACHYON__BASIS_HASHTABLE
#include <stddef.h>

// Separate chaining.

// key: owned & freed by hash table.
// value: owned & freed by others. lend to hash table. freed by others.
// pieces: owned & freed by hash table.
typedef struct HashTablePiece {
	char* key;
	void* value;
	struct HashTablePiece* next;
} HashTablePiece;

// chain_array: owned by hash table. freed by hash table.
typedef struct HashTable {
	size_t bucket_cnt;
	HashTablePiece** chain_array;
} HashTable;

HashTable *HashTable_Create(size_t bucket_count);

void HashTable_Dispose(HashTable* t);
void* HashTable_Get(HashTable* t, const char* key);
void HashTable_Put(HashTable* t, char* key, void* value);
void HashTable_Delete(HashTable* t, const char* key);
void HashTable_ForEach(HashTable *t, void (*f)(const char *key, void *value));

#endif


