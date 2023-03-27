// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

int brk(void *addr)
{
	long int outcome = syscall(__NR_brk, addr);

	if (outcome < 0) {
		errno = -outcome;

		return -1;
	}

	return 0;
}