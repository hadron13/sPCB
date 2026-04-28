
/*
 *  copyright © 2026 hadron13
 *
 *  permission is hereby granted, free of charge, to any person obtaining a copy of this software
 *  and associated documentation files (the “software”), to deal in the software without
 *  restriction, including without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the software, and to permit persons to whom the
 *  software is furnished to do so, subject to the following conditions:
 *
 *  the above copyright notice and this permission notice shall be included in all copies or
 *  substantial portions of the software.
 *
 *  the software is provided “as is”, without warranty of any kind, express or implied, including
 *  but not limited to the warranties of merchantability, fitness for a particular purpose and
 *  noninfringement. in no event shall the authors or copyright holders be liable for any claim,
 *  damages or other liability, whether in an action of contract, tort or otherwise, arising
 *  from, out of or in connection with the software or the use or other dealings in the software.
 */


/*
 *  ||      ||     /\     //===\\  ||      ||  ========    /\     ||==\\   ||     ======
 *  ||      ||    //\\    ||       ||      ||     ||      //\\    ||   ||  ||     ||
 *  ||======||   //  \\   \\===\\  ||======||     ||     //  \\   ||==\\   ||     ||===
 *  ||      ||  //====\\        || ||      ||     ||    //====\\  ||   ||  ||     ||
 *  ||      || //      \\ \\===//  ||      ||     ||   //      \\ ||==//   ||==== ======
 *
 *
 *  Usage:
 *  hash32_t my_table = hash32_create(8); // creates table with 2^8 buckets ready to use
 *
 *  hash32_insert(&my_table, 1984, 42);   // inserts value 42 with key 1984, resizes table if needed
 *
 *  hash32_resize(&my_table, 7);          // resizes and rehashes table to new exponent
 *
 *  uint32_t value = hash32_lookup(&my_table, 1984); // lookups value with key 1984
 *
 *  hash32_delete(&my_table, 1984);       // deletes and replaces key with a gravestone
 *
 *  hash32_destroy(&my_table);            // free the table's contents
 *
 *  
 *  for other hashtable types, replace hash32 with hash64 or str_hash
 *  
 *  special note for str_hash: strings are not copied, making sure they live is out of the table's scope. 
 *
 *  note for hash32 and hash64: empty keys are marked with UINT32_MAX and UINT64_MAX respectively, 
 *  and gravestone as UINT32_MAX -1 and UINT64_MAX -1 respectively, therefore do not use these values as keys
 */


// TODO: gravestone counting & rebuilding

#include<stdlib.h>
#include<string.h>
#include<stdint.h>


#ifdef STRINGS_H
typedef struct{
    size_t length;
    size_t exponent;
    size_t tombstones;
    string_t *keys;
    uint64_t *values;
}str_hash_t;
#endif

typedef struct{
    size_t length;
    size_t exponent;
    size_t tombstones;
    uint32_t *keys;
    uint32_t *values;
}hash32_t;

typedef struct{
    size_t length;
    size_t exponent;
    size_t tombstones;
    uint64_t *keys;
    uint64_t *values;
}hash64_t;

#define STR_HASH_MISSING UINT64_MAX
#define HASH32_MISSING UINT32_MAX
#define HASH64_MISSING UINT64_MAX

static int32_t  msi_lookup(uint64_t hash, int exp, int32_t idx);
static uint64_t fnv1a_hash(const unsigned char *data, size_t length);

#ifdef STRINGS_H
static str_hash_t str_hash_create(size_t initial_exponent);
static void       str_hash_resize(str_hash_t *hash_table, size_t new_exponent);
static void       str_hash_insert(str_hash_t *restrict hash_table, string_t key, uint64_t value);
static uint64_t   str_hash_lookup(str_hash_t *restrict hash_table, string_t key);
static uint64_t   str_hash_delete(str_hash_t *restrict hash_table, string_t key);
static void       str_hash_destroy(str_hash_t *hash_table);
static void       str_hash_print(str_hash_t *hash_table);
#endif

static hash32_t   hash32_create(size_t initial_exponent);
static void       hash32_resize(hash32_t *hash_table, size_t new_exponent);
static void       hash32_insert(hash32_t *hash_table, uint32_t key, uint32_t value);
static uint32_t   hash32_lookup(hash32_t *hash_table, uint32_t key);
static uint32_t   hash32_delete(hash32_t *hash_table, uint32_t key);
static void       hash32_destroy(hash32_t *hash_table);

