// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <stdarg.h>
#include <errno.h>

int close(int fd)
{
	// 0 if success, -error otherwise
	int outcome = syscall(__NR_close, fd);

	if (outcome < 0) {
		errno = -outcome;
		return -1;
	}
	return 0;
}
