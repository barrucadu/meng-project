#ifndef _ARMSTRONG_VIRDING_H
#define _ARMSTRONG_VIRDING_H

/*
 * Cells are implemented as a pair of tagged unions, as either the car
 * or the cdr may contain data or a reference to another cell.
 */
typedef enum { ATOM, REFERENCE } ctag;
struct cell;
typedef struct component {
  ctag tag;
  union {
    struct cell *ptr;
    unsigned int data;
  } val;
} component;
typedef struct cell {
  component car;
  component cdr;
} cell;

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
 *
 * Furthemore, we'll assume there is only one root, and it is the most
 * recently allocated cell.
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
