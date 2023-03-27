// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <fcntl.h>
#include <internal/syscall.h>
#include <errno.h>

int stat(const char *restrict path, struct stat *restrict buf)
{
	// valid information if success, -error otherwise
	int info = syscall(__NR_stat, path, buf);

	if (info < 0) {
		errno = -info;
		return -1;
	}

	return 0;
}
