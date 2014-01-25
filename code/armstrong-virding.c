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
#define gchead(ptr) ((gccell*)((char*)ptr - offsetof(gccell, cell)))
#define hist(ptr)   (gchead(ptr)->history)
#define marked(ptr) (gchead(ptr)->mark != UNMARKED)

/* Prototypes */
static void gc(void);
static void free_cons(cell* cell);

/**
 * Cells in this scheme have a header consisting of a mark word and a
 * history pointer.
 */
typedef struct {
  unsigned int mark;
  cell* history;
  cell cell;
} gccell;

/* Helpers for assertion checking */
static bool on_free_list(const gccell* thecell);
static bool reachable_from(const gccell* root, const gccell* cell);
static bool reachable(const gccell* cell);
static unsigned int cell_id(const gccell* thecell);

/**
 * The heap is modelled as an array of gc cells
 */
gccell heap[NUM_CELLS * sizeof(gccell)];

/**
 * The address of the previously allocated cell is used to construct
 * the history list.
 */
cell* history = NULL;

/**
 * We keep a pointer to the head of the free list.
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
  for(unsigned int i = 0; i < NUM_CELLS; i++)
    free_cons(&(heap[i].cell));
}

/**
 * Allocate a cell
 */
cell* alloc(component car, component cdr)
{
  if(current->cdr.val.ptr == NULL)
    {
      fprintf(stderr, "Initiating garbage collection\n");

      // Precondition: `first` is reachable
      assert(reachable(gchead(first)));

      // When the collector is called, it assumes the roots have been
      // marked
      for(unsigned int i = 0; i < NUM_ROOTS; i++)
        if(roots[i] != NULL)
          gchead(roots[i])->mark = MARKED;

      gc();

      // Live Cell Invariant (defn. 4.0.6)
      for(unsigned int i = 0; i < NUM_CELLS; i++)
        if(reachable(&heap[i]))
          assert(!on_free_list(&heap[i]));

      // Postcondition: everything allocated is reachable
      for(unsigned int i = 0; i < NUM_CELLS; i++)
        if(!on_free_list(&heap[i]))
          assert(reachable(&heap[i]));
    }

  if(current->cdr.val.ptr == NULL)
    return NULL;

  // Update the "current" cell.
  cell* thecell = current;
  current = thecell->cdr.val.ptr;

  // Update the cell's history, and the global history
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
 * Run the Armstrong/Virding collector over the heap.
 */
static void gc()
{
  cell *last = current;
  cell *SCAV = hist(last);

  while(SCAV != first)
    {
      if(marked(SCAV))
        {
          if(SCAV->car.tag == REFERENCE && SCAV->car.val.ptr != NULL)
            gchead(SCAV->car.val.ptr)->mark = MARKED;

          if(SCAV->cdr.tag == REFERENCE && SCAV->cdr.val.ptr != NULL)
            gchead(SCAV->cdr.val.ptr)->mark = MARKED;

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

      // alloc(c) ∧ id c > id SCAV ⇒ (∀ c → x, id x < id SCAV ⇒ marked(h,x))
      for(unsigned int i = 0; i < NUM_CELLS; i++)
        if(reachable(&heap[i]) && cell_id(&heap[i]) > cell_id(gchead(SCAV)))
          {
            if(heap[i].cell.car.tag == REFERENCE &&
               heap[i].cell.car.val.ptr != NULL &&
               cell_id(gchead(heap[i].cell.car.val.ptr)) < cell_id(gchead(SCAV)))
              assert(gchead(heap[i].cell.car.val.ptr)->mark == MARKED);

            if(heap[i].cell.cdr.tag == REFERENCE &&
               heap[i].cell.cdr.val.ptr != NULL &&
               cell_id(gchead(heap[i].cell.cdr.val.ptr)) < cell_id(gchead(SCAV)))
              assert(gchead(heap[i].cell.cdr.val.ptr)->mark == MARKED);
          }

      // id c ⩾ id SCAV ⇒ (alloc(c) <=> c ∈ reach(h', roots))
      for(unsigned int i = 0; i < NUM_CELLS; i++)
        if(reachable(&heap[i]) && cell_id(&heap[i]) >= cell_id(gchead(SCAV)))
           assert(reachable(&heap[i]));

      // id c > id SCAV ∧ alloc(c) ⇒ ¬marked(h,c)
      for(unsigned int i = 0; i < NUM_CELLS; i++)
        if(reachable(&heap[i]) && cell_id(&heap[i]) > cell_id(gchead(SCAV)))
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
  for(cell* cur = current; cur != NULL; cur = cur->cdr.val.ptr)
    if(gchead(cur) == thecell)
      return true;
  return false;
}

/**
 * Return true if the cell can be reached from the given root
 */
static bool reachable_from(const gccell* root, const gccell* cell)
{
  if(root == cell)
    return true;

  if(root->cell.car.tag == REFERENCE &&
     root->cell.car.val.ptr != NULL &&
     reachable_from(gchead(root->cell.car.val.ptr), cell))
    return true;

  if(root->cell.cdr.tag == REFERENCE &&
     root->cell.cdr.val.ptr != NULL &&
     reachable_from(gchead(root->cell.cdr.val.ptr), cell))
    return true;

  return false;
}

/**
 * Return true if the cell can be reached from a root
 */
static bool reachable(const gccell* cell)
{
  for(unsigned int i = 0; i < NUM_ROOTS; i++)
    if(roots[i] != NULL &&
       reachable_from(gchead(roots[i]), cell))
      return true;

  return false;
}

/**
 * Get the ID of an allocated cell
 */
static unsigned int cell_id(const gccell* thecell)
{
  unsigned int pos = 0;
  unsigned int len = 0;
  bool found = false;

  for(cell* cur = current; cur != NULL; cur = gchead(cur)->history)
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
  component _car = { .tag = REFERENCE, .val.ptr = car };
  component _cdr = { .tag = REFERENCE, .val.ptr = cdr };
  return alloc(_car, _cdr);
}

cell* alloc_ptr_atom(cell* car, unsigned int cdr)
{
  component _car = { .tag = REFERENCE, .val.ptr  = car };
  component _cdr = { .tag = ATOM,      .val.data = cdr };
  return alloc(_car, _cdr);
}

cell* alloc_atom_ptr(unsigned int car, cell* cdr)
{
  component _car = { .tag = ATOM,      .val.data = car };
  component _cdr = { .tag = REFERENCE, .val.ptr  = cdr };
  return alloc(_car, _cdr);
}

cell* alloc_atom_atom(unsigned int car, unsigned int cdr)
{
  component _car = { .tag = ATOM, .val.data = car };
  component _cdr = { .tag = ATOM, .val.data = cdr };
  return alloc(_car, _cdr);
}
