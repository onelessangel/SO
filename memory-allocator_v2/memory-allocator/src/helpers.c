#include "helpers.h"

struct block_meta *request_space(struct block_meta *last, size_t size)
{
	struct block_meta *block = sbrk(0);						// get current position
	void *requested_chunk = sbrk(size + METADATA_SIZE);		// save space for struct

	// DIE(block == requested_chunk, "positions are the same when requesting space");

	if (request_space == (void *)-1) {
		return NULL;	// sbrk failed 
	}

	if (last) {
		last->next = block;
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

struct block_meta *get_free_block(struct block_meta *base, size_t size)
{
	struct block_meta *curr = base;

	// find first usable block
	while (curr && !block_is_usable(curr, size)) {
		curr = curr->next;
	}

	return curr;
}

void split_block(struct block_meta *block, size_t size)
{
	size_t aligned_size = ALIGN(size);
	size_t remaining_size = block->size - (aligned_size + METADATA_SIZE);

	if (remaining_size < METADATA_SIZE + 1) {
		return;
	}

	// tinker alignment
	struct block_meta *second_block;
	second_block = block + aligned_size + METADATA_SIZE;
	second_block->next = block->next;
	second_block->size = ALIGN(remaining_size);
	second_block->status = block->status;

	block->size = aligned_size;
	block->next = second_block;
}

void coalesce_blocks(struct block_meta *base)
{
	struct block_meta *curr = base;
	size_t total_size = curr->size;

	while (curr) {
		if (curr->status == STATUS_FREE && curr->next->status == STATUS_FREE) {
			curr->size += ALIGN((curr->next)->size + METADATA_SIZE);
			curr->next = (curr->next)->next;
		} else {
			curr = curr->next;
		}
	}
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
