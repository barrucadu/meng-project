#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

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

/* Helpers for assertion checking */
static bool on_free_list(const cell* thecell);
static bool reachable_from(const cell* root, const cell* thecell);
static bool reachable(const cell* thecell);
static void check_pointers_updated(const cell* root, const cell* cur);

/**
 * Allocate a cell
 */
cell* alloc()
{
  if(next == NUM_CELLS)
    {
      fprintf(stderr, "Initiating garbage collection\n");

      // Precondition: No inter-semispace pointers
      cell *sspace = (cons_space == FIRST) ? semispace1 : semispace2;

      for(unsigned int i = 0; i < NUM_CELLS; i++)
        if(reachable(&sspace[i]))
          {
            if(sspace[i].car.tag == REFERENCE &&
               sspace[i].car.val.ptr != NULL)
              assert(sspace[i].car.val.ptr >= sspace &&
                     sspace[i].car.val.ptr <= &sspace[NUM_CELLS]);

            if(sspace[i].cdr.tag == REFERENCE &&
               sspace[i].cdr.val.ptr != NULL)
              assert(sspace[i].cdr.val.ptr >= sspace &&
                     sspace[i].cdr.val.ptr <= &sspace[NUM_CELLS]);
          }

      gc();

      sspace = (cons_space == FIRST) ? semispace1 : semispace2;

      // Live Cell Invariant (defn. 4.0.6)
      for(unsigned int i = 0; i < NUM_CELLS; i++)
        if(reachable(&sspace[i]))
          assert(!on_free_list(&sspace[i]));

      // Postcondition: everything allocated is reachable
      for(unsigned int i = 0; i < NUM_CELLS; i++)
        if(!on_free_list(&sspace[i]))
          assert(reachable(&sspace[i]));
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
      {
        roots[i] = collect(roots[i]);

        // Loop variant: everything reachable from the root has its
        // pointers updated.
        check_pointers_updated(roots[i], NULL);
      }

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
      component a = { .tag = p->car.tag, .val = p->car.val };
      component b = { .tag = p->cdr.tag, .val = p->cdr.val };
      cell* q = alloc_ptr_ptr(NULL, NULL);

      // rplaca(p, ALREADYCOPIED)
      p->car.tag = REFERENCE;
      p->car.val.ptr = ALREADYCOPIED;

      // rplacd(p, q)
      p->cdr.tag = REFERENCE;
      p->cdr.val.ptr = q;

      // nrplaca(q, collect(a))
      if(a.tag == REFERENCE && a.val.ptr != NULL)
        q->car.val.ptr = collect(a.val.ptr);

      // nrplacd(q, collect(b)) 
      if(b.tag == REFERENCE && b.val.ptr != NULL)
        q->cdr.val.ptr = collect(b.val.ptr);

      return q;
    }
}

/**
 * Check if the cell is on the implicit free list
 */
static bool on_free_list(const cell* thecell)
{
  if(pointer_space == FIRST)
    return thecell >= &semispace1[next];
  else
    return thecell >= &semispace2[next];
}

/**
 * Check if a cell is reachable from the given root
 */
static bool reachable_from(const cell* root, const cell* thecell)
{
  if(root == thecell)
    return true;

  if(root->car.tag == REFERENCE &&
     root->car.val.ptr != NULL &&
     root->car.val.ptr != ALREADYCOPIED &&
     reachable_from(root->car.val.ptr, thecell))
    return true;

  if(root->cdr.tag == REFERENCE &&
     root->cdr.val.ptr != NULL &&
     reachable_from(root->cdr.val.ptr, thecell))
    return true;

  return false;
}

/**
 * Check if a cell is reachable
 */
static bool reachable(const cell* thecell)
{
  for(unsigned int i = 0; i < NUM_ROOTS; i++)
    if(roots[i] != NULL &&
       reachable_from(roots[i], thecell))
      return true;

  return false;
}

/**
 * Check that all the pointers reachable from a cell have been updated
 */
static void check_pointers_updated(const cell* root, const cell* cur)
{
  if(root == cur)
    return;

  if(cur == NULL)
    cur = root;

  cell *sspace = (cons_space == FIRST) ? semispace1 : semispace2;

  if(cur->car.tag == REFERENCE &&
     cur->car.val.ptr != ALREADYCOPIED &&
     cur->car.val.ptr != NULL)
    {
      assert(cur->car.val.ptr >= sspace &&
             cur->car.val.ptr <= &sspace[NUM_CELLS]);
      check_pointers_updated(root, cur->car.val.ptr);
    }

  if(cur->cdr.tag == REFERENCE &&
     cur->cdr.val.ptr != NULL)
    {
      assert(cur->cdr.val.ptr >= sspace &&
             cur->cdr.val.ptr <= &sspace[NUM_CELLS]);
      check_pointers_updated(root, cur->cdr.val.ptr);
    }
}

/**
 * Helper functions
 */
cell* alloc_components(component car, component cdr)
{
  cell* out = alloc();
  if(out != NULL)
    {
      out->car = car;
      out->cdr = cdr;
    }
  return out;
}

cell* alloc_ptr_ptr(cell* car, cell* cdr)
{
  component _car = { .tag = REFERENCE, .val.ptr = car };
  component _cdr = { .tag = REFERENCE, .val.ptr = cdr };
  return alloc_components(_car, _cdr);
}

cell* alloc_ptr_atom(cell* car, unsigned int cdr)
{
  component _car = { .tag = REFERENCE, .val.ptr  = car };
  component _cdr = { .tag = ATOM,      .val.data = cdr };
  return alloc_components(_car, _cdr);
}

cell* alloc_atom_ptr(unsigned int car, cell* cdr)
{
  component _car = { .tag = ATOM,      .val.data = car };
  component _cdr = { .tag = REFERENCE, .val.ptr  = cdr };
  return alloc_components(_car, _cdr);
}

cell* alloc_atom_atom(unsigned int car, unsigned int cdr)
{
  component _car = { .tag = ATOM, .val.data = car };
  component _cdr = { .tag = ATOM, .val.data = cdr };
  return alloc_components(_car, _cdr);
}
