#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

typedef struct{
    uint8_t *buffer;
    size_t capacity;
    size_t pointer;
}arena_t;


arena_t arena_init(size_t capacity){
    void *buf = malloc(capacity);
    if(buf == NULL)
        return (arena_t){.buffer = NULL};
    
    return (arena_t){
        .buffer = buf,
        .capacity = capacity,
        .pointer = 0
    };
}

int arena_grow(arena_t *arena, size_t new_capacity){
    void *new_buffer = realloc(arena->buffer, new_capacity);
    if(new_buffer){
        arena->buffer = new_buffer;
        arena->capacity = new_capacity;
        return 0;
    }
    return -1;
}

void *arena_allocate(arena_t *arena, size_t size){
    if(arena->pointer + size > arena->capacity){
        if(arena_grow(arena, arena->capacity * 2) == -1){
            return NULL;
        }
    }

    void *buf = arena->buffer + arena->pointer;
    arena->pointer += size + (size % 8)?( 8 - size % 8 ) : 0;
    return buf;
}

void arena_clear(arena_t *arena){
    arena->pointer = 0;
}

void arena_free(arena_t *arena){
    free(arena->buffer);
    arena->capacity = 0;
}

void test_arena(){

    arena_t minha_arena = arena_init(16 * sizeof(int));
    
    int *bogolais = arena_allocate(&minha_arena, 3 * sizeof(int));

    bogolais[0] = 42;
    bogolais[1] = 67;
    bogolais[2] = 37;

    char *bello = "Bello uorld";
    char *string = arena_allocate(&minha_arena, sizeof(bello));
    strcpy(string, bello);

    printf("%i %i %i\n", bogolais[0], bogolais[1], bogolais[2]);
    printf("%s", string);

    arena_clear(&minha_arena);
}