#include <stddef.h>

#include "armstrong-virding.h"

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

/**
 * The heap is modelled as an array of gc cells
 */
gccell* heap[NUM_CELLS * sizeof(gccell)];

/**
 * The address of the previously allocated cell is used to construct
 * the history list.
 */
cell* history;

/**
 * We keep a pointer to the head of the free list.
 */
cell* current;

/**
 * For simplicity of implementation, we just have one root. In a
 * "proper" implementation as part of some larger system (such as a
 * programming language runtime) there would be an easy way to get all
 * the roots, but, well, this is C.
 */
cell* root;

/**
 * The first allocated cell.
 */
cell* first;

/**
 * Initialise: construct the freelist.
 */
void initialise()
{
  history = NULL;
  current = NULL;
  root = NULL;
  first = NULL;

  for(gccell* thecell = heap[0]; thecell < heap[NUM_CELLS]; thecell++)
    free_cons(&(thecell->cell));
}

/**
 * Allocate a cell
 */
cell* alloc(component car, component cdr)
{
  if(current->car.val.ptr == NULL)
    gc();

  if(current->car.val.ptr == NULL)
    return NULL;

  // Update the "current" cell.
  cell* thecell = current;
  current = thecell->cdr.val.ptr;

  // Update the cell's history, and the global history
  hist(thecell) = history;
  history = thecell;

  // Unmark the cell
  gchead(thecell)->mark = UNMARKED;

  // Set the car and cdr
  thecell->car = car;
  thecell->cdr = cdr;

  // Update root and first
  root = thecell;
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
          if(SCAV->car.tag == REFERENCE)
            gchead(SCAV->car.val.ptr)->mark = MARKED;

          if(SCAV->cdr.tag == REFERENCE)
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
    }
}

/**
 * Free a cons cell
 */
static void free_cons(cell* cell)
{
  cell->cdr.tag = REFERENCE;
  cell->cdr.val.ptr = current;

  hist(current) = cell;
  hist(cell) = history;

  current = cell;
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
