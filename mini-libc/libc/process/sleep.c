// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <time.h>
#include <internal/syscall.h>
#include <internal/types.h>

unsigned int sleep(unsigned int seconds)
{
	struct timespec ts_req = {seconds, 0};
	struct timespec ts_rem;

	return nanosleep(&ts_req, &ts_rem) == 0 ? 0 : ts_rem.tv_sec;
}