#include <common.h>
#include <asm.h>
#include <os/bios.h>
#include <os/task.h>
#include <os/string.h>
#include <os/loader.h>
#include <type.h>
#include <os/console.h>
#include <os/turn.h>

#define VERSION_BUF 64
#define TASK_NUM_LOC 0x502001fa
#define TASK_INFO_LOC (TASK_NUM_LOC - 8)
#define BATCH_NUM_LOC (TASK_INFO_LOC - 2)
#define BATCH_INFO_P_LOC (BATCH_NUM_LOC - 8)

int version = 2; // version must between 0 and 9
char buf[VERSION_BUF];

// tasks
task_info_t apps[APP_MAXNUM];
short appnum;
task_info_t batchs[BATCH_MAXNUM];
short batchnum;

static int bss_check(void)
{
    for (int i = 0; i < VERSION_BUF; ++i)
    {
        if (buf[i] != 0)
        {
            return 0;
        }
    }
    return 1;
}

static void init_bios(void)
{
    volatile long (*(*jmptab))() = (volatile long (*(*))())BIOS_JMPTAB_BASE;

    jmptab[CONSOLE_PUTSTR] = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ] = (long (*)())sd_read;
}

static void init_task_info(void)
{
    // TODO: [p1-task4] Init 'tasks' array via reading app-info sector
    // NOTE: You need to get some related arguments from bootblock first
    // load pointer from TASK_INFO_LOC
    appnum = batchnum = 0;
    long phyaddr = *((long *)TASK_INFO_P_LOC);
    short tasknum = *((short *)TASK_NUM_LOC);
    // read img to some random memory
    task_info_t *task = (task_info_t *)load_img(APP_MEM_BASE, phyaddr,
                                                sizeof(task_info_t) * tasknum, FALSE);
    for (int i = 0; i < tasknum; i++)
    {
        if (task->type == APP)
            apps[appnum++] = *task++;
        else
            batchs[batchnum++] = *task++;
    }
}
static void app_list(void)
{
    bios_putstr("App list:\n\r");
    for (int i = 0; i < appnum; i++)
    {
        CONSOLE_PRINT("\t________________________________\n\r", apps[i].name);
    }
    bios_putstr("Batch list:\n\r");
    for (int i = 0; i < batchnum; i++)
    {
        CONSOLE_PRINT("\t________________________________\n\r", batchs[i].name);
    }
}
int main(void)
{
    // Check whether .bss section is set to zero
    int check = bss_check();

    // Init jump table provided by BIOS (ΦωΦ)
    init_bios();

    // Init task information (〃'▽'〃)
    init_task_info();

    // Output 'Hello OS!', bss check result and OS version
    bios_putstr("Hello OS!\n\r");

    buf[0] = check ? 't' : 'f';
    buf[1] = version + '0';
    buf[2] = '\0';
    CONSOLE_PRINT("bss check=_, version=_\n\r", buf);

    z_st(tasknum, 10, buf, VERSION_BUF, 0);
    CONSOLE_PRINT("tasknum=__\n\r", buf);
    z_st(batchnum, 10, buf, VERSION_BUF, 0);
    CONSOLE_PRINT("batchnum=__\n\r", buf);

    // execute batch(s)
    for (int i = 0; i < batchnum; i++)
        if (batchs[i].execute_on_load)
            run_batch(i);

    // TODO: Load tasks by either task id [p1-task3] or task name [p1-task4],
    //   and then execute them.
    while (1)
    {
        bios_putstr("\n\r> ");

        CONSOLE_GETLINE(buf, VERSION_BUF);
        strip(buf);
        if (strcmp(buf, "help") == 0)
        {
            app_list();
            continue;
        }
        if (strncmp(buf, "batch ", 5) == 0)
        {
            lstrip(buf + 6);
            int batchid = get_taskid_by_name(buf + 6, BATCH);
            if (batchid < 0)
                console_print("No such batch: ________________________________\n\r", buf + 6);
            else
                run_batch(batchid);
            continue;
        }

        int appid = get_taskid_by_name(buf, APP);
        if (appid < 0)
        {
            console_print("No such app: ________________________________\n\r", buf);
            continue;
        }

        z_st(appid, 10, buf, VERSION_BUF, 0);
        CONSOLE_PRINT("appid=__,", buf);
        z_st(apps[appid].entrypoint, 16, buf, VERSION_BUF, 8);
        CONSOLE_PRINT(" entrypoint=__________,", buf);
        z_st(apps[appid].phyaddr, 16, buf, VERSION_BUF, 8);
        CONSOLE_PRINT(" phyaddr=__________,", buf);
        z_st(apps[appid].size, 16, buf, VERSION_BUF, 8);
        CONSOLE_PRINT(" size=__________\n\r", buf);

        CONSOLE_PRINT("Loading user app: ________________________________...", apps[appid].name);

        int (*app)() = (int (*)())load_task_img(appid, APP);
        if (app == 0)
            bios_putstr(" error, abort\n\r");
        else
        {
            bios_putstr(" loaded, running\n\r");
            app();
            bios_putstr("Finished\n\r");
        }
    }

    // Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
    while (1)
    {
        asm volatile("wfi");
    }

    return 0;
}
