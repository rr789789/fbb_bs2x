#include "los_memory.h"
#if (defined(LOSCFG_MEM_TASK_STAT))
#include "../mem/bestfit_little/los_memory_internal.h"
#define HEAP_CAST(t, exp)       ((t)(exp))

VOID LOS_PoolInfoGet(VOID *pool)
{
    LosHeapNode *node = NULL;
    UINT32 task_id[15] = { 0 }, task_heap_used[15] = { 0 };
    UINT32 task_size = 0;
    bool mark = false;

    struct LosHeapManager *heapMan = HEAP_CAST(struct LosHeapManager *, pool);
    if (heapMan == NULL) {
        return;
    }
    PRINT_INFO("<--- all node used info. --->\r\n");
    PRINT_INFO("node id      node size    node addr     node owner\n");
    node = heapMan->head;
    while (node != NULL) {
        TSK_INFO_S task_info = { 0 };
        if (LOS_TaskInfoGet(NODE_TASKID(node), &task_info) == LOS_OK) {
            PRINT_INFO("0x%-8x   0x%-8x   0x%-8x    %s\n",
                       NODE_TASKID(node), NODE_SIZE(node), NODE_DATA(node), task_info.acName);
        } else {
            PRINT_INFO("0x%-8x   0x%-8x   0x%-8x    unKnown.\n",
                       NODE_TASKID(node), NODE_SIZE(node), NODE_DATA(node));
        }
        for (UINT32 i = 0; i < task_size; i++) {
            if (task_id[i] == NODE_TASKID(node)) {
                task_heap_used[i] += NODE_SIZE(node);
                mark = true;
                break;
            }
        }
        if (mark == false) {
            task_id[task_size] = NODE_TASKID(node);
            task_heap_used[task_size] += NODE_SIZE(node);
            task_size++;
        }
        node = ((heapMan->tail == node) ? NULL : (LosHeapNode *)(UINTPTR)(NODE_DATA(node) + NODE_SIZE(node)));
        if (heapMan->tail == node) {
            break;
        }
        mark = false;
    }
    PRINT_INFO("<--- total task used info. --->\r\n");
    PRINT_INFO("task id      task used     task name\n");
    for (UINT32 j = 0; j < task_size; j++) {
        TSK_INFO_S task_info = { 0 };
        if (LOS_TaskInfoGet(task_id[j], &task_info) == LOS_OK) {
            PRINT_INFO("0x%-8x   0x%-8x    %s\n",
                       task_id[j], task_heap_used[j], task_info.acName);
        } else {
            PRINT_INFO("0x%-8x   0x%-8x    unKnown.\r\n",
                       task_id[j], task_heap_used[j]);
        }
    }
    return;
}
#endif
