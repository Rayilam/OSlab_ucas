#include <asm.h>
#include <assert.h>
#include <asm/unistd.h>
#include <common.h>
#include <csr.h>
#include <os/irq.h>
#include <os/kernel.h>
#include <os/loader.h>
#include <os/lock.h>
#include <os/mm.h>
#include <os/sched.h>
#include <os/string.h>
#include <os/task.h>
#include <os/time.h>
#include <os/thread.h>
#include <printk.h>
#include <screen.h>
#include <sys/syscall.h>
#include <type.h>

#define TASK_NUM_LOC 0x502001fa
#define TASK_INFO_P_LOC (TASK_NUM_LOC - 8)

// tasks
task_info_t apps[APP_MAXNUM];
short appnum;
task_info_t batchs[BATCH_MAXNUM];
short batchnum;

static void init_jmptab(void)
{
    volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

    jmptab[CONSOLE_PUTSTR] = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ] = (long (*)())sd_read;
    jmptab[QEMU_LOGGING] = (long (*)())qemu_logging;
    jmptab[SET_TIMER] = (long (*)())set_timer;
    jmptab[READ_FDT] = (long (*)())read_fdt;
    jmptab[MOVE_CURSOR] = (long (*)())screen_move_cursor;
    jmptab[PRINT] = (long (*)())printk;
    jmptab[YIELD] = (long (*)())do_scheduler;
    jmptab[MUTEX_INIT] = (long (*)())do_mutex_lock_init;
    jmptab[MUTEX_ACQ] = (long (*)())do_mutex_lock_acquire;
    jmptab[MUTEX_RELEASE] = (long (*)())do_mutex_lock_release;
}

static void init_task_info(void)
{
    // Init 'tasks' array via reading app-info sector
    // load pointer from TASK_INFO_P_LOC
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

static void init_pcb0(void)
{
    // set pid0_pcb.kernel_sp
    pid0_pcb.kernel_sp -= sizeof(regs_context_t);

    // initialize 'current_running'
    current_running = &pid0_pcb;
    asm volatile(
        "add tp, %0, zero\n\r"
        :
        : "r"(current_running));
}

static void init_syscall(void)
{
    // initialize system call table.
    // see arch/riscv/include/asm/unistd.h
    syscall[SYSCALL_EXEC] = (long (*)())do_exec;
    syscall[SYSCALL_EXIT] = (long (*)())do_exit;
    syscall[SYSCALL_SLEEP] = (long (*)())do_sleep;
    syscall[SYSCALL_KILL] = (long (*)())do_kill;
    syscall[SYSCALL_WAITPID] = (long (*)())do_waitpid;
    syscall[SYSCALL_PS] = (long (*)())do_process_show;
    syscall[SYSCALL_GETPID] = (long (*)())do_getpid;
    syscall[SYSCALL_YIELD] = (long (*)())do_scheduler;
    syscall[SYSCALL_WRITE] = (long (*)())screen_write;
    syscall[SYSCALL_READCH] = (long (*)())bios_getchar;
    syscall[SYSCALL_CURSOR] = (long (*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH] = (long (*)())screen_reflush;
    syscall[SYSCALL_CLEAR] = (long (*)())screen_clear;
    syscall[SYSCALL_CURSOR_R] = (long (*)())screen_move_cursor_r;
    syscall[SYSCALL_GET_TIMEBASE] = (long (*)())get_time_base;
    syscall[SYSCALL_GET_TICK] = (long (*)())get_ticks;
    syscall[SYSCALL_LOCK_INIT] = (long (*)())do_mutex_lock_init;
    syscall[SYSCALL_LOCK_ACQ] = (long (*)())do_mutex_lock_acquire;
    syscall[SYSCALL_LOCK_RELEASE] = (long (*)())do_mutex_lock_release;
    // syscall[SYSCALL_SHOW_TASK]     = (long (*)()) do_process_show;  // FIXME?
    syscall[SYSCALL_BARR_INIT] = (long (*)())do_barrier_init;
    syscall[SYSCALL_BARR_WAIT] = (long (*)())do_barrier_wait;
    syscall[SYSCALL_BARR_DESTROY] = (long (*)())do_barrier_destroy;
    syscall[SYSCALL_COND_INIT] = (long (*)())do_condition_init;
    syscall[SYSCALL_COND_WAIT] = (long (*)())do_condition_wait;
    syscall[SYSCALL_COND_SIGNAL] = (long (*)())do_condition_signal;
    syscall[SYSCALL_COND_BROADCAST] = (long (*)())do_condition_broadcast;
    syscall[SYSCALL_COND_DESTROY] = (long (*)())do_condition_destroy;
    // syscall[SYSCALL_MBOX_OPEN]     = (long (*)()) do_mbox_open;
    // syscall[SYSCALL_MBOX_CLOSE]    = (long (*)()) do_mbox_close;
    // syscall[SYSCALL_MBOX_SEND]     = (long (*)()) do_mbox_send;
    // syscall[SYSCALL_MBOX_RECV]     = (long (*)()) do_mbox_recv;
    syscall[SYSCALL_THREAD_CREATE] = (long (*)())thread_create;
    syscall[SYSCALL_THREAD_JOIN] = (long (*)())thread_join;
    syscall[SYSCALL_THREAD_EXIT] = (long (*)())thread_exit;
}

void init_shell(void)
{
    /* S_CORE
        init_pcb(0, 0, 0, 0, 0);
    */
    init_pcb("shell", 0, NULL);
}

int main(void)
{
    // Init jump table provided by BIOS (??????)
    init_jmptab();

    // Init task information (???'???'???)
    init_task_info();

    // set log level
    set_loglevel(LOG_DEBUG);

    // Init Process Control Blocks |???'-'???) ???
    init_pcb0();
    logging(LOG_INFO, "init", "PCB0 initialization succeeded.\n");
    init_shell();
    logging(LOG_INFO, "init", "Shell initialization succeeded.\n");

    // Read CPU frequency (?????????-)_
    time_base = bios_read_fdt(TIMEBASE);

    // Init lock mechanism o(??^???)o
    init_locks();
    init_barriers();
    init_conditions();
    logging(LOG_INFO, "init", "Lock mechanism initialization succeeded.\n");

    // Init interrupt (^_^)
    init_exception();
    logging(LOG_INFO, "init", "Interrupt processing initialization succeeded.\n");

    // Init system call table (0_0)
    init_syscall();
    logging(LOG_INFO, "init", "System call initialized successfully.\n");

    // Init screen (QAQ)
    init_screen();
    logging(LOG_INFO, "init", "SCREEN initialization succeeded.\n");

    // Setup timer interrupt and enable all interrupt globally
    // NOTE: The function of sstatus.sie is different from sie's
    enable_interrupt();
    bios_set_timer(get_ticks() + TIMER_INTERVAL);
    logging(LOG_INFO, "init", "Timer initialization succeeded & interrupt enabled.\n");

    // Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
    while (1)
    {
        // If you do non-preemptive scheduling, it's used to surrender control
        // do_scheduler();

        // If you do preemptive scheduling, they're used to enable CSR_SIE and wfi
        enable_preempt();
        asm volatile("wfi");
    }

    return 0;
}
