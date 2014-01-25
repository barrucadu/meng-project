#include <stddef.h>
#include <stdio.h>

#include "shared.h"
#include "fenichel-yochelson.h"

/**
 * The list of roots
 */
extern cell* roots[NUM_ROOTS];

/* Utility functions */
#define flip(space) space = (space == FIRST) ? SECOND : FIRST

/* "ALREADYCOPIED" value - an invalid pointer */
#define ALREADYCOPIED (cell*)1

/**
 * Names for the semispaces
 */
typedef enum { FIRST, SECOND } semispace;

/**
 * The semispace in which we allocate cons
 */
semispace cons_space = FIRST;

/**
 * The semispace in which we interpret pointers
 */
semispace pointer_space = FIRST;

/**
 * The heap consists of two semispaces able to hold the given number
 * of cells.
 */
cell semispace1[NUM_CELLS];
cell semispace2[NUM_CELLS];

/**
 * The ID of the next cell to allocate
 */
unsigned int next = 0;

/* Prototypes */
static void gc(void);
static cell* collect(cell* p);

/**
 * Allocate a cell
 */
cell* alloc()
{
  if(next == NUM_CELLS)
    {
      fprintf(stderr, "Initiating garbage collection\n");
      gc();
    }

  if(next == NUM_CELLS)
    return NULL;

  // Get the cell 
  cell* thecell;
  if(cons_space == FIRST)
    thecell = &semispace1[next];
  else
    thecell = &semispace2[next];

  // Initialise its car and cdr to sensible values
  thecell->car.tag = REFERENCE;
  thecell->car.val.ptr = NULL;
  thecell->cdr.tag = REFERENCE;
  thecell->cdr.val.ptr = NULL;

  // Update the next cell
  next ++;

  // Done!
  return thecell;
}

/**
 * Run the Fenichel/Yochelson collector
 */
static void gc()
{
  flip(cons_space);
  next = 0;

  for(unsigned int i = 0; i < NUM_ROOTS; i ++)
    if(roots[i] != NULL)
      roots[i] = collect(roots[i]);

  flip(pointer_space);
}

/**
 * Recursively collect
 *
 * This implementation has no atoms, other than unsigned ints, which
 * fit in the same space as a pointer, so the atomic case doesn't need
 * to be considered. Besides, that's not really covered by the
 * collector.
 */
static cell* collect(cell* p)
{
  if(p->car.tag == REFERENCE && p->car.val.ptr == ALREADYCOPIED)
    {
      return p->cdr.val.ptr;
    }
  else
    {
      component a = p->car;
      component b = p->cdr;
      cell* q = alloc();

      // rplaca(p, ALREADYCOPIED)
      p->car.tag = REFERENCE;
      p->car.val.ptr = ALREADYCOPIED;

      // rplacd(p, q)
      p->cdr.tag = REFERENCE;
      p->cdr.val.ptr = q;

      // nrplaca(q, collect(a))
      q->car = a;
      if(a.tag == REFERENCE && a.val.ptr != NULL)
        q->car.val.ptr = collect(q->car.val.ptr);

      // nrplacd(q, collect(b))
      q->cdr = b;
      if(b.tag == REFERENCE && b.val.ptr != NULL)
        q->cdr.val.ptr = collect(q->cdr.val.ptr);

      return q;
    }
}
