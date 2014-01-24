#ifndef _FENICHEL_YOCHELSON_H
#define _FENICHEL_YOCHELSON_H

#include "shared.h"

/**
 * Allocate a single cell of memory if possible: if there is no memory
 * available, initiate a garbage collection, and try again. If there
 * just isn't enough memory, return NULL.
 */
cell* alloc(void);

#endif
