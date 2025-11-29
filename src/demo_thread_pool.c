/* FILE: src/demo_thread_pool.c */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>

#define THREAD_POOL_SIZE 4
#define TASK_QUEUE_SIZE 10

struct task {
    void (*function)(void *);
    void *arg;
    int task_id;
};

K_MSGQ_DEFINE(task_queue, sizeof(struct task), TASK_QUEUE_SIZE, 4);

/* Examples tasks */
void fast_task(void *arg){
    int task_id = (int)arg;
    printk("[task-%d] fast task executing\n", task_id);
    k_msleep(100);
    printk("[task-%d] fast task done\n", task_id);
}

void medium_task(void *arg){
    int task_id = (int)arg;
    printk("[task-%d] mediumm task executing\n", task_id);
    k_msleep(300);
    printk("[task-%d] medium task done\n", task_id);
}

void slow_task(void *arg){
    int task_id = (int)arg;
    printk("[task-%d] slow task executing\n", task_id);
    k_msleep(500);
    printk("[task-%d] slow task done\n", task_id);
}

void very_slow_task(void *arg){
    int task_id = (int)arg;
    printk("[task-%d] very slow task executing\n", task_id);
    k_msleep(1000);
    printk("[task-%d] very slow task done\n", task_id);
}
/* Worker thread function */
void worker_pool_thread(void *p1, void *p2, void *p3){
    int worker_id = (int)p1;
    struct task t;
    printk("[WORKER-%d] started and ready for work\n",worker_id);
    while(1){
        if(k_msgq_get(&task_queue, &t, K_FOREVER)==0){
            printk("[WORKER-%d] Got task %d\n", worker_id, t.task_id);
            t.function(t.arg);
            printk("[WORKER-%d] Completed task %d\n", worker_id, t.task_id);
        }
    }
}

void task_dispatcher(void *p1, void *p2, void *p3){
    struct task t;
    int task_count = 0;
    void (*task_types[])(void *) = {fast_task, medium_task, slow_task, very_slow_task};
    const char *task_names[] = {"fast", "medium", "slow", "very_slow"};

    k_msleep(1000); // Wait for workers to be ready
    printk("[DISPATCHER] Starting to dispatch tasks\n");
    while(1){
        int type = sys_rand32_get() % 4;

        t.function = task_types[type];
        t.arg = (void *)task_count;
        t.task_id =task_count;

        printk("[DISPATCHER] Dispatching %s task %d (QUEUE: %u/%u)\n",
        task_names[type], task_count, 
        k_msgq_num_used_get(&task_queue),
        TASK_QUEUE_SIZE);

        if(k_msgq_put(&task_queue, &t, K_NO_WAIT) !=0){
            printk("[DISPATCHER] Task queue full,task %d dropped \n", task_count);
        }
        task_count++;
        k_msleep(500);
    }
}

K_THREAD_STACK_ARRAY_DEFINE(worker_stacks, THREAD_POOL_SIZE, 1024);
struct k_thread worker_threads[THREAD_POOL_SIZE];

K_THREAD_STACK_DEFINE(dispatcher_stack, 1024);
struct k_thread dispatcher_thread;

int main(void){
    printk("\n=== Thread Pool Demo ===\n\n");

    /* Create worker threads */
    for(int i=0; i<THREAD_POOL_SIZE; i++){
        k_thread_create(&worker_threads[i], worker_stacks[i],
            K_THREAD_STACK_SIZEOF(worker_stacks[i]),
            worker_pool_thread, (void *)i, NULL, NULL,
            5, 0, K_NO_WAIT);
    }

    /* Create dispatcher thread */
    k_thread_create(&dispatcher_thread, dispatcher_stack,
        K_THREAD_STACK_SIZEOF(dispatcher_stack),
        task_dispatcher, NULL, NULL, NULL,
        5, 0, K_NO_WAIT);

    return 0;
}