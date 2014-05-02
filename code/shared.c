#include <stddef.h>
#include "shared.h"

/**
 * Roots are stored in a fixed-size list
 */
cell* roots[NUM_ROOTS] = { NULL };

/**
 * Add a root if there is room
 */
cell** add_root(cell* root)
{
  // Don't add the same root twice
  for(uint i = 0; i < NUM_ROOTS; i++)
    if(roots[i] == root)
      return &roots[i];

  // Add to the first free slot
  for(uint i = 0; i < NUM_ROOTS; i++)
    if(roots[i] == NULL)
      {
        roots[i] = root;
        return &roots[i];
      }

  return NULL;
}

/**
 * Remove a root
 */
void remove_root(cell* root)
{
  for(uint i = 0; i < NUM_ROOTS; i++)
    if(roots[i] == root)
      roots[i] = NULL;
}
