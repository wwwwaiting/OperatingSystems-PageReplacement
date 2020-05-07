#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int clock_hand;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {

	// check if the reference is set
	while (coremap[clock_hand % memsize].pte->frame & PG_REF) {
		// set the reference bit to 0
		coremap[clock_hand % memsize].pte->frame &= ~(PG_REF);
		clock_hand++;
	}
	// circular over the index
	int evict_idx = clock_hand % memsize;
	clock_hand++;

	return evict_idx;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// no need to update information
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm.
 */
void clock_init() {
	clock_hand = 0;
}
