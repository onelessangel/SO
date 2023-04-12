#include "helpers.h"

struct block_meta *request_space(struct block_meta **base, struct block_meta *last, size_t size)
{
	struct block_meta *block = sbrk(0);						// get current position
	void *requested_chunk = sbrk(size + METADATA_SIZE);		// save space for struct

	// DIE(block == requested_chunk, "positions are the same when requesting space");

	if (requested_chunk == (void *)-1) {
		return NULL;	// sbrk failed 
	}

	// if (last) {
	// 	last->next = block;
	// }

	struct block_meta *temp = *base;

	if (temp == NULL) {
		*base = block;
	} else {
		while (temp->next) {
			temp = temp->next;
		}

		temp->next = block;
	}

	block->size = ALIGN(size);
	block->next = NULL;
	block->status = STATUS_FREE;

	return block;
}

bool block_is_usable(struct block_meta *block, size_t size)
{
	if (block == NULL) {
		return false;
	}

	// if (block->status != STATUS_FREE || block->size == 0) {
	// 	return false;
	// }

	// printf("am ajuns aici\n");
	// printf("size: %ld\n", block->size);
	// printf("status: %d\n", block->status);
	// printf("size: %ld\n", block->size);
	return block->status == STATUS_FREE && block->size >= size;
}

struct block_meta *find_free_block(struct block_meta *base, struct block_meta **last, size_t size)
{
	struct block_meta *current = base;

	while (current && !(current->status == STATUS_FREE && current->size >= size)) {
		*last = current;
    	current = current->next;
  	}

	if (current->status == STATUS_FREE) {
		return current;
	}

  	return NULL;
}


struct block_meta *get_best_fit(struct block_meta *base, struct block_meta **last, size_t size)
{
	struct block_meta *block = base;
	struct block_meta *best_fit = NULL;

	while(block) {
		if (block->status == STATUS_FREE && block->size >= size) {
			if (best_fit == NULL) {
				best_fit = block;
			} else if (block->size < best_fit->size) {
				best_fit = block;
			}
		}

		*last = block;
		block = block->next;
	}

	return best_fit;
}

void split_block(struct block_meta *block, size_t size, size_t nmemb)
{
	// if (block != NULL) {
	// 	printf("%ld\n", block->size);
	// }
	// printf("%ld\n", block->size);
	size_t aligned_size = ALIGN(size * nmemb);
	size_t remaining_size = block->size - aligned_size - METADATA_SIZE;

	// printf("remaining size: %d\n", remaining_size);

	if (block->size - aligned_size < METADATA_SIZE + ALIGN(1)) {
		// block->size = aligned_size;		//
		block->status = STATUS_ALLOC;
		return;
	}

	// tinker alignment
	struct block_meta *second_block;
	second_block = (struct block_meta *)((unsigned long)block + ALIGN(aligned_size + METADATA_SIZE));

	second_block->next = block->next;
	second_block->size = remaining_size;
	second_block->status = STATUS_FREE;

	block->size = aligned_size;
	block->next = second_block;
	block->status = STATUS_ALLOC;
}

void coalesce_blocks(struct block_meta *base)
{
	struct block_meta *curr = base;
	// size_t total_size = curr->size;
	// int counter = 0;

	while (curr) {
		if (curr->next == NULL) {	/* ?????????????? */
			return;
		}

		if (curr->status == STATUS_FREE && (curr->next)->status == STATUS_FREE) {
			curr->size += (curr->next)->size + METADATA_SIZE;
			curr->next = (curr->next)->next;
		} else {
			curr = curr->next;
		}
		// counter++;
	}

	// printf("counter: %d\n", counter);
}

void init_heap(struct block_meta **base)
{
	*base = sbrk(ALIGN(sizeof(struct block_meta)));
	DIE(*base == NULL, "heap init failed");

	(*base)->size = 0;
	(*base)->next = NULL;
	(*base)->status = STATUS_FREE;
}

struct block_meta *get_block_ptr(void *ptr)
{
  return (struct block_meta*) ptr - 1;
}
