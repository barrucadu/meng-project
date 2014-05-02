#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "shared.h"
#include "armstrong-virding.h"

/**
 * The list of roots
 */
extern cell* roots[NUM_ROOTS];

/* 0 = unmarked, anything else = marked */
#define UNMARKED 0
#define MARKED   1

/* Utility functions for manipulating cells */
#define gchead(ptr) ((gccell*)((char*)ptr \
  - offsetof(gccell, cell)))
#define hist(ptr) (gchead(ptr)->history)
#define marked(ptr) (gchead(ptr)->mark \
  != UNMARKED)

/* Prototypes */
static void gc(void);
static void free_cons(cell* cell);

/**
 * Cells in this scheme have a header
consisting of a mark word and a history
pointer.
 */
typedef struct {
  uint mark;
  cell* history;
  cell cell;
} gccell;

/* Helpers for assertion checking */
static bool on_free_list(const gccell* thecell);
static bool reachable_from(const gccell* root,
                           const gccell* cell);
static bool reachable(const gccell* cell);
static uint cell_id(const gccell* thecell);

/**
 * The heap is modelled as an array of gc
 * cells
 */
gccell heap[NUM_CELLS * sizeof(gccell)];

/**
 * The address of the previously allocated
 * cell is used to construct the history
 * list.
 */
cell* history = NULL;

/**
 * We keep a pointer to the head of the free
 * list.
 */
cell* current = NULL;

/**
 * The first allocated cell.
 */
cell* first = NULL;

/**
 * Initialise: construct the freelist.
 */
void initialise()
{
  for(uint i = 0; i < NUM_CELLS; i++)
    free_cons(&(heap[i].cell));
}

/**
 * Allocate a cell
 */
cell* alloc(component car, component cdr)
{
  if(current->cdr.val.ptr == NULL)
    {
      fprintf(stderr,
        "Initiating garbage collection\n");

      // Precondition: `first` is reachable
      assert(reachable(gchead(first)));

      // When the collector is called, it
      // assumes the roots have been marked
      for(uint i = 0; i < NUM_ROOTS; i++)
        if(roots[i] != NULL)
          gchead(roots[i])->mark = MARKED;

      gc();

      // Live Cell Invariant (defn. 4.0.6)
      for(uint i = 0; i < NUM_CELLS; i++)
        if(reachable(&heap[i]))
          assert(!on_free_list(&heap[i]));

      // Postcondition: everything allocated
      // is reachable
      for(uint i = 0; i < NUM_CELLS; i++)
        if(!on_free_list(&heap[i]))
          assert(reachable(&heap[i]));
    }

  if(current->cdr.val.ptr == NULL)
    return NULL;

  // Update the "current" cell.
  cell* thecell = current;
  current = thecell->cdr.val.ptr;

  // Update the cell's history, and the
  // global history
  hist(thecell) = history;
  hist(current) = thecell;
  history = thecell;

  // Unmark the cell
  gchead(thecell)->mark = UNMARKED;

  // Set the car and cdr
  thecell->car = car;
  thecell->cdr = cdr;

  // Update first
  if(first == NULL)
    first = thecell;

  // Done!
  return thecell;
}

/**
 * Run the Armstrong/Virding collector over
 * the heap.
 */
