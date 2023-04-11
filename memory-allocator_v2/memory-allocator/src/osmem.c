// SPDX-License-Identifier: BSD-3-Clause

#include "osmem.h"
#include "helpers.h"
// #include "../utils/printf.h"

struct block_meta *global_base = NULL;
struct block_meta *last;

void *os_malloc(size_t size)
{
	printf("am intrat in malloc\n");

	if (size <= 0) {
		return NULL;
	}

	size_t aligned_size = ALIGN(size);
	// static bool heap_is_init = false;

	// if (!heap_is_init) {
	// 	init_heap(&global_base);
	// }

	struct block_meta *block;

	if (aligned_size + METADATA_SIZE < MMAP_THRESHOLD) {
		printf_("am ajuns aici\n");
		// block = get_free_block(global_base, )
		// facem request_space de la ultimul block free
		if (global_base == NULL) {
			global_base = request_space(NULL, MMAP_THRESHOLD - METADATA_SIZE);

			split_block(global_base, aligned_size);			
			global_base->status = STATUS_ALLOC;

			last = global_base;

			return (void *)(global_base + 1);
		}

		coalesce_blocks(global_base);

		block = get_free_block(global_base, aligned_size);

		// nu exista destul spatiu liber
		if (block == NULL || block->size < aligned_size) {
			block = request_space(last, MMAP_THRESHOLD - METADATA_SIZE);
		}

		split_block(block, aligned_size);
		block->status = STATUS_ALLOC;
		return (void *)(block + 1);
		// block = request_space(NULL, MMAP_THRESHOLD - METADATA_SIZE);
		// coalesce_blocks(block);

		// split_block(block, aligned_size);
		// block->size = aligned_size;
		// block->status = STATUS_ALLOC;
		// block->next = NULL;

		// if (global_base == NULL) {
		// 	global_base = block;
		// }
		

		// DIE??

		return (void *)(block + 1);
	}

	printf("hello\n");

	block = (struct block_meta *) mmap(NULL, METADATA_SIZE + aligned_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	block->size = aligned_size;
	block->status = STATUS_MAPPED;
	block->next = NULL;

	printf("%d\n", block->size);

	if (global_base == NULL) {
		global_base = block;
	}

	// printf("%d", global_base->size);

	return (void *)(block + 1);
}

void os_free(void *ptr)
{
	printf("am intrat in free\n");

	// struct block_meta *curr = (struct block_meta *) ptr - 1;


	if (global_base == NULL) {
		return;
	}

	struct block_meta *curr = get_block_ptr(ptr);
	printf("%d\n", curr->status);
	printf("%d\n", curr->size);

	printf("buuuuuuuna\n");

	while (curr && curr->status == STATUS_ALLOC) {
		curr->status = STATUS_FREE;
		curr = curr->next;
	}

	// printf("STATUS: %d\n", curr->status);

	if (curr->status == STATUS_MAPPED) {
		printf("sunt aici\n");
		munmap(curr, curr->size + METADATA_SIZE);
		printf("still alive\n");
		// curr->status = STATUS_FREE;
		// printf("status: %d\n", curr->status);
		// curr = curr->next;
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
