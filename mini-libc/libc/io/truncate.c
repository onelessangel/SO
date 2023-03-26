// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <internal/types.h>
#include <errno.h>

int truncate(const char *path, off_t length)
{
	// 0 if success, -error otherwise
	int outcome = syscall(__NR_truncate, path, length);

	if (outcome < 0) {
		errno = -outcome;
		return -1;
	}

	return 0;
}
