#include <stddef.h>
#include <stdbool.h>
#include "shared.h"

/**
 * Roots are stored in a fixed-size list
 */
cell* roots[NUM_ROOTS] = { NULL };

/**
 * Add a root if there is room
 */
bool add_root(cell* root)
{
  // Don't add the same root twice
  for(unsigned int i = 0; i < NUM_ROOTS; i++)
    if(roots[i] == root)
      return true;

  // Add to the first free slot
  for(unsigned int i = 0; i < NUM_ROOTS; i++)
    if(roots[i] == NULL)
      {
        roots[i] = root;
        return true;
      }

  return false;
}

/**
 * Remove a root
 */
void remove_root(cell* root)
{
  for(unsigned int i = 0; i < NUM_ROOTS; i++)
    if(roots[i] == root)
      roots[i] = NULL;
}
