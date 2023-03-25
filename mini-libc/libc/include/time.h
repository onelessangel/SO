/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef __TIME_H__
#define __TIME_H__	1

#ifdef __cplusplus
extern "C" {
#endif

int nanosleep(const struct timespec *req, struct timespec *rem);

#ifdef __cplusplus
}
#endif

#endif
