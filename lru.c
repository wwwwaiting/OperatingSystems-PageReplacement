#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;


// Doubly linked list for lru
// Tail keeps track of the least recently used page.
Node *head;
Node *tail;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	// Return the index of last node in the doubly linked list
	int idx = tail->idx;

	// De-reference the last node
	coremap[idx].node = NULL;
	Node* temp = tail;
	tail = tail->prev;
	tail->next = NULL;

	free(temp);

	return idx;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {

	int frame = p->frame >> PAGE_SHIFT;
	Node* ref = coremap[frame].node;

	// If ref is not in array aka. does not exist
	if (ref == NULL) {

		// create new node ref and initialize the information of ref
		Node* ref = malloc(sizeof(Node));
		ref->idx = p->frame >> PAGE_SHIFT;
		ref->next = ref->prev = NULL;

		ref->next = head;
		
		// Add ref to the referrence list
		coremap[frame].node = ref;

		if (tail == NULL) {             // If doubly linked list is empty
			head = tail = ref;
		} else {                        // If not, add ref to the head
			head->prev = ref;           // of the list
			head = ref;
		}

	} else if ( ref != head ) {         // ref exists, move ref to the head
		ref->prev->next = ref->next;    // connect ref-prev and ref-next if exists

		if (ref->next) {
			ref->next->prev = ref->prev;
		}

		if (ref == tail) { // if ref is the tail, update tail to ref->prev
           ref->next = NULL;
		   tail = ref->prev;
        }

        // updating information for ref
		ref->next = head;
        ref->prev = NULL;
		ref->next->prev = ref;

		head = ref;
	}

	return;
}


/* Initialize any data structures needed for this
 * replacement algorithm
 */
void lru_init() {
	// Initialize necessary fields for lru
	// This is needed for accessing the node in O(1) time.
	for (int i = 0; i < memsize; i++) {
		coremap[i].node = NULL;
	}
	// initialize head and tail to null
	head = NULL;
	tail = NULL;

	return;
}
