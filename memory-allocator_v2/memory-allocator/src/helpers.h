/* SPDX-License-Identifier: BSD-3-Clause */

#pragma once

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>

#define DIE(assertion, call_description)						\
	do {														\
		if (assertion) {										\
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);	\
			perror(call_description);							\
			exit(errno);										\
		}														\
	} while (0)

/* Structure to hold memory block metadata */
struct block_meta {
	size_t size;
	int status;
	struct block_meta *next;
};

// struct block_meta *request_space(struct block_meta *last, size_t size);
struct block_meta *request_space(struct block_meta **base, struct block_meta *last, size_t size);
bool block_is_usable(struct block_meta *block, size_t size);
// struct block_meta *find_free_block(struct block_meta *base, struct block_meta **last, size_t size);
// struct block_meta *get_free_block(struct block_meta *base, size_t size);
struct block_meta *get_best_fit(struct block_meta *base, struct block_meta **last, size_t size);
void split_block(struct block_meta *block, size_t size, size_t nmemb);
void coalesce_blocks(struct block_meta *base);
void init_heap(struct block_meta **base);
struct block_meta *get_block_ptr(void *ptr);

/* Block metadata status values */
#define STATUS_FREE   0
#define STATUS_ALLOC  1
#define STATUS_MAPPED 2

/* Alignment - https://moss.cs.iit.edu/cs351/slides/slides-malloc.pdf */
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/* Sizes */
#define MMAP_THRESHOLD	(128 * 1024)
#define METADATA_SIZE	ALIGN((sizeof(struct block_meta)))

#define MALLOC_NMEMB	1