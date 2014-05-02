#ifndef _FENICHEL_YOCHELSON_H
#define _FENICHEL_YOCHELSON_H

#include "shared.h"

/**
 * Allocate a single cell of memory if
 * possible: if there is no memory available,
 * initiate a garbage collection, and try
 * again. If there * just isn't enough
 * memory, return NULL.
 */
cell* alloc_cell(void);

/**
 * Some helper functions for allocation
 */
cell* alloc(component car, component cdr);
cell* alloc_ptr_ptr(cell* car, cell* cdr);
cell* alloc_ptr_atom(cell* car, uint cdr);
cell* alloc_atom_ptr(uint car, cell* cdr);
cell* alloc_atom_atom(uint car, uint cdr);

#endif
