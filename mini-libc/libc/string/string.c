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

	size_t i = strlen(destination);
	size_t j = 0;

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

	while (str1[i] != '\0' && str1[i] == str2[i]) {
		i++;
	}

	return (unsigned char)str1[i] - (unsigned char)str2[i];
}

int strncmp(const char *str1, const char *str2, size_t len)
{
	size_t i = 0;

	while (str1[i] != '\0' && str1[i] == str2[i] && i < len - 1) {
		i++;
	}

	return (unsigned char)str1[i] - (unsigned char)str2[i];
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
	while (*str != '\0' && *str != c) {
		str++;
	}

	// check c == '\0'
	if (*str == c) {
		return str;
	}

	return NULL;
}

char *strrchr(const char *str, int c)
{
	char *last = NULL;

	while (*str != '\0') {
		if (*str == c) {
			last = str;
		}

		str++;
	}

	// check c == '\0'
	if (*str == c) {
		last = str;
	}

	return last;
}

char *strstr(const char *haystack, const char *needle)
{
	if (strlen(needle) == 0) {
		return haystack;
	}

	size_t i;

	while (*haystack != '\0') {
		i = 0;

		while (needle[i] != '\0' && haystack[i] == needle[i]) {
			i++;
		}

		if (needle[i] == '\0') {
			return haystack;
		}

		haystack++;
	}
	
	return NULL;
}

char *strrstr(const char *haystack, const char *needle)
{
	size_t needle_len = strlen(needle);

	if (needle_len == 0) {
		return haystack;
	}

	char *prev_ptr = NULL;
	char *ptr = NULL;

	while (*haystack != '\0') {
		ptr = strstr(haystack, needle);

		if (ptr == NULL) {
			return prev_ptr;
		}

		prev_ptr = ptr;
		haystack += needle_len;
	}

	return NULL;
}

void *memcpy(void *destination, const void *source, size_t num)
{
	char *dest = destination;
	char *src = source;

	while (num--) {
		*dest++ = *src++;
	}

	return destination;
}

void *memmove(void *destination, const void *source, size_t num)
{
	char *dest = destination;
	char *src = source;

	if (dest < src) {
		return memcpy(destination, source, num);
	}

	// memory overlaps and destination is after source
	dest = dest + (num - 1);
	src = src + (num - 1);

	while (num--) {
		*dest-- = *src--;
	}

	return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
	const char *str1 = ptr1;
	const char *str2 = ptr2;

	return strncmp(str1, str2, num);
}

void *memset(void *source, int value, size_t num)
{
	char *src = source;

	while (num--) {
		*src++ = value;
	}

	return source;
}