static hash64_t   hash64_create(size_t initial_exponent);
static void       hash64_resize(hash64_t *hash_table, size_t new_exponent);
static void       hash64_insert(hash64_t *hash_table, uint64_t key, uint64_t value);
static uint64_t   hash64_lookup(hash64_t *hash_table, uint64_t key);
static uint64_t   hash64_delete(hash64_t *hash_table, uint64_t key);
static void       hash64_destroy(hash64_t *hash_table);



static int32_t msi_lookup(uint64_t hash, int exp, int32_t idx){
    uint32_t mask = ((uint32_t)1 << exp) - 1;
    uint32_t step = (hash >> (64 - exp)) | 1;
    return (idx + step) & mask;
}

static uint64_t fnv1a_hash(const unsigned char *data, size_t length){
    uint64_t hash = 0xcbf29ce484222325;
    
    for(size_t i = 0; i < length; i++){
        hash ^= data[i];
        hash *= 0x100000001b3;
    }
    return hash;
}

static uint32_t lowbias32(uint32_t x){
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

#ifdef STRINGS_H
static const char *STR_HASH_GRAVESTONE = (const char*)UINTPTR_MAX;

static str_hash_t str_hash_create(size_t initial_exponent){
    size_t initial_size = 1 << initial_exponent;
    str_hash_t hash_table = {
        .length   = 0, 
        .exponent = initial_exponent,
        .tombstones = 0,
        .keys   = calloc(initial_size, sizeof (*hash_table.keys)),
        .values = malloc(initial_size * sizeof (*hash_table.values))
    };

    if(hash_table.keys == NULL || hash_table.values == NULL)
        return (str_hash_t){.exponent = 0, .tombstones = 0, .length = 0, .keys = NULL, .values = NULL};
    
    return hash_table;
}

static void str_hash_resize(str_hash_t *hash_table, size_t new_exponent){
    
    size_t old_size = 1 << hash_table->exponent;

    str_hash_t new_hash = str_hash_create(new_exponent);

    for(size_t i = 0; i < old_size; i++){
        if(hash_table->keys[i].cstr == NULL || hash_table->keys[i].cstr == STR_HASH_GRAVESTONE)
            continue;
        
        str_hash_insert(&new_hash, hash_table->keys[i], hash_table->values[i]);
    }

    *hash_table = new_hash;
}

static void str_hash_insert(str_hash_t *restrict hash_table, string_t key, uint64_t value){    

    if(hash_table->length + 1  == (1 << hash_table->exponent) - ((1 << hash_table->exponent) >> 1) || 
        hash_table->tombstones > hash_table->length >> 2){
        str_hash_resize(hash_table, hash_table->exponent + 1);
    }

    uint64_t hash = fnv1a_hash((unsigned char*)key.cstr, key.length);
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);

        if(hash_table->keys[index].cstr != NULL && hash_table->keys[index].cstr != STR_HASH_GRAVESTONE && !string_equal(hash_table->keys[index], key)){
            continue;
        }

        hash_table->keys[index] = key;
        hash_table->values[index] = value;
        hash_table->length++;
        return;
    }
}

static uint64_t str_hash_lookup(str_hash_t * restrict hash_table, string_t key){
    uint64_t hash = fnv1a_hash((unsigned char*)key.cstr, key.length);
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index].cstr == NULL)
            return STR_HASH_MISSING;
        if(hash_table->keys[index].cstr == STR_HASH_GRAVESTONE)
            continue;

        if(!string_equal(hash_table->keys[index], key)){
            continue;
        }
        
        return hash_table->values[index];
    }
}
#endif


#define HASH32_GRAVESTONE (UINT32_MAX-1)

static hash32_t hash32_create(size_t initial_exponent){
    size_t initial_size = 1 << initial_exponent;
    hash32_t hash_table = {
        .length   = 0, 
        .exponent = initial_exponent,
        .tombstones = 0,
        .keys   = malloc(initial_size * sizeof (*hash_table.keys)),
        .values = malloc(initial_size * sizeof (*hash_table.values))
    };

    memset(hash_table.keys, 0xff, initial_size * sizeof(*hash_table.keys));

    if(hash_table.keys == NULL || hash_table.values == NULL)
        return (hash32_t){0, 0, 0, NULL, NULL};
    
    return hash_table;
}


static void hash32_resize(hash32_t *hash_table, size_t new_exponent){
    size_t old_size = 1 << hash_table->exponent;

    hash32_t new_hash = hash32_create(new_exponent);

    for(size_t i = 0; i < old_size; i++){
        if(hash_table->keys[i] >= HASH32_GRAVESTONE)
            continue;

        hash32_insert(&new_hash, hash_table->keys[i], hash_table->values[i]);    
    }

    *hash_table = new_hash;
}

