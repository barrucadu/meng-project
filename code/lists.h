#ifndef _LISTS_H
#define _LISTS_H

#include "shared.h"

typedef cell list;

/**
 * Allocate a singleton list
 */
list* singleton(unsigned int data);

/**
 * Reverse a list
 */
list* reverse(list* xs);

/**
 * Create a new list by appending ys to xs. This allocates a totally
 * new xs, to avoid mutating cdrs.
 */
list* append(list* xs, list* ys);

/**
 * Special-case of append for appending data to a list.
 */
list* append_data(unsigned int data, list* xs);

/**
 * Get the head of a list, as a singleton list
 */
list* head(list* xs);

/**
 * Get the tail of a list
 */
list* tail(list* xs);

#endif
