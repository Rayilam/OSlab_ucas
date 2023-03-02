#include <os/bios.h>
#include <os/console.h>
#include <os/loader.h>
#include <os/string.h>
#include <os/task.h>
#include <os/turn.h>
/*
int get_taskid_by_name(char *taskname) {
    for (int i=0; i<tasknum; i++) {
        if (strcmp(taskname, tasks[i].name) == 0)
            return i;
    }
    return -1;
}
*/
int get_taskid_by_name(char *name, task_type_t type)
{
    task_info_t *tasks = type == APP ? apps : batchs;
    int num = type == APP ? appnum : batchnum;
    for (int i = 0; i < num; i++)
    {
        if (strcmp(name, tasks[i].name) == 0)
            return i;
    }
    return -1;
}

int run_batch(int batchid)
{
    task_info_t batch = batchs[batchid];

    int err = 0;

    CONSOLE_PRINT(" ===== executing batch: ________________"
                  "________________ =====\n\r",
                  batch.name);

    char *file = (char *)load_task_img(batchid, BATCH);
    char buf[512];
    while (*file)
    {
        int i = 0;
        while (*file && *file != '\n')
            buf[i++] = *file++;
        file += *file == '\n';

        buf[i] = '\0';
        i = strip(buf);
        if (i == 0)
            continue;

        CONSOLE_PRINT(" -> executing ________________________________\n\r", buf);
        int taskid = get_taskid_by_name(buf, APP);
        if (taskid < 0)
        {
            err = 1;
            bios_putstr("Task not found, please check your batch file\n\r");
        }
        else
        {
            int (*task)() = (int (*)())load_task_img(taskid, APP);
            if (task == 0)
            {
                err = 1;
                bios_putstr("Task load error\n\r");
            }
            else
            {
                task();
            }
        }
    }

    CONSOLE_PRINT(" ===== completed batch: ________________"
                  "________________ =====\n\r",
                  batch.name);

    return err;
}