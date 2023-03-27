// SPDX-License-Identifier: BSD-3-Clause

#include <fcntl.h>
#include <internal/syscall.h>
#include <internal/types.h>
#include <stdarg.h>
#include <errno.h>

int open(const char *filename, int flags, ...)
{
	va_list valist;
	mode_t mode;

	va_start(valist, flags);
	mode = va_arg(valist, mode_t);
	va_end(valist);

	// fd if success, -error otherwise
	int fd = syscall(__NR_open, filename, flags, mode);

	if (fd < 0) {
		errno = -fd;
		return -1;
	}

	return fd;
}
