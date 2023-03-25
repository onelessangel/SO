// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <internal/io.h>
#include <string.h>

int puts(const char *s)
{
	size_t s_len = strlen(s);

	if (write(1, s, s_len) < 0) {
		return EOF;
	}

	if (write(1, "\n", 1) < 0) {
		return EOF;
	}

	return s_len + 1;
}
