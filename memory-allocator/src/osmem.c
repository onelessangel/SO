// SPDX-License-Identifier: BSD-3-Clause

#include "osmem.h"
#include "helpers.h"

struct block_meta *global_base = NULL;

void *os_malloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}

	size_t aligned_size = ALIGN(size);
	static bool heap_is_init = false;

	if (!heap_is_init) {
		init_heap(&global_base);
	}

	struct block_meta *block;

	if (aligned_size + METADATA_SIZE < MMAP_THRESHOLD) {
		// block = get_free_block(global_base, )
		// facem request_space de la ultimul block free
		block = request_space(global_base, MMAP_THRESHOLD - METADATA_SIZE - global_base->size);
		// coalesce_blocks(block);

		split_block(block, aligned_size);
		block->status = STATUS_ALLOC;

		if (global_base->size == 0) {
			global_base = block;
		}
		

		// DIE??


	}

	return block + 1;
}

void os_free(void *ptr)
{
	struct block_meta *curr = get_block_ptr(ptr);

	while (curr->status == STATUS_ALLOC) {
		curr->status = STATUS_FREE;
		curr = curr->next;
	}
}

void *os_calloc(size_t nmemb, size_t size)
{
	/* TODO: Implement os_calloc */
	return NULL;
}

void *os_realloc(void *ptr, size_t size)
{
	/* TODO: Implement os_realloc */
	return NULL;
}
