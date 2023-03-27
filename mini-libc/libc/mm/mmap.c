// SPDX-License-Identifier: BSD-3-Clause

#include <sys/mman.h>
#include <internal/syscall.h>
#include <fcntl.h>
#include <errno.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	// int flag = fcntl(fd, F_GETFD);

	// if ((flags & MAP_SHARED == 0) && (flags & MAP_PRIVATE == 0)) {
	// 	errno = EINVAL;
	// 	return MAP_FAILED;
	// }

	long int mapped_area = syscall(__NR_mmap, addr, length, prot, flags, fd, offset);

	if (mapped_area < 0) {
		errno = -mapped_area;
		// if (errno == EBADF) {
		// 	errno = EINVAL;
		// }

		return -1;
	}

	// if (fcntl(fd, F_GETFD) == -1) {
	// 	// Invalid file descriptor, set errno and return MAP_FAILED
	// 	errno = EBADF;
	// 	munmap(mapped_area, length);

	// 	return MAP_FAILED;
	// }

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
