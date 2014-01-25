#include <stddef.h>

#include "shared.h"
#include "lists.h"

#if GC == 0
#include "armstrong-virding.h"
#elif GC == 1
#include "fenichel-yochelson.h"
#else
#error "GC must be 0 (for armstrong-virding) or 1 (for fenichel-yochelson)"
#endif

list* singleton(unsigned int data)
{
#if GC == 0
  cell* out = alloc_atom_ptr(data, NULL);
#else
  cell* out = alloc();
  if(out != NULL)
    {
      out->car.tag = ATOM;
      out->car.val.data = data;
      out->cdr.tag = REFERENCE;
      out->cdr.val.ptr = NULL;
    }
#endif

  return out;
}

list* reverse(list* xs)
{
  cell* out = NULL;
  for(cell* cur = xs; cur != NULL; cur = cur->cdr.val.ptr)
    {
      // To prevent out being thrown away by the garbage collector, we
      // need to set it as a root before we allocate, and then remove
      // once we're done.
      if(out != NULL)
        add_root(out);
      component cdr = { .tag = REFERENCE, .val.ptr = out };
#if GC == 0
      cell* out2 = alloc(cur->car, cdr);
#else
      cell* out2 = alloc();
      if(out2 != NULL)
        {
          out2->car = xs->car;
          out2->cdr = cdr;
        }
#endif
      if(out != NULL)
        remove_root(out);
      out = out2;
    }
  return out;
}

list* append(list* xs, list* ys)
{
  cell* rxs = reverse(xs);
  add_root(rxs);
  cell* out = ys;

  for(cell* cur = rxs; cur != NULL; cur = cur->cdr.val.ptr)
    {
      if(out != ys)
        add_root(out);
      component cdr = { .tag = REFERENCE, .val.ptr = out };
#if GC == 0
      cell* out2 = alloc(cur->car, cdr);
#else
      cell* out2 = alloc();
      if(out2 != NULL)
        {
          out2->car = xs->car;
          out2->cdr = cdr;
        }
#endif
      if(out != ys)
        remove_root(out);
      out = out2;
    }

  remove_root(rxs);
    
  return out;
}

list* append_data(unsigned int data, list* xs)
{
#if GC == 0
  cell* out = alloc_atom_ptr(data, xs);
#else
  cell* out = alloc();
  if(out != NULL)
    {
      out->car.tag = ATOM;
      out->car.val.data = data;
      out->cdr.tag = REFERENCE;
      out->cdr.val.ptr = xs;
    }
#endif

  return out;
}

list* head(list* xs)
{
#if GC == 0
  cell* out = (xs->car.tag == ATOM) ? alloc_atom_ptr(xs->car.val.data, NULL) : alloc_ptr_ptr(xs->car.val.ptr, NULL);
#else
  cell* out = alloc();
  if(out != NULL)
    {
      out->car = xs->car;
      out->cdr.tag = REFERENCE;
      out->cdr.val.ptr = NULL;
    }
#endif

  return out;
}

list* tail(list* xs)
{
  return xs->cdr.val.ptr;
}
