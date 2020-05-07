#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *tracefile;

// Global array for storing all the vaddr idx doubly linked list
Array ** array;

// To keep track of the current position in the tracefile / reference string
int offset;



//======================= Helper functions =============================//
// function for hashing vaddr to an unique integer
int vaddr_to_int(addr_t addr){
    addr >>= 3;

	addr = (addr ^ (addr >> 10) ^ (addr >> 20));

    return (int) addr & 0x3FF;
}

void add_node(int i, addr_t addr) {
	// create a new node for the new addr and initialize the information for new node
	Node* n = malloc(sizeof(Node));
	n->idx = i;
	n->vaddr = addr;
	n->next = n->prev = NULL;

	// get an index using hash function for the addr
	int idx = vaddr_to_int(addr);

	Array *ref = array[idx];
	// If the addr does not exist in the array
	if (ref == NULL) {
		ref = malloc(sizeof(Array));
		ref->head = ref->tail = n;
		array[idx] = ref;
	} else {
		ref->tail->next = n;
		ref->tail = n;
	}

	return;
}

// For popping the first node from idx list given the addr.
void pop(addr_t addr){
	int idx = vaddr_to_int(addr);
	Array* target = array[idx];

	if (target->head != target->tail) {
		Node * temp = target->head;
		target->head = target->head->next;
		free(temp);
	} else {
		free(target->head);
	}

	return;
}
//======================= Helper functions =============================//

/* Page to evict is chosen using the optimal (aka MIN) algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {

	int max_distance = -1;
	int evict_idx = -1;
	int i;
	int done = 0;

	for (i = 0; i < memsize; i++) {
		// check if the addr appears in the future or not
		int x = vaddr_to_int(coremap[i].vaddr);
		if (array[x] != NULL) {
			// the addr appears in the future, record the distance
			int dist = array[x]->head->idx - offset;
			coremap[i].dist = dist;
		}
	}
	// find the index of the node with the furtest distance or no longer appears in the future
	for (i = 0; i < memsize; i++) {
		if (coremap[i].dist > max_distance && !done) {
			max_distance = coremap[i].dist;
			evict_idx = i;
			coremap[i].dist = -1;
		} else if (coremap[i].dist == -1) {
			evict_idx = i;
			done = 1;
		}
	}

	return evict_idx;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	// Update the current position
	offset++;

	// Physical frame number
	int frame = p->frame >> PAGE_SHIFT;
	addr_t addr = coremap[frame].vaddr;
	pop(addr);

	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	FILE *tfp = stdin;
	char buf[MAXLINE];
	char type;
	addr_t vaddr = -1;
	int idx = 0;

	//Initialize the array
	array = malloc(sizeof(Array) * PAGE_SIZE);
	for (int i = 0 ; i < PAGE_SIZE; i++) {
		array[i] = NULL;
	}

	//Initialize necessary fields for opt
	for (int i = 0; i < memsize; i++) {
		coremap[i].node = NULL;         // Not needed for opt
		coremap[i].dist = -1;
	}

	// Starting postion
	offset = 0;


	if(tracefile != NULL) {
		if((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
		}
	}

	while(fgets(buf, MAXLINE, tfp) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			// Add this vaddr to the data structure
			add_node(idx, vaddr);
			idx++;
		} else {
			continue;
		}
	}

}
