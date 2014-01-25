#ifndef _FENICHEL_YOCHELSON_H
#define _FENICHEL_YOCHELSON_H

#include "shared.h"

/**
 * Allocate a single cell of memory if possible: if there is no memory
 * available, initiate a garbage collection, and try again. If there
 * just isn't enough memory, return NULL.
 */
cell* alloc(void);

/**
 * Some helper functions for allocation
 */
cell* alloc_components(component car, component cdr);
cell* alloc_ptr_ptr(cell* car, cell* cdr);
cell* alloc_ptr_atom(cell* car, unsigned int cdr);
cell* alloc_atom_ptr(unsigned int car, cell* cdr);
cell* alloc_atom_atom(unsigned int car, unsigned int cdr);

#endif
