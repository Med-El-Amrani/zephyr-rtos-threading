/* File : demo_memory_slab.c */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define SLAB_SIZE 320
#define BLOCK_SIZE 64
#define NUM_BLOCKS (SLAB_SIZE / BLOCK_SIZE)
#define NUM_USERS 4

K_MEM_SLAB_DEFINE(my_slab, BLOCK_SIZE, NUM_BLOCKS, 4);

struct data_block {
    uint32_t thread_id;
    uint32_t allocation_num;
    uint32_t timestamp;
    uint8_t data[BLOCK_SIZE - 3 * sizeof(uint32_t)];
};

static int total_allocations = 0;
static int total_failures = 0;
static int blocks_in_use = 0;

K_MUTEX_DEFINE(stats_mutex);

void memory_slab_user(void *p1, void *p2, void *p3){
    int thread_id = (int)p1;
    int my_allocation_count = 0;
    int my_failures = 0;

    printk("[USER-%d] Started\n", thread_id);

    while(my_allocation_count < 5){ // each thread does 5 allocations
        struct data_block *block;

        printk("[USER-%d] Attempting allocation #%d...\n", thread_id, my_allocation_count);

        int ret = k_mem_slab_alloc(&my_slab,  (void **)&block, K_MSEC(2000));

        if(ret == 0){
            /* Success */
            k_mutex_lock(&stats_mutex, K_FOREVER);
            total_allocations++;
            blocks_in_use++;
            k_mutex_unlock(&stats_mutex);

            uint32_t free_bloks = k_mem_slab_num_free_get(&my_slab);
            printk("[USER-%d] Allocation #%d successful. Free blocks left: %d\n",
                   thread_id, my_allocation_count, free_bloks);

            /* Fill in data */
            block->thread_id = thread_id;
            block->allocation_num = my_allocation_count;
            block->timestamp = k_uptime_get_32();

            /* fill data array with pattern*/
            for(int i=0; i < sizeof(block->data);i++){
                block->data[i] = (thread_id * 16 + i) & 0xFF;
            }

            /* Simulate work - different threads hold blocks for different times */
            uint32_t hold_time = 500 + (thread_id * 200);
            printk("[USER-%d] Working, it will hold block for %d ms\n", thread_id, hold_time);
            k_sleep(K_MSEC(hold_time));

            /* Verify data integrity */
            bool data_ok = true;
            for(int i=0; i < sizeof(block->data); i++){
                if(block->data[i] != (uint8_t)((thread_id * 16 + i) & 0xFF)){
                    data_ok = false;
                    break;
                }
            }

            if(data_ok){
                printk("[USER-%d] Data integrity check passed for allocation #%d\n",
                       thread_id, my_allocation_count);
            } else {
                printk("[USER-%d] Data integrity check FAILED for allocation #%d\n",
                       thread_id, my_allocation_count);
            }

            /* Free the block */
            k_mem_slab_free(&my_slab, (void *)block);

            k_mutex_lock(&stats_mutex, K_FOREVER);
            blocks_in_use--;
            k_mutex_unlock(&stats_mutex);

            free_bloks = k_mem_slab_num_free_get(&my_slab);
            printk("[USER-%d] Freed allocation #%d. Free blocks left: %d\n",
                   thread_id, my_allocation_count, free_bloks);

            my_allocation_count++;


        }else{
            /* Allocation failed */
            k_mutex_lock(&stats_mutex, K_FOREVER);
            total_failures++;
            k_mutex_unlock(&stats_mutex);

            my_failures++;
            printk("[USER-%d] Allocation #%d FAILED! (Timeout or no memory)\n",
                   thread_id, my_allocation_count);
        }
        /* Short pause between allocations */
        k_msleep(200);
    }

    printk("[USER-%d] Finished. Total allocations: %d, Failures: %d\n",
           thread_id, my_allocation_count, my_failures);
}

K_THREAD_STACK_ARRAY_DEFINE(user_stacks, NUM_USERS, 1024);
struct k_thread user_threads[NUM_USERS];

K_THREAD_STACK_DEFINE(monitor_stack, 1024);
struct k_thread monitor_thread;

/* Monitor Thread */
void monitor_thread_entry(void *p1, void *p2, void *P3){
    int last_allocations = 0;
    k_msleep(2000); /* Initial delay */
    while(1){
        k_msleep(3000);

        k_mutex_lock(&stats_mutex, K_FOREVER);
        int allocs = total_allocations;
        int fails = total_failures;
        int in_use = blocks_in_use;
        k_mutex_unlock(&stats_mutex);

        printk("[MONITOR] Total Allocations: %d (Delta: %d), Total Failures: %d, Blocks in use: %d\n",
               allocs, allocs - last_allocations, fails, in_use);

        int new_allocs = allocs - last_allocations;
        last_allocations = allocs;

        uint32_t used = k_mem_slab_num_used_get(&my_slab);
        uint32_t free = k_mem_slab_num_free_get(&my_slab);
        printk("\n      Memory Slab Statistics           \n");
        printk("\n");
        printk(" Total Allocations:  %3d              \n", allocs);
        printk(" Total Failures:     %3d              \n", fails);
        printk(" Blocks Used:        %3u/%3u          \n", used, NUM_BLOCKS);
        printk(" Blocks Free:        %3u/%3u          \n", free, NUM_BLOCKS);
        printk(" Rate (last 3s):     %3d allocs       \n", new_allocs);
        printk(" Success Rate:       %3d%%            \n", 
               (allocs + fails > 0) ? (allocs * 100 / (allocs + fails)) : 100);

        printk("---------------------------------------\n");

        if(allocs >= NUM_USERS * 5){
            printk("[MONITOR] All user threads completed their allocations. Exiting monitor.\n");
            break;
        }
        
    }

}

int main(void){
    printk("\nMemory Slab Demo \n\n");

    /* Start user threads */
    for(int i=0; i < NUM_USERS; i++){
        k_thread_create(&user_threads[i], user_stacks[i], K_THREAD_STACK_SIZEOF(user_stacks[i]),
                        memory_slab_user, (void *)(i+1), NULL, NULL,
                        7, 0, K_NO_WAIT);
    }

    /* Start monitor thread */
    k_thread_create(&monitor_thread, monitor_stack, K_THREAD_STACK_SIZEOF(monitor_stack),
                    monitor_thread_entry, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);

    printk("All threads started!\n\n");
    
    /* Main thread waits */
    while (1) {
        k_msleep(10000);
    }


    return 0;
}

