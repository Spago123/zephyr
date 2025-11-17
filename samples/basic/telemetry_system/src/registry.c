#include "registry.h"

/* Declare externs */
#define VAR_ENTRY(ID, VAR, NAME) extern float VAR;
#include VAR_LIST_INCLUDE
#undef VAR_ENTRY

/* Construct the registry array */
#define VAR_ENTRY(ID, VAR, NAME) { NAME, &VAR },
var_entry var_registry[VAR_COUNT] = {
    #include VAR_LIST_INCLUDE
};
#undef VAR_ENTRY
