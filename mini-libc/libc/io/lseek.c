// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <internal/types.h>
#include <errno.h>

off_t lseek(int fd, off_t offset, int whence)
{
	// offset, -error otherwise
	offset = syscall(__NR_lseek, fd, offset, whence);

	if (offset < 0) {
		errno = -offset;
		return (off_t) -1;
	}

	return offset;
}
