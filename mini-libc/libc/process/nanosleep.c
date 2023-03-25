// SPDX-License-Identifier: BSD-3-Clause

#include <time.h>
#include <internal/types.h>
#include <internal/syscall.h>
#include <errno.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
	// 0 if success, -error otherwise
	int outcome = syscall(__NR_nanosleep, req, rem);

	if (outcome < 0) {
		errno = -outcome;
		return -1;
	}

	return 0;
}