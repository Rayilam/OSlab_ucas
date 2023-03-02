#ifndef __INCLUDE_TASK_H__
#define __INCLUDE_TASK_H__

#include <type.h>

#define TASK_MEM_BASE 0x52000000
#define TASK_MAXNUM 16
#define TASK_SIZE 0x10000
/* TODO: [p1-task4] implement your own task_info_t! */
/*
//typedef struct {
    //char name[32];
    //uint64_t entrypoint;
    //int size;
    //int phyaddr;
//} task_info_t;
*/
/*
//extern task_info_t tasks[TASK_MAXNUM];
//extern short tasknum;
//int get_taskid_by_name(char *taskname);
//#endif
*/

#define BATCH_MEM_BASE (APP_MEM_BASE + APP_MAXNUM * APP_SIZE)
#define BATCH_MAXNUM 16

// task type
typedef enum
{
    APP,
    BATCH
} task_type_t;

// task info
typedef struct
{
    char name[32];
    task_type_t type;
    int size;
    int phyaddr;
    uint64_t entrypoint;
    int execute_on_load;
} task_info_t;

extern task_info_t apps[APP_MAXNUM];
extern short appnum;
extern task_info_t batchs[BATCH_MAXNUM];
extern short batchnum;

int get_taskid_by_name(char *name, task_type_t type);
int run_batch(int batchid);

#endif