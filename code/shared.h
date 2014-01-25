#ifndef _SHARED_H
#define _SHARED_H

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
 * Add a root to the list of roots. To avoid the need to build a
 * linked list (and so need to allocate memory), a fixed-size list is
 * used. This function returns a pointer to the root - giving a layer
 * of indirection which allows the GC to move stuff around in the
 * background. In a "proper" implementation, there would be a better
 * way to get the roots than this.
 */
cell** add_root(cell* root);

/**
 * Remove a root from the list of roots. This always succeeds: if the
 * root isn't in the list, this is essentially an expensive NOP
 */
void remove_root(cell* root);

#endif
