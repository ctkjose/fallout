//
//  langCommon.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#include "langCommon.h"

char * strClone(char *a){
	if(!a) return NULL;
	
	char *b = strdup(a);
	return b;
	
	/*size_t sz = strlen(a);
	if(sz <= 0) sz = 0;
	
	char *b = (char *) malloc((sz + 1));
	if(!b) return NULL;
			   
	strcpy(b, a);
	return b;*/
}

///Returns the number of characters copied.
///
///Copies up to `size` characters from `src` to `dest`. `dest` is ereased before the copy.
///
///Parameters:
///`size` is the maximun numbers of characters to copy including the NULL terminator
int strSafeCopy(char *dest, int size, char *src) {
    if(!dest) return 0;
    memset(dest,0, size);
    if (size <= 0) return 0;

    size_t len = (int)strlen(src);
    
    if (len >= size) len = size - 1;
    memcpy(dest, src, len);
    dest[len+1] = '\0';
    
    return (int) len;
}

//
int strCopy(char *a, char *b){
	if(!a) return 0;
	if(b){
		free(b);
	}
	
	b = malloc(sizeof(char) * (strlen(a) + 1));
	if(!b) return 0;
	
	strcpy(b, a);
	free(b);
	return 1;
}

/*out of memory check*/
void *oom(void *ptr){
	if (!ptr) {
		langLogMessage("Runtime Error: unable to allocate memory.");
		abort();
	}
	return ptr;
}


char * langCreateID(const char *prefix, int sz){
	
	char *id = malloc(sizeof(char) * sz+1);
	
	time_t result = time(NULL);
	srand((int) result);
	int num=0;
	int i = 0;
	int a = 0;
	int mz = sz-1;
	
	for(i=0;i < strlen(prefix);i++){
		id[i] = prefix[i];
	}
	
	a = (i+4) > mz ? mz : i+4;
	
	for(i=i;i<a;i++){
		if(i == sz) break;
		num = (rand() % (91 - 65) + 65);
		id[i] = (char) num;
	}
	
	for(i=i;i<=mz;i++){
		if(i == sz) break;
		num = (rand() % (58 - 48) + 48);
		id[i] = (char) num;
	}
	
	id[sz]=0;
	return id;
}



#pragma HASH-TABLE

#define DICT_INITIAL_CAPACITY 16  // must not be zero




hashDictionary* dictionaryCreate(void) {
	// Allocate space for hash table struct.
	hashDictionary* table = malloc(sizeof(hashDictionary));
	if (table == NULL) {
		return NULL;
	}
	table->length = 0;
	table->capacity = DICT_INITIAL_CAPACITY;

	// Allocate (zero'd) space for entry buckets.
	table->entries = calloc(table->capacity, sizeof(hashItem));
	if (table->entries == NULL) {
		free(table); // error, free table before we return!
		return NULL;
	}
	return table;
}

void dictionaryFree(hashDictionary * table) {
	// First free allocated keys.
	for (size_t i = 0; i < table->capacity; i++) {
		if (table->entries[i].key != NULL) {
			free((void*)table->entries[i].key);
		}
	}

	// Then free entries array and table itself.
	free(table->entries);
	free(table);
}


// Return 64-bit FNV-1a hash for key (NUL-terminated)
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

uint64_t dictionaryComputeKey(const char* key) {
	uint64_t hash = FNV_OFFSET;
	for (const char* p = key; *p; p++) {
		hash ^= (uint64_t)(unsigned char)(*p);
		hash *= FNV_PRIME;
	}
	return hash;
}

void* dictionaryGet(hashDictionary* table, const char* key) {
	// AND hash with capacity-1 to ensure it's within entries array.
	uint64_t hash = dictionaryComputeKey(key);
	size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

	// Loop till we find an empty entry.
	while (table->entries[index].key != NULL) {
		if (strcmp(key, table->entries[index].key) == 0) {
			// Found key, return value.
			return table->entries[index].value;
		}
		// Key wasn't in this slot, move to next (linear probing).
		index++;
		if (index >= table->capacity) {
			// At end of entries array, wrap around.
			index = 0;
		}
	}
	return NULL;
}

// Internal function to set an entry (without expanding table).
static const char* dictionary_set_entry(hashItem* entries, size_t capacity, const char* key, void* value, size_t* plength) {
	// AND hash with capacity-1 to ensure it's within entries array.
	uint64_t hash = dictionaryComputeKey(key);
	size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

	// Loop till we find an empty entry.
	while (entries[index].key != NULL) {
		if (strcmp(key, entries[index].key) == 0) {
			// Found key (it already exists), update value.
			entries[index].value = value;
			return entries[index].key;
		}
		// Key wasn't in this slot, move to next (linear probing).
		index++;
		if (index >= capacity) {
			// At end of entries array, wrap around.
			index = 0;
		}
	}

	// Didn't find key, allocate+copy if needed, then insert it.
	if (plength != NULL) {
		key = strdup(key);
		if (key == NULL) {
			return NULL;
		}
		(*plength)++;
	}
	entries[index].key = (char*)key;
	entries[index].value = value;
	return key;
}

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
static int dictionaryExpand(hashDictionary* table) {
	// Allocate new entries array.
	size_t new_capacity = table->capacity * 2;
	if (new_capacity < table->capacity) {
		return 0;  // overflow (capacity would be too big)
	}
	hashItem* new_entries = calloc(new_capacity, sizeof(hashItem));
	if (new_entries == NULL) {
		return 0;
	}

	// Iterate entries, move all non-empty ones to new table's entries.
	for (size_t i = 0; i < table->capacity; i++) {
		hashItem entry = table->entries[i];
		if (entry.key != NULL) {
			dictionary_set_entry(new_entries, new_capacity, entry.key, entry.value, NULL);
		}
	}

	// Free old entries array and update this table's details.
	free(table->entries);
	table->entries = new_entries;
	table->capacity = new_capacity;
	return 1;
}

const char* dictionarySet(hashDictionary* table, const char* key, void* value) {
	if (value == NULL) {
		return NULL;
	}

	// If length will exceed half of current capacity, expand it.
	if (table->length >= table->capacity / 2) {
		if (!dictionaryExpand(table)) {
			return NULL;
		}
	}

	// Set entry and update length.
	return dictionary_set_entry(table->entries, table->capacity, key, value,
						&table->length);
}

size_t dictionaryLength(hashDictionary* table) {
	if(!table) return 0;
	return table->length;
}

langDictIterator dictionaryGetIterator(hashDictionary* table) {
	langDictIterator it;
	it._table = table;
	it._index = 0;
	return it;
}

int dictionaryNext(langDictIterator* it) {
	// Loop till we've hit end of entries array.
	hashDictionary* table = it->_table;
	while (it->_index < table->capacity) {
		size_t i = it->_index;
		it->_index++;
		if (table->entries[i].key != NULL) {
			// Found next non-empty item, update iterator key and value.
			hashItem entry = table->entries[i];
			it->key = entry.key;
			it->value = entry.value;
			return 1;
		}
	}
	return 0;
}
