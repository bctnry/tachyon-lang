#include "hashtable.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Jenkin's "one_at_a_time" hash function.
uint32_t bytestring_hash(const char* key, size_t length) {
	size_t i = 0;
	uint32_t res = 0;
	while (i < length) {
		res += key[i];
		res += res << 10;
		res ^= res >> 6;
		i += 1;
	}
	res += res << 3;
	res ^= res >> 11;
	res += res << 15;
	return res;
}


HashTable* HashTable_Create(size_t bucket_count) {
	if (bucket_count == 0) { bucket_count = 16; }
	HashTable* res = (HashTable*)calloc(1, sizeof(HashTable));
	res->chain_array = (HashTablePiece**)calloc(bucket_count, sizeof(HashTablePiece*));
	res->bucket_cnt = bucket_count;
	return res;
}

void HashTable_Dispose(HashTable* t) {
	for (size_t i = 0; i < t->bucket_cnt; i++) {
		HashTablePiece* subj = t->chain_array[i];
		while (subj != NULL) {
			HashTablePiece* next = subj->next;
			free(subj);
			subj = next;
		}
	}
	free(t->chain_array);
	free(t);
}

size_t HashTable__hash(HashTable* t, uint32_t first_hash) {
	return first_hash % (t->bucket_cnt);
}

void* HashTable_Get(HashTable* t, const char* key) {
	uint32_t khash = bytestring_hash(key, strlen(key));
	HashTablePiece* head = t->chain_array[HashTable__hash(t, khash)];
	if (head == NULL) { return NULL; }
	while (head != NULL) {
		if (strcmp(head->key, key) == 0) { return head->value; }
		head = head->next;
	}
	return NULL;
}

void HashTable_Put(HashTable* t, char* key, void* value) {
	uint32_t shash = bytestring_hash(key, strlen(key));
	size_t h = HashTable__hash(t, shash);
	if (t->chain_array[h] != NULL) {
		HashTablePiece* subj = t->chain_array[h];
		_Bool found = 0;
		while (subj != NULL) {
			if (strcmp(subj->key, key) ==0) { found = 1; break; }
			subj = subj->next;
		}
		if (found) {
			subj->value = value;
		} else {
			HashTablePiece* p = (HashTablePiece*)calloc(1, sizeof(HashTablePiece));
			p->key = key;
			p->value = value;
			p->next = t->chain_array[h];
			t->chain_array[h] = p;
		}
	} else {
		HashTablePiece* p = (HashTablePiece*)calloc(1, sizeof(HashTablePiece));
		p->key = key;
		p->value = value;
		p->next = t->chain_array[h];
		t->chain_array[h] = p;
	}
}

void HashTable_Delete(HashTable* t, const char* key) {
	size_t h = HashTable__hash(t, bytestring_hash(key, strlen(key)));
	HashTablePiece* p = t->chain_array[h];
	HashTablePiece* prev = NULL;
	if (p != NULL && strcmp(p->key, key) == 0) {
		t->chain_array[h] = p->next;
		free(p);
		return;
	}
	while (p != NULL) {
		if (key == p->key) {
			if (prev != NULL) { prev->next = p->next; }
			free(p);
			return;
		}
		prev = p;
		p = p->next;
	}
}

void HashTable_ForEach(HashTable* t, void (*f)(const char* key, void* value)) {
	for (size_t i = 0; i < t->bucket_cnt; i++) {
		HashTablePiece* subj = t->chain_array[i];
		while (subj != NULL) {
			f(subj->key, subj->value);
			subj = subj->next;
		}
	}
}

