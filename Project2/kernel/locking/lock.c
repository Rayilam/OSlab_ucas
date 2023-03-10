#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <os/irq.h>
#include <atomic.h>
#include <printk.h>

mutex_lock_t mlocks[LOCK_NUM];

void init_locks(void)
{
    /* TODO: [p2-task2] initialize mlocks */
    for (int i = 0; i < LOCK_NUM; i++)
    {
        spin_lock_init(&mlocks[i].lock);
        list_init(&mlocks[i].block_queue);
        mlocks[i].allocated = 0;
    }
}

void spin_lock_init(spin_lock_t *lock)
{
    /* TODO: [p2-task2] initialize spin lock */
    lock->status = UNLOCKED;
}

int spin_lock_try_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] try to acquire spin lock */
    int status = atomic_swap_d(LOCKED, (ptr_t)&lock->status);
    return status;
}

void spin_lock_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] acquire spin lock */
    while (spin_lock_try_acquire(lock) == LOCKED)
        ;
}

void spin_lock_release(spin_lock_t *lock)
{
    /* TODO: [p2-task2] release spin lock */
    lock->status = UNLOCKED;
}

int do_mutex_lock_init(int key)
{
    disable_preempt();
    /* TODO: [p2-task2] initialize mutex lock */
    // key has been allocated with a lock
    for (int i = 0; i < LOCK_NUM; i++)
    {
        if (mlocks[i].key == key)
        {
            mlocks[i].allocated++;
            logging(LOG_INFO, "mlock", "key=%d, lock#%d found\n", key, i);
            enable_preempt();
            return i;
        }
    }
    // allocate a new lock
    for (int i = 0; i < LOCK_NUM; i++)
    {
        if (mlocks[i].allocated == 0)
        {
            mlocks[i].allocated = 1;
            mlocks[i].key = key;
            logging(LOG_INFO, "mlock", "key=%d, lock#%d allocated\n", key, i);
            enable_preempt();
            return i;
        }
    }
    // allocate failed
    enable_preempt();
    return -1;
}

void do_mutex_lock_acquire(int mlock_idx)
{
    disable_preempt();
    /* TODO: [p2-task2] acquire mutex lock */
    logging(LOG_INFO, "mlock", "acquire lock#%d...\n", mlock_idx);
    if (atomic_swap_d(LOCKED, (ptr_t)&mlocks[mlock_idx].lock.status) == UNLOCKED)
    {
        logging(LOG_INFO, "mlock", "...success\n");
        enable_preempt();
    }
    else
    {
        logging(LOG_INFO, "mlock", "...failed, block\n");
        enable_preempt();
        do_block(current_running, &mlocks[mlock_idx].block_queue);
    }
}

void do_mutex_lock_release(int mlock_idx)
{
    disable_preempt();
    /* TODO: [p2-task2] release mutex lock */
    logging(LOG_INFO, "mlock", "release lock#%d...\n", mlock_idx);
    if (list_is_empty(&mlocks[mlock_idx].block_queue))
    {
        logging(LOG_INFO, "mlock", "success\n");
        mlocks[mlock_idx].lock.status = UNLOCKED;
    }
    else
    {
        logging(LOG_INFO, "mlock", "success, unblock a process from queue\n");
        do_unblock(&mlocks[mlock_idx].block_queue);
    }
    enable_preempt();
}
