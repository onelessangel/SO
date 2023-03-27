// SPDX-License-Identifier: BSD-3-Clause

#include <internal/mm/mem_list.h>
#include <internal/types.h>
#include <internal/essentials.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

void *malloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}

	void *start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (start == MAP_FAILED) {
		return NULL;
	}

	if (mem_list_add(start, size) == -1) {
		return NULL;
	}

	return start;
}

void *calloc(size_t nmemb, size_t size)
{
	return memset(malloc(size), 0, nmemb);
}

void free(void *ptr)
{
	struct mem_list *list = mem_list_find(ptr);
	munmap(list->start, list->len);
	mem_list_del(ptr);
}

void *realloc(void *ptr, size_t size)
{
	if (ptr == NULL) {
		return malloc(size);
	}

	if (size == 0) {
		free(ptr);
		return NULL;
	}

	struct mem_list *list = mem_list_find(ptr);
	void *new_mem_addr = malloc(size);
	memmove(new_mem_addr, list->start, list->len);

	return new_mem_addr;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	size_t total_size = nmemb * size;

	if (size != 0 && total_size / size != nmemb) {
		errno = ENOMEM;
		return NULL;
	}

	return realloc(ptr, total_size);
}
