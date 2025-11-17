#ifndef REGISTRY_H
#define REGISTRY_H

#include <zephyr/kernel.h>
#include <stddef.h>

#ifndef STRINGIFY
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
#endif

typedef struct {
    const char *name;
    float *ptr;
} var_entry;

/*
 * Variable list inclusion.
 * User defines VARIABLE_LIST_FILE (via CMake define forwarded from Kconfig)
 */
#if defined(VARIABLE_LIST_FILE)
    #define VAR_LIST_INCLUDE STRINGIFY(VARIABLE_LIST_FILE)
#else
    #error "You must define VARIABLE_LIST_FILE (use Kconfig or -DVARIABLE_LIST_FILE)!"
#endif

/* Generate enum IDs from the VAR_ENTRY list */
#define VAR_ENTRY(ID, VAR, NAME) ID,
typedef enum {
    #include VAR_LIST_INCLUDE
    VAR_COUNT
} var_id;
#undef VAR_ENTRY

extern var_entry var_registry[VAR_COUNT];

static inline var_entry *registry_get(var_id id) {
    if ((int)id < 0 || (int)id >= VAR_COUNT) return NULL;
    return &var_registry[id];
}

#endif // REGISTRY_H
