// Copyright Teodora Stroe 331CA 2023

#include "helpers.h"

struct block_meta *request_space(struct block_meta *base, size_t size)
{
	struct block_meta *block = sbrk(0);						// get current position
	void *requested_chunk = sbrk(size + METADATA_SIZE);		// save space for struct

	if (requested_chunk == (void *)-1) {
		return NULL;	// sbrk failed 
	}

	struct block_meta *temp = base;

	block->size = ALIGN(size);
	block->next = NULL;
	block->status = STATUS_FREE;

	if (temp == NULL) {
		base = block;
		return block;
	}

	while (temp->next) {
		temp = temp->next;
	}

	temp->next = block;

	return block;
}

bool block_is_usable(struct block_meta *block, size_t size)
{
	if (!block) {
		return false;
	}

	return block->status == STATUS_FREE && block->size >= size;
}

struct block_meta *get_best_fit(struct block_meta *base, struct block_meta **last, size_t size)
{
	struct block_meta *block = base;
	struct block_meta *best_fit = NULL;

	while(block) {
		if (block_is_usable(block, size)) {
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

void coalesce_free_blocks(struct block_meta *base)
{
	struct block_meta *curr = base;

	while (curr) {
		if (curr->next == NULL) {
			return;
		}

		if (curr->status == STATUS_FREE && (curr->next)->status == STATUS_FREE) {
			merge_block(curr);
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