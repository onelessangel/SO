// Copyright Teodora Stroe 331CA 2023

#include "osmem.h"
#include "helpers.h"

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
			global_base = request_space(global_base, MMAP_THRESHOLD - METADATA_SIZE);

			split_block(global_base, aligned_size);
			global_base->status = STATUS_ALLOC;

			return (void *)(global_base + 1);
		}

		coalesce_free_blocks(global_base);

		block = get_best_fit(global_base, &last, aligned_size);

		if (block == NULL) {
			if (last->status == STATUS_FREE) {
				sbrk(aligned_size - last->size);
				last->status = STATUS_ALLOC;
				last->size = aligned_size;

				return (void *)(last + 1);
			}

			block = request_space(global_base, aligned_size);

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
			global_base = request_space(global_base, MMAP_THRESHOLD - METADATA_SIZE);

			split_block(global_base, aligned_size);
			global_base->status = STATUS_ALLOC;

			memset((void *)(global_base + 1), 0, aligned_size);
			return (void *)(global_base + 1);
		}

		coalesce_free_blocks(global_base);

		block = get_best_fit(global_base, &last, aligned_size);

		if (block == NULL) {
			if (last->status == STATUS_FREE) {
				sbrk(aligned_size - last->size);
				last->status = STATUS_ALLOC;
				last->size = aligned_size;

				memset((void *)(last + 1), 0, aligned_size);
				return (void *)(last + 1);
			}

			block = request_space(global_base, aligned_size);

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

static void *alloc_new_free_old(void *old_block_ptr, size_t alloc_size, size_t copy_size)
{
	struct block_meta *old_block = get_block_ptr(old_block_ptr);
	struct block_meta *new_block = get_block_ptr(os_malloc(alloc_size));
	memcpy(new_block + 1, old_block + 1, copy_size);
	os_free(old_block_ptr);

	return (void *)(new_block + 1);
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

	if (old_block->size == aligned_size) {
		return ptr;
	}

	if (aligned_size < old_block->size) {
		if (old_block->status == STATUS_MAPPED) {
			return alloc_new_free_old(ptr, aligned_size, aligned_size);
		}

		split_block(old_block, aligned_size);

		return (void *)(old_block + 1);
	}

	if (old_block->status == STATUS_MAPPED) {
		return alloc_new_free_old(ptr, aligned_size, old_block->size);
	}

	size_t extra_size = aligned_size - old_block->size - METADATA_SIZE;

	coalesce_free_blocks(global_base);

	if (old_block->next == NULL) {
		if (request_space(global_base, extra_size) == NULL) {
			return NULL;
		}

		merge_block(old_block);

		return (void *)(old_block + 1);
	}

	if (block_is_usable(old_block->next, extra_size)) {
		split_block(old_block->next, extra_size);
		merge_block(old_block);

		return (void *)(old_block + 1);
	}

	return alloc_new_free_old(ptr, aligned_size, old_block->size);
}
