// SPDX-License-Identifier: BSD-3-Clause

#include <string.h>

char *strcpy(char *destination, const char *source)
{
	if (destination == NULL || source == NULL) {
		return NULL;
	}

	size_t i = 0;

	while (source[i] != '\0') {
		destination[i] = source[i];
		i++;
	}

	destination[i] = '\0';

	return destination;
}

char *strncpy(char *destination, const char *source, size_t len)
{
	if (destination == NULL || source == NULL) {
		return NULL;
	}

	size_t i = 0;

	for (; i < len; i++) {
		if (source[i] == '\0') {
			break;
		}

		destination[i] = source[i];
	}

	for (; i < len; i++) {
		destination[i] = '\0';
	}

	return destination;
}

char *strcat(char *destination, const char *source)
{
	if (destination == NULL || source == NULL) {
		return NULL;
	}

	size_t i = 0, j = 0;

	while (destination[i] != '\0') {
		i++;
	}

	while (source[j] != '\0') {
		destination[i++] = source[j++];
	}

	destination[i] = '\0';

	return destination;
}

char *strncat(char *destination, const char *source, size_t len)
{
	if (destination == NULL || source == NULL) {
		return NULL;
	}

	size_t i = strlen(destination);
	size_t j = 0;

	for (; j < len; i++, j++) {
		destination[i] = source[j];
	}

	destination[i] = '\0';

	return destination;
}

int strcmp(const char *str1, const char *str2)
{
	size_t i = 0;
	int res;

	while (str1[i] != '\0' || str2[i] != '\0') {
		res = (int)str1[i] - (int)str2[i];

		if (res != 0) {
			break;
		}
		// if (*ptr1 == *ptr2) {
		// 	ptr1++;
		// 	ptr2++;
		// 	continue;
		// }
	}

	return res;
}

int strncmp(const char *str1, const char *str2, size_t len)
{
	/* TODO: Implement strncmp(). */
	return -1;
}

size_t strlen(const char *str)
{
	size_t i = 0;

	for (; *str != '\0'; str++, i++)
		;

	return i;
}

char *strchr(const char *str, int c)
{
	/* TODO: Implement strchr(). */

	return NULL;
}

char *strrchr(const char *str, int c)
{
	/* TODO: Implement strrchr(). */

	return NULL;
}

char *strstr(const char *haystack, const char *needle)
{
	/* TODO: Implement strstr(). */
	return NULL;
}

char *strrstr(const char *haystack, const char *needle)
{
	/* TODO: Implement strrstr(). */
	return NULL;
}

void *memcpy(void *destination, const void *source, size_t num)
{
	/* TODO: Implement memcpy(). */

	return destination;
}

void *memmove(void *destination, const void *source, size_t num)
{
	/* TODO: Implement memmove(). */

	return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
	/* TODO: Implement memcmp(). */
	return -1;
}

void *memset(void *source, int value, size_t num)
{
	/* TODO: Implement memset(). */

	return source;
}
