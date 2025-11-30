/* File : src/demo_barrier.c */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>

#define NUM_BARRIER_THREADS 4

K_SEM_DEFINE(barrier_sem, 0, NUM_BARRIER_THREADS);
K_MUTEX_DEFINE(barrier_mutex);
static int barrier_count = 0;

void barrier_wait(void){
	k_mutex_lock(&barrier_mutex, K_FOREVER);
	barrier_count++;

	printk("Thread %p at barrier (count: %d/%d)|n",
	k_current_get(), barrier_count, NUM_BARRIER_THREADS);

	if(barrier_count == NUM_BARRIER_THREADS){
		printk("All threads reached barrier! Releasing... ***\n");
		for(int i=0;i<NUM_BARRIER_THREADS;i++){
			k_sem_give(&barrier_sem);
		}
		barrier_count = 0;
	}
	k_mutex_unlock(&barrier_mutex);

	k_sem_take(&barrier_sem, K_FOREVER);
	
}

void barrier_thread(void *p1, void *p2, void *p3){
	int thread_id = (int)p1;
	int iteration = 0;

	while(1){
		printk("[THREAD-%d] Iteration %d - Phase 1 starting\n",
			thread_id, iteration);

		/* simulate different work duration*/
		k_msleep(sys_rand32_get() % 1000 + 500);
		
		printk("[THREAD-%d] Iteration %d - Phase 1 done, waiting at barrier\n",
			thread_id, iteration);
		
		barrier_wait();

		printk("[THREAD-%d] Iteration %d - Phase 2 starting \n",
			thread_id, iteration);

		k_msleep(sys_rand32_get() % 1000 + 500);

		printk("[THREAD-%d] Iteration %d - Phase 2 done\n",
			thread_id, iteration);

		iteration++;
		k_msleep(1000);
		
	}
}

K_THREAD_STACK_ARRAY_DEFINE(barrier_stacks, NUM_BARRIER_THREADS, 1024);
struct k_thread barrier_threads[NUM_BARRIER_THREADS];

int main(void){
	
	printk("\n===================================");
	printk(" Barrier synchonization demo \n");
	printk(" %d threads will synchronize at barriers\n", NUM_BARRIER_THREADS);
	printk("=====================================\n\n");

	for(int i=0;i< NUM_BARRIER_THREADS; i++){
		k_thread_create(&barrier_threads[i], barrier_stacks[i],
				K_THREAD_STACK_SIZEOF(barrier_stacks[i]),
				barrier_thread, (void *)i, NULL, NULL, 
				5, 0, K_NO_WAIT);
	}

	while(1){
		k_msleep(1000);
	}

	return 0;
}


