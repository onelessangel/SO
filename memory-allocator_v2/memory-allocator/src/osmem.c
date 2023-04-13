// SPDX-License-Identifier: BSD-3-Clause

#include "osmem.h"
#include "helpers.h"
// #include "../utils/printf.h"

struct block_meta *global_base;
struct block_meta *last;
struct block_meta *realloc_base;
bool mmap_was_used;

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
	mmap_was_used = true;

	return (void *)(block + 1);
}

void os_free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	if (global_base == NULL && !mmap_was_used) {
		return;
	}

	struct block_meta *curr = get_block_ptr(ptr);

	// if (curr == NULL) {
	// return;
	// }

	// printf("status in free: %d\n", curr->status);
	// printf("hello\n");

	if (curr->status == STATUS_ALLOC) {
		// printf("am ajuns aici\n");
		curr->status = STATUS_FREE;
		// printf("am reusit\n");
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
	mmap_was_used = true;

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
	struct block_meta *block;
	struct block_meta *new_block;

	if (old_block->status == STATUS_FREE) {
		return NULL;
	}

	size_t aligned_size = ALIGN(size);

	if (old_block->size == aligned_size) {
		return ptr;
	}

	if (aligned_size < old_block->size) {
		// printf("old size: %d\n", old_block->size);
		// printf("status alocare: %d\n", old_block->status);

		if (old_block->status == STATUS_MAPPED) {
			new_block = get_block_ptr(os_malloc(aligned_size));
			memcpy(new_block + 1, old_block + 1, aligned_size);
			printf("fac free in size < old_size\n");
			os_free(ptr);

			return (void *)(new_block + 1);
		}

		split_block(old_block, aligned_size);

		return (void *)(old_block + 1);
	}

	if (old_block->status == STATUS_MAPPED) {
		new_block = get_block_ptr(os_malloc(aligned_size));
		memcpy(new_block + 1, old_block + 1, old_block->size);
		printf("fac free in STATUS MAPPED\n");
		os_free(ptr);

		return (void *)(new_block + 1);
	}

	size_t extra_size = aligned_size - old_block->size - METADATA_SIZE;

	coalesce_blocks(global_base);

	if (old_block->next == NULL) {
		printf("blocul urmator nu exista\n");
		block = request_space(&global_base, last, extra_size);

		if (block == NULL) {
			return NULL;
		}

		merge_block(old_block);

		return (void *)(old_block + 1);
	}

	if ((old_block->next)->status == STATUS_FREE) {
		printf("blocul urmtor e FREE\n");
		// coalesce_blocks(global_base);  // !!
		// split_block(old_block->next, extra_size);
		merge_block(old_block);

		return (void *)(old_block + 1);
	}

	printf("blocul urmator e alocat\n");
	new_block = get_block_ptr(os_malloc(aligned_size));
	memcpy(new_block + 1, old_block + 1, old_block->size);
	printf("fac free pentru ca urmatorul bloc e alocat\n");
	os_free(ptr);

	return (void *)(new_block + 1);
}
