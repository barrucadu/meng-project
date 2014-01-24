#ifndef _ARMSTRONG_VIRDING_H
#define _ARMSTRONG_VIRDING_H

#include "shared.h"

/**
 * Initialise the memory management system. After calling this,
 * invariants should all hold.
 */
void initialise(void);

/**
 * Allocate a single cell of memory if possible: if there is no memory
 * available, initiate a garbage collection, and try again. If there
 * just isn't enough memory, return NULL.
 * 
 * In the Armstrong/Virding collector, cells can only point back in
 * time, and so the car and cdr must be specified at allocation
 * time. For simplicity, immutability is not enforced in this
 * implementation, but would be in a "proper" implementation.
 */
cell* alloc(component car, component cdr);

/**
 * Some helper functions for allocation without explicitly building
 * the components
 */
cell* alloc_ptr_ptr(cell* car, cell* cdr);
cell* alloc_ptr_atom(cell* car, unsigned int cdr);
cell* alloc_atom_ptr(unsigned int car, cell* cdr);
cell* alloc_atom_atom(unsigned int car, unsigned int cdr);

#endif
