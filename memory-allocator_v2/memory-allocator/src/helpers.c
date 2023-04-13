#include "helpers.h"

struct block_meta *request_space(struct block_meta **base, struct block_meta *last, size_t size)
{
	struct block_meta *block = sbrk(0);						// get current position
	void *requested_chunk = sbrk(size + METADATA_SIZE);		// save space for struct

	if (requested_chunk == (void *)-1) {
		return NULL;	// sbrk failed 
	}

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
	if (!block) {
		return false;
	}

	return block->status == STATUS_FREE && block->size >= size;
}

struct block_meta *find_free_block(struct block_meta *base, struct block_meta **last, size_t size)
{
	struct block_meta *current = base;

	while (current && !block_is_usable(current, size)) {
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

void split_block(struct block_meta *block, size_t size)
{
	size_t aligned_size = ALIGN(size);
	size_t remaining_size = block->size - aligned_size - METADATA_SIZE;

	if (block->size - aligned_size < METADATA_SIZE + ALIGN(1)) {
		// block->status = STATUS_ALLOC;
		return;
	}

	struct block_meta *second_block;
	second_block = (struct block_meta *)((unsigned long)block + ALIGN(aligned_size + METADATA_SIZE));

	second_block->next = block->next;
	second_block->size = remaining_size;
	second_block->status = STATUS_FREE;

	block->size = aligned_size;
	block->next = second_block;
	block->status = STATUS_ALLOC;
}

// void truncate_block(struct block_meta *block, size_t size)
// {
// 	size_t aligned_size = ALIGN(size);
// 	size_t remaining_size = block->size - aligned_size - METADATA_SIZE;

// 	// VERIFICARE??

// 	struct block_meta *second_block;
// 	second_block = (struct block_meta *)((unsigned long)block + ALIGN(aligned_size + METADATA_SIZE));

// 	second_block->next = block->next;
// 	second_block->size = remaining_size;

// 	block->size = aligned_size;
// 	block->next = second_block;

// 	if (block->status == STATUS_ALLOC) {
// 		block->status = STATUS_ALLOC;
// 		second_block->status = STATUS_FREE;
// 	} else if (block->status == STATUS_MAPPED) {
// 		block->status = STATUS_MAPPED;
// 		munmap(second_block, second_block->size + METADATA_SIZE);
// 	}
// }

void coalesce_blocks(struct block_meta *base)
{
	struct block_meta *curr = base;

	while (curr) {
		if (curr->next == NULL) {
			return;
		}

		if (curr->status == STATUS_FREE && (curr->next)->status == STATUS_FREE) {
			curr->size += (curr->next)->size + METADATA_SIZE;
			curr->next = (curr->next)->next;
		} else {
			curr = curr->next;
		}
	}
}

void merge_block(struct block_meta *old_block)
{
	old_block->size += (old_block->next)->size + METADATA_SIZE;
	old_block->next = (old_block->next)->next;
}

struct block_meta *get_block_ptr(void *ptr)
{
  return (struct block_meta*)(ptr - METADATA_SIZE);
}
