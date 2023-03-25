// SPDX-License-Identifier: BSD-3-Clause

#include <time.h>
#include <internal/types.h>
#include <internal/syscall.h>
#include <errno.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
	errno = syscall(__NR_nanosleep, req, rem);
	return errno == 0 ? 0 : -1;
}