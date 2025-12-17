#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stddef.h>
#include <log.h>

#ifdef DA_DBG
#define DA_FMT(level, message, ...) logg(level, message, __VA_ARGS__)
#else
#define DA_FMT(...) ((void)0)
#endif

#define CREATE_DYNAMIC_ARRAY(ty)                \
    typedef struct {                            \
        ty * values;                            \
        size_t count;                           \
        size_t capacity;                        \
    } DynamicArray_##ty

#define da_append(array, elt)                                           \
    do {                                                                \
        if ((array)->values == NULL) {                                  \
            (array)->capacity = 5;                                      \
            (array)->values = malloc((array)->capacity * sizeof(elt));  \
            DA_FMT(WARNING, "[ DYNAMIC_ARRAY ] ALLOC BECAUSE NULL. ACTUAL SIZE IS %ld", (array)->capacity); \
        }                                                               \
        if ((array)->count >= (array)->capacity) {                      \
            (array)->capacity *= 2;                                     \
            (array)->values = realloc((array)->values, (array)->capacity * sizeof(elt)); \
            DA_FMT(WARNING, "[ DYNAMIC_ARRAY ] REALLOC BECAUSE FULL. NEW SIZE IS %ld", (array)->capacity); \
        }                                                               \
        (array)->values[(array)->count++] = elt;                        \
    } while(0)

#define da_free(array)                          \
    do {                                        \
        free((array)->values);                  \
    } while(0)

#define NewArray() {0}

CREATE_DYNAMIC_ARRAY(int);

#endif
