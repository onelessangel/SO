// SPDX-License-Identifier: BSD-3-Clause

#include "osmem.h"
#include "helpers.h"
// #include "../utils/printf.h"

struct block_meta *global_base = NULL;
struct block_meta *last = NULL;

void *os_malloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}

	size_t aligned_size = ALIGN(size);
	struct block_meta *block;

	if (aligned_size + METADATA_SIZE < MMAP_THRESHOLD) {
		if (global_base == NULL) {
			global_base = request_space(&global_base, NULL, MMAP_THRESHOLD - METADATA_SIZE);

			split_block(global_base, aligned_size);
			global_base->status = STATUS_ALLOC;

			return (void *)(global_base + 1);
		}

		coalesce_blocks(global_base);

		block = get_best_fit(global_base, &last, aligned_size);

		if (block == NULL) {
			if (last->status == STATUS_FREE) {
				sbrk(aligned_size - last->size);
				last->status = STATUS_ALLOC;
				last->size = aligned_size;

				return (void *)(last + 1);
			}

			block = request_space(&global_base, last, aligned_size);

			if (!block) {
				return NULL;
			}

		} else {
			split_block(block, aligned_size);
		}

		block->status = STATUS_ALLOC;

		return (void *)(block + 1);		
	}

	block = (struct block_meta *) mmap(NULL, METADATA_SIZE + aligned_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	block->size = aligned_size;
	block->status = STATUS_MAPPED;
	block->next = NULL;

	if (!global_base) {
		global_base = block;
	}

	return (void *)(block + 1);
}

void os_free(void *ptr)
{
	if (global_base == NULL) {
		return;
	}

	struct block_meta *curr = get_block_ptr(ptr);

	if (curr->status == STATUS_ALLOC) {
		curr->status = STATUS_FREE;
		return;
	}

	if (curr->status == STATUS_MAPPED) {
		munmap(curr, curr->size + METADATA_SIZE);
	}
}

void *os_calloc(size_t nmemb, size_t size)
{
	if (size == 0 || nmemb == 0) {
		return NULL;
	}

	size_t total_size = size * nmemb;
	size_t aligned_size = ALIGN(total_size);

	size_t page_size = getpagesize();
	struct block_meta *block;

	if (aligned_size + METADATA_SIZE < page_size) {
		if (global_base == NULL) {
			global_base = request_space(&global_base, NULL, MMAP_THRESHOLD - METADATA_SIZE);

			split_block(global_base, aligned_size);
			global_base->status = STATUS_ALLOC;

			memset((void *)(global_base + 1), 0, aligned_size);
			return (void *)(global_base + 1);
		}

		coalesce_blocks(global_base);

		block = get_best_fit(global_base, &last, aligned_size);

		if (block == NULL) {
			if (last->status == STATUS_FREE) {
				sbrk(aligned_size - last->size);
				last->status = STATUS_ALLOC;
				last->size = aligned_size;

				memset((void *)(last + 1), 0, aligned_size);
				return (void *)(last + 1);
			}

			block = request_space(&global_base, last, aligned_size);

			if (block == NULL) {
				return NULL;
			}
		} else {
			split_block(block, aligned_size);
		}

		block->status = STATUS_ALLOC;

		memset((void *)(block + 1), 0, aligned_size);
		return (void *)(block + 1);

	}

	block = (struct block_meta *) mmap(NULL, METADATA_SIZE + aligned_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	block->size = aligned_size;
	block->status = STATUS_MAPPED;
	block->next = NULL;

	if (global_base == NULL) {
		global_base = block;
	}

	memset((void *)(block + 1), 0, aligned_size);
	return (void *)(block + 1);
}

void *os_realloc(void *ptr, size_t size)
{
	if (!ptr) {
		return os_malloc(size);
	}

	if (size == 0) {
		os_free(ptr);
		return NULL;
	}

	struct block_meta *old_block = get_block_ptr(ptr);

	if (old_block->status == STATUS_FREE) {
		return NULL;
	}

	size_t aligned_size = ALIGN(size);

	if (old_block->size == size) {
		return ptr;
	}

	if (size < old_block->size) {
		printf("old size: %d\n", old_block->size);
		printf("status alocare: %d\n", old_block->status);
		// split_block(old_block, aligned_size);

		// struct block_meta *temp = (old_block->next)->next;
		// // os_free(old_block->next);
		// old_block->next = temp;

		return (void *)(old_block + 1);
	}

	// coalesce_blocks(global_base);



	return NULL;
}