static void hash32_insert(hash32_t *hash_table, uint32_t key, uint32_t value){
    if(hash_table->length + 1 == (size_t)((1 << hash_table->exponent) * 0.6f) ||
        hash_table->tombstones > hash_table->length >> 2){
        hash32_resize(hash_table, hash_table->exponent + 1);
    }
    
    // uint64_t hash = lowbias32(key);
    
    //Sppoooky, but somehow works
    uint64_t hash = key;
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);

        if(hash_table->keys[index] < HASH32_GRAVESTONE && hash_table->keys[index] != key){
            continue;
        }

        hash_table->keys[index] = key;
        hash_table->values[index] = value;
        hash_table->length++;
        return;
    }
}

static uint32_t hash32_lookup(hash32_t *hash_table, uint32_t key){
    // uint64_t hash = lowbias32(key);
    uint64_t hash = key;
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == UINT32_MAX)
            return HASH32_MISSING;
        if(hash_table->keys[index] != key || hash_table->keys[index] == HASH32_GRAVESTONE)
            continue;
        
        return hash_table->values[index];
    }
}

static uint32_t hash32_delete(hash32_t *hash_table, uint32_t key){
    // uint64_t hash = lowbias32(key);
    uint64_t hash = key;
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == UINT32_MAX)
            return UINT32_MAX;
        if(hash_table->keys[index] != key || hash_table->keys[index] == HASH32_GRAVESTONE)
            continue;
        
        hash_table->keys[index] = HASH32_GRAVESTONE;
        hash_table->tombstones++;
        return hash_table->values[index];
    }
}

static void hash32_destroy(hash32_t *hash_table){
    hash_table->length = 0;
    hash_table->exponent = 0;
    free(hash_table->keys);
    free(hash_table->values);
}


#define HASH64_GRAVESTONE (UINT64_MAX-1)

static hash64_t hash64_create(size_t initial_exponent){
    size_t initial_size = 1 << initial_exponent;
    hash64_t hash_table = {
        .length   = 0, 
        .exponent = initial_exponent,
        .keys   = malloc(initial_size * sizeof (*hash_table.keys)),
        .values = malloc(initial_size * sizeof (*hash_table.values))
    };

    memset(hash_table.keys, 0xff, initial_size * sizeof(*hash_table.keys));

    if(hash_table.keys == NULL || hash_table.values == NULL)
        return (hash64_t){0, 0, 0, NULL, NULL};
    
    return hash_table;
}

static void hash64_resize(hash64_t *hash_table, size_t new_exponent){

    size_t old_size = 1 << hash_table->exponent;

    hash64_t new_hash = hash64_create(new_exponent);

    for(size_t i = 0; i < old_size; i++){
        if(hash_table->keys[i] >= HASH64_GRAVESTONE)
            continue;

        hash64_insert(&new_hash, hash_table->keys[i], hash_table->values[i]);    
    }

    *hash_table = new_hash;
}

static void hash64_insert(hash64_t *hash_table, uint64_t key, uint64_t value){
    if(hash_table->length + 1 == 1 << hash_table->exponent ||
        hash_table->tombstones > hash_table->length >> 2){
        hash64_resize(hash_table, hash_table->exponent + 1);
    }

    uint64_t hash = fnv1a_hash((unsigned char*)&key, sizeof(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);

        if(hash_table->keys[index] < HASH64_GRAVESTONE && hash_table->keys[index] != key){
            continue;
        }

        hash_table->keys[index] = key;
        hash_table->values[index] = value;
        hash_table->length++;
        return;
    }
}

static uint64_t hash64_lookup(hash64_t *hash_table, uint64_t key){
    uint64_t hash = fnv1a_hash((unsigned char*)&key, sizeof(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == UINT64_MAX)
            return HASH64_MISSING;
        if(hash_table->keys[index] != key || hash_table->keys[index] == HASH64_GRAVESTONE)
            continue;
        
        return hash_table->values[index];
    }
}

static uint64_t hash64_delete(hash64_t *hash_table, uint64_t key){
    uint64_t hash = fnv1a_hash((unsigned char*)&key, sizeof(key));
    for(int32_t index = hash;;){
        index = msi_lookup(hash, hash_table->exponent, index);
        
        if(hash_table->keys[index] == UINT64_MAX)
            return 0;
        if(hash_table->keys[index] != key || hash_table->keys[index] == HASH64_GRAVESTONE)
            continue;
        hash_table->keys[index] = HASH64_GRAVESTONE;
        hash_table->tombstones++;
        return hash_table->values[index];
    }
}

static void hash64_destroy(hash64_t *hash_table){
    hash_table->tombstones = 0;
    hash_table->length = 0;
    hash_table->exponent = 0;
    free(hash_table->keys);
    free(hash_table->values);
}