static void gc()
{
  cell *last = current;
  cell *SCAV = hist(last);

  while(SCAV != first)
    {
      if(marked(SCAV))
        {
          component car = SCAV->car;
          component cdr = SCAV->cdr;

          if(car.tag == REFERENCE
             && car.val.ptr != NULL)
            gchead(car.val.ptr)->mark = MARKED;

          if(cdr.tag == REFERENCE
             && cdr.val.ptr != NULL)
            gchead(cdr.val.ptr)->mark = MARKED;

          gchead(SCAV)->mark = UNMARKED;
          last = SCAV;
          SCAV = hist(last);
        }
      else
        {
          cell *tmp = SCAV;
          SCAV = hist(SCAV);
          hist(last) = SCAV;
          free_cons(tmp);
        }

      uint SCAV_id = cell_id(gchead(SCAV));

      // alloc(c) && id c > id SCAV -> (forall.
      // c -> x, id x < id SCAV -> marked(h,x))
      for(uint i = 0; i < NUM_CELLS; i++)
        if(reachable(&heap[i])
           && cell_id(&heap[i]) > SCAV_id)
          {
            cell cell = heap[i].cell;
            component car = cell.car;
            component cdr = cell.cdr;

            if(cell.car.tag == REFERENCE &&
               cell.car.val.ptr != NULL &&
               cell_id(gchead(car.val.ptr))
                 < SCAV_id)
              assert(gchead(car.val.ptr)->mark
                       == MARKED);

            if(cell.cdr.tag == REFERENCE &&
               cell.cdr.val.ptr != NULL &&
               cell_id(gchead(cdr.val.ptr))
                 < SCAV_id)
              assert(gchead(cdr.val.ptr)->mark
                       == MARKED);
          }

      // id c >= id SCAV -> (alloc(c) <=>
      // c \in reach(h', roots))
      for(uint i = 0; i < NUM_CELLS; i++)
        if(reachable(&heap[i])
           && cell_id(&heap[i]) >= SCAV_id)
           assert(reachable(&heap[i]));

      // id c > id SCAV && alloc(c) ->
      // !marked(h,c)
      for(uint i = 0; i < NUM_CELLS; i++)
        if(reachable(&heap[i])
           && cell_id(&heap[i]) > SCAV_id)
          assert(heap[i].mark == UNMARKED);
    }
}

/**
 * Free a cons cell
 */
static void free_cons(cell* cell)
{
  cell->cdr.tag = REFERENCE;
  cell->cdr.val.ptr = current;

  if(cell == history)
    history = hist(cell);

  current = cell;
}

/**
 * Check if a cell is on the free list
 */
static bool on_free_list(const gccell* thecell)
{
  for(cell* cur = current;
      cur != NULL;
      cur = cur->cdr.val.ptr)
    if(gchead(cur) == thecell)
      return true;
  return false;
}

/**
 * Return true if the cell can be reached
 * from the given root
 */
static bool reachable_from(const gccell* root, const gccell* cell)
{
  if(root == cell)
    return true;

  component car = root->cell.car;
  component cdr = root->cell.cdr;

  if(car.tag == REFERENCE &&
     car.val.ptr != NULL &&
     reachable_from(gchead(car.val.ptr), cell))
    return true;

  if(cdr.tag == REFERENCE &&
     cdr.val.ptr != NULL &&
     reachable_from(gchead(cdr.val.ptr), cell))
    return true;

  return false;
}

/**
 * Return true if the cell can be reached
 * from a root
 */
static bool reachable(const gccell* cell)
{
  for(uint i = 0; i < NUM_ROOTS; i++)
    if(roots[i] != NULL &&
       reachable_from(gchead(roots[i]), cell))
      return true;

  return false;
}

/**
 * Get the ID of an allocated cell
 */
static uint cell_id(const gccell* thecell)
{
  uint pos = 0;
  uint len = 0;
  bool found = false;

  for(cell* cur = current;
      cur != NULL;
      cur = gchead(cur)->history)
    {
      len ++;
      if(!found) pos ++;
      if(gchead(cur) == thecell) found = true;
    }

  return len - pos;
}

/* Helper functions */
cell* alloc_ptr_ptr(cell* car, cell* cdr)
{
  component _car = { .tag = REFERENCE,
                     .val.ptr = car };
  component _cdr = { .tag = REFERENCE,
                     .val.ptr = cdr };
  return alloc(_car, _cdr);
}

cell* alloc_ptr_atom(cell* car, uint cdr)
{
  component _car = { .tag = REFERENCE,
                     .val.ptr  = car };
  component _cdr = { .tag = ATOM,
                     .val.data = cdr };
  return alloc(_car, _cdr);
}

cell* alloc_atom_ptr(uint car, cell* cdr)
{
  component _car = { .tag = ATOM,
                     .val.data = car };
  component _cdr = { .tag = REFERENCE,
                     .val.ptr  = cdr };
  return alloc(_car, _cdr);
}

cell* alloc_atom_atom(uint car, uint cdr)
{
  component _car = { .tag = ATOM,
                     .val.data = car };
  component _cdr = { .tag = ATOM,
                     .val.data = cdr };
  return alloc(_car, _cdr);
}
