#include <stdio.h>
#include <assert.h>

#include "lists.h"
#include "shared.h"

extern cell* roots[NUM_ROOTS];

#if GC == 0
#include "armstrong-virding.h"
#endif

int main(void)
{
#if GC == 0
  // Initialise the Armstrong-Virding collector
  initialise();
#else
  // Armstrong/Virding has a one cell overhead, so allocate one more
  // for Fenichel/Yochelson so we get the same behaviour.
  add_root(singleton(0));
#endif

  // Try to allocate a bunch of cells
  list** xs = NULL;
  for(unsigned int i = 0; i < NUM_CELLS - 1; i++)
    {
      list** newxs = add_root(append_data(i, (xs == NULL) ? NULL : *xs));

      assert(*newxs != NULL);

      if(xs != NULL)
        remove_root(*xs);
      xs = newxs;
    }

  printf("xs: ");
  for(cell* x = *xs; x != NULL; x = x->cdr.val.ptr)
    printf("%u ", x->car.val.data);
  printf("\n");

  // Try allocating one more cell
  assert(singleton(0) == NULL);

  // Chop off the head of the list and allocate a few more cells
  add_root(tail(tail(tail(*xs))));
  remove_root(*xs);
  list** sing1 = add_root(singleton(0));
  list** sing2 = add_root(singleton(1));
  list** sing3 = add_root(singleton(2));
  list** sing4 = add_root(singleton(3));
  assert(*sing1 != NULL);
  assert(*sing2 != NULL);
  assert(*sing3 != NULL);
  assert(*sing4 == NULL);

  return 0;
}
