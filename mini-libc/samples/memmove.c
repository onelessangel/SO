// SPDX-License-Identifier: BSD-3-Clause

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static int test_memmove_src_before_dst(void)
{
	char src[128];
	char *dst = src + 16;

	memset(src, 'a', 128);
	memset(dst, 'b', 112);
	memmove(dst, src, 112);

	return dst[0] == 'a' && dst[1] == 'a';
}

int main(void)
{
	/*
	 * dst and src do not overlap.
	 * dst should be set to "Let's test memmove against simple string!\n".
	 */
	char src[] = "Let's test memmove against simple string!\n";
	char dst[100];
	char str2[] = "memmove can be very useful......\n";

	memmove(dst, src, strlen(src));
	dst[strlen(src)] = '\0';

	write(1, dst, strlen(dst));
	puts("hello");

	/*
	 * dst and src overlap.
	 * dst should point to "memmove can be very useful.\n".
	 */
	memmove(str2 + 20, str2 + 15, 11);
	write(1, str2, strlen(str2));

	char my_string[] = "Mama are mere.";

	memmove(my_string +  20, my_string, strlen(my_string));
	puts(my_string);


	

	char source[128];
	char *destination = source + 16;

	memset(source, 'a', 128);
	memset(destination, 'b', 112);
	memmove(destination, source, 112);

	puts(&destination[0]);
	puts(&destination[1]);

	return 0;
}
