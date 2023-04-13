Copyright Teodora Stroe 331CA 2023

# Memory Allocator

## 1. FILE GUIDE

```
.
├── helpers.c		-- helper functions         
├── helpers.h		-- helper functions and use ful MACROs
├── libosmem.so
├── Makefile		-- Makefile
├── osmem.c    		-- memory functions implementation
├── osmem.h    		-- memory functions header
└── README.md       -- README
```

## MEMORY FUNCTIONS

### `os_malloc`

- checks if must allocate on heap

	- creates heap head if it does not exist
	- merges free blocks and gets most suitable free block
	- if suitable FREE block doen't exist, checks if it is possible to extend last 
	  block or requests needed space
	- crops FREE block to needed size

- otherwise, allocates with `mmap`

### `os_free`

- performs basic necessity checks
- frees heap space by setting status as STATUS_FREE
- frees mapped space with `munmap`,

### `os_calloc`

- similar to `os_malloc`
- uses pagesize as condition to alloc space on heap
- uses `memset` to initialize memory with `0`

### `os_realloc`

- calls `os_malloc` or `os_free` if necessary
- if the size is smaller than the previously allocated size

	- if old memory was mapped, remaps with new size - new memory block will be smaller
	- if already on heap, crops to correct size

- if the new size is bigger than the previously allocated size

	- if old memory was mapped, remaps with new size - new memory block will be larger
	- otherwise, memory is alread on heap

		- computes extra size needed
		- merges free blocks
		- if there is no next block,
		  requests space and merge old block with the new requested block
		- if next block is FREE and with a suitable size,
		  crops next block to needed size and merges the two blocks
		- otherwise, next block is already occupied,
		  allocates new memory and frees old memory

## HELPER FUNCTIONS

### `request_space` [2]

- allocates more space on heap, starting from given block

### `block_is_usable`

- check if block has STATUS_FREE and has size big enough to use

### `get_best_fit` [3]

- searches for a free, usable block by applying the best fit algorithm

### `split_bloc` [3]

- splits block into 2 blocks:

	- the first one has the needed size
	- the second one has the remaining size

### `coalesce_free_blocks`

- navigates throught the whole heap and merges free blocks

### `merge_block` [1]

- merges given block with the next block

### `get_block_ptr` [2]

- gets block from pointer

## Resources:

[1] https://open-education-hub.github.io/operating-systems/Assignments/Memory%20Allocator/#objectives

[2] https://danluu.com/malloc-tutorial/

[3] https://moss.cs.iit.edu/cs351/slides/slides-malloc.pdf