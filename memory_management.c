#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MEMORYSIZE 128
#define BLOCKSIZE 2
#define NUMREQUESTS 10000

typedef struct Node {
	int free; // amount of free memory
	int pid;
	struct Node *next;
} Node;

/* required functions */
int allocate_mem_ff(int process_id, int num_units);
int allocate_mem_bf(int process_id, int num_units);
int allocate_mem_wf(int process_id, int num_units);
int deallocate_mem(int process_id);
int fragment_count();

/* helper functions */
Node *build_memory();
int random_range(int min, int max);
void print_memory();
void choose_deallocate(Node *pidList);
void simulate(int (*allocate)(int, int), char *title);

Node *head;

int main() {
	/* seed random */
	time_t t;
	srand((unsigned)time(&t));
	
	simulate(allocate_mem_ff,"First Fit");
	simulate(allocate_mem_bf,"Best Fit");
	simulate(allocate_mem_wf,"Worst Fit");
}

int allocate_mem_ff(int process_id, int num_units) {
	int num_nodes = (int)ceil(num_units/2);
	int free_block = 0;
	int count = 0;
	Node *current = head;
	Node *block_start = NULL;
	
	/* find block */
	while(free_block < num_nodes) {
		
		if(current->free == BLOCKSIZE) {
			if(free_block == 0)
				block_start = current;
			
			free_block++;
		} else {
			free_block = 0;
		}
		
		count++;
		current = current->next;
		
		if(current == NULL)
			if(free_block < num_nodes)
				return -1; // couldn't find a valid block
			else
				break; // found a valid block at the end
	}
	
	/* allocate */
	int i;
	current = block_start;
	for(i = 0; i < num_nodes-1; i++) {
		current->free = 0;
		current->pid = process_id;
		current = current->next;
	}
	
	/* last node might not be filled */
	current->free = num_units%BLOCKSIZE;
	current->pid = process_id;
	
	return count;
}

int allocate_mem_bf(int process_id, int num_units) {
	int num_nodes = (int)ceil(num_units/2);
	int free_block = 0;
	int best_block = MEMORYSIZE; // starts at largest size and gets smaller
	int count = 0;
	Node *current = head;
	Node *block_start = NULL;
	Node *best_start = NULL; // points to current best block of memory
	
	/* find block */
	while(current != NULL) {
		
		if(current->free == BLOCKSIZE) {
			if(free_block == 0)
				block_start = current;
			
			free_block++;
		} else {
			free_block = 0;
		}
		
		/* compare current block to previous best */
		if((free_block >= num_nodes) && (free_block < best_block)) {
			best_block = free_block;
			best_start = block_start;
		}
		
		count++;
		current = current->next;
		
	}
	
	if(best_start == NULL)
		return -1; // couldn't find a valid block
	
	/* allocate */
	int i;
	current = best_start;
	for(i = 0; i < num_nodes-1; i++) {
		current->free = 0;
		current->pid = process_id;
		current = current->next;
	}
	
	/* last node might not be filled */
	current->free = num_units%BLOCKSIZE;
	current->pid = process_id;
	
	return count;
}

int allocate_mem_wf(int process_id, int num_units) {
	int num_nodes = (int)ceil(num_units/2);
	int free_block = 0;
	int worst_block = 0; // starts at smallest size and gets larger
	int count = 0;
	Node *current = head;
	Node *block_start = NULL;
	Node *worst_start = NULL; // points to current worst block of memory
	
	/* find block */
	while(current != NULL) {
		
		if(current->free == BLOCKSIZE) {
			if(free_block == 0)
				block_start = current;
			
			free_block++;
		} else {
			free_block = 0;
		}
		
		/* compare current block to previous worst */
		if((free_block >= num_nodes) && (free_block > worst_block)) {
			worst_block = free_block;
			worst_start = block_start;
		}
		
		count++;
		current = current->next;
		
	}
	
	if(worst_start == NULL)
		return -1; // couldn't find a valid block
	
	/* allocate */
	int i;
	current = worst_start;
	for(i = 0; i < num_nodes-1; i++) {
		current->free = 0;
		current->pid = process_id;
		current = current->next;
	}
	
	/* last node might not be filled */
	current->free = num_units%BLOCKSIZE;
	current->pid = process_id;
	
	return count;
}

int fragment_count() {
	int count = 0;
	Node *current = head;
	while(current) {
		count += current->free;
		current = current->next;
	}
	return count;
}

int deallocate_mem(int process_id) {
	Node *current = head;
	int allocated = -1;
	while(current) {
		if(current->pid == process_id) {
			current->pid = 0;
			current->free = BLOCKSIZE;
			allocated = 1;
			
		/* if the process is found, but not in the current node, we're done */
		} else if(allocated == 1)
			break;
		
		current = current->next;
	}
	return allocated;
}

void simulate(int (*allocate)(int, int), char *title) {
	head = build_memory();
	int i;
	int traversed = 0;
	int hits = 0;
	
	/* circular list to keep track of PIDs allocated */
	Node *pIdHead = (Node *)malloc(sizeof(Node)); 
	Node *currentId = pIdHead;
	currentId->next = pIdHead;
	
	for(i = 0; i < NUMREQUESTS; i++) {
		int pid = random_range(0,999999);
		
		if(random_range(0,3) == 1) // 1 in 4 chance for deallocation request
			choose_deallocate(pIdHead);
		else {	
			int result = allocate(pid,random_range(3,10));
			//int result = allocate(pid,3);
			if(result != -1) {
				currentId->pid = pid;
				currentId->next = (Node *)malloc(sizeof(Node));
				currentId = currentId->next;
				currentId->next = pIdHead;
				hits++;
				traversed += result;
			}
		}
	}
	
	//print_memory();
	printf("%s\n",title);
	printf("Average Nodes Traversed: %.2f\n",(double)traversed/hits);
	printf("Request Denial Percentage: %.2f\n",(double)(NUMREQUESTS-hits)/NUMREQUESTS*100);
	printf("Average Fragments: %.2f\n",(double)fragment_count()/hits);
}

void print_memory() {
	Node *current = head;
	while(current) {
		printf("%d",current->free);
		current = current->next;
	}
	printf("\n");
}

int random_range(int min, int max) {
	int range = (max - min) + 1;
	return (rand()%range + min);
}

Node *build_memory() {
	Node *head = (Node *)malloc(sizeof(Node));
	Node *current = head;
	
	int i;
	for(i = 0; i < MEMORYSIZE-1; i++) {
		current->free = BLOCKSIZE;
		current->pid = 0;
		current->next = (Node *)malloc(sizeof(Node));
		current = current->next;
	}
	
	return head;
}

void choose_deallocate(Node *pidList) {
	Node *c = pidList;
	int i;
	int max = random_range(0,10);
	
	/* randomly iterate through list of PIDs and find one to deallocate */
	for(i = 0; i < max; i++)
		c = c->next;
			
	/* deallocate and remove node from list */
	deallocate_mem(c->next->pid);
	c->next = c->next->next;
}
