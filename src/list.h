/*
 *  Copyright © 2025 hadron13
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 *  and associated documentation files (the “Software”), to deal in the Software without
 *  restriction, including without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all copies or
 *  substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 *  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIST_H
#define LIST_H


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
    size_t size, capacity;
} list_header_t;

#define list_header(list)  ((list_header_t *)(list) - 1)

#define list_size(list)    ((list) ? list_header(list)->size : 0)
#define list_capacity(list)((list) ? list_header(list)->capacity : 0)
#define list_full(list)    (list_size(list) == list_capacity(list))
#define list_init(type)    (list__init(sizeof(type), 1))
#define list_alloc(type, capacity)(list__init(sizeof(type), capacity))
#define list_free(list)    (free(list_header(list)))

#define list_push(list, e) (  (list) =  \
                         list_full(list)? list__resize((list), sizeof(*list),list_capacity(list)*2):\
                                        (list),\
                         (list)[list_header(list)->size++] = (e))

#define list_pop(list)     (  list_header(list)->size--, (list) = (list_size(list) <= list_capacity(list) / 2)?\
                         list__resize((list), sizeof(*list), list_capacity(list)/2) : (list))

#define list_swap_delete(list, i)((list)[i] = (list)[list_size(list) - 1], list_pop(list))

#define list_join(list1, list2) (list1 = list__join(list1, list2, sizeof(*(list1))), assert(sizeof(*(list1))==sizeof(*(list2))) )

#define list_resize(list, s)(list = list__resize(list, sizeof(*(list)), s))

#define list_last(list)((list)[list_size(list) - 1])


static void *list__init(size_t element_size, size_t elements){
    list_header_t *header = malloc(element_size * elements + sizeof(list_header_t));
    if(header == NULL)
        return NULL;
    header->size = 0;
    header->capacity = elements;
    return (void*)(header + 1);
}

static void *list__resize(void *list, size_t element_size, size_t new_ammount) {
    list_header(list)->capacity = new_ammount;
    return (list_header_t*)realloc(list_header(list), (element_size * new_ammount) + sizeof(list_header_t)) + 1;
}

static void *list__join(void *list1, void *list2, size_t element_size){
    size_t total_size = (list_size(list1) + list_size(list2));

    list1 = list__resize(list1, element_size, total_size);
    memcpy( (unsigned char*)list1 + list_size(list1) * element_size, list2, list_size(list2) * element_size);

    list_header(list1)->size += list_size(list2);

    return list1;
}




#endif /*LIST_H*/
