// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <internal/syscall.h>
#include <errno.h>

int fstat(int fd, struct stat *st)
{
	// valid information if success, -error otherwise
	int info = syscall(__NR_fstat, fd, st);

	if (info < 0) {
		errno = -info;
		return -1;
	}

	return 0;
}
