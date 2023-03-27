// SPDX-License-Identifier: BSD-3-Clause

#include <sys/mman.h>
#include <internal/syscall.h>
#include <fcntl.h>
#include <errno.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	long int mapped_area = syscall(__NR_mmap, addr, length, prot, flags, fd, offset);

	if (mapped_area < 0) {
		errno = -mapped_area;

		return MAP_FAILED;
	}

	return (void *)mapped_area;
}

void *mremap(void *old_address, size_t old_size, size_t new_size, int flags)
{
	long int mapped_area = syscall(__NR_mremap, old_address, old_size, new_size, flags);

	if (mapped_area < 0) {
		errno = mapped_area;
		return MAP_FAILED;
	}

	return (void *)mapped_area;
}

int munmap(void *addr, size_t length)
{
	long int mapped_area = syscall(__NR_munmap, addr, length);

	if (mapped_area < 0) {
		errno = -mapped_area;
		return -1;
	}

	return 0;
}
