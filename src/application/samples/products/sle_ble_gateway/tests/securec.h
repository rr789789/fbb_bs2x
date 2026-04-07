/**
 * Mock securec.h for PC-side unit tests.
 */
#ifndef MOCK_SECUREC_H
#define MOCK_SECUREC_H

#include <stddef.h>

int memcpy_s(void *dest, size_t dest_size, const void *src, size_t count);
int memset_s(void *dest, size_t dest_size, int ch, size_t count);

#endif /* MOCK_SECUREC_H */
