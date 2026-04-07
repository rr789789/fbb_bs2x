/**
 * Mock soc_osal.h for PC-side unit tests.
 * Shadows the real header when compiled with -Itests before the SDK include path.
 */
#ifndef MOCK_SOC_OSAL_H
#define MOCK_SOC_OSAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define OSAL_SUCCESS  0
#define OSAL_FAILURE  1

/* Message queue */
int osal_msg_queue_create(const char *name, unsigned long queue_len,
                          unsigned long *queue_id, unsigned int flags,
                          unsigned short max_msgsize);
int osal_msg_queue_write_copy(unsigned long queue_id, void *buffer,
                              unsigned int bufsize, unsigned int timeout);
int osal_msg_queue_read_copy(unsigned long queue_id, void *buffer,
                             unsigned int *bufsize, unsigned int timeout);
int osal_msg_queue_delete(unsigned long queue_id);

/* Debug print */
void osal_printk(const char *fmt, ...);

/* Memory (not used by service layer directly, but included for completeness) */
static inline void *osal_vmalloc(unsigned long size) { return malloc(size); }
static inline void  osal_vfree(void *p)              { free(p); }

#endif /* MOCK_SOC_OSAL_H */
