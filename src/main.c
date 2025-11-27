#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/random/random.h>

/* Thread Stack Sizes */
#define PRODUCER_STACK_SIZE 1024
#define CONSUMER_STACK_SIZE 1024
#define PRIORITY_STACK_SIZE 1024
#define WORKER_STACK_SIZE 1024
#define MONITOR_STACK_SIZE 1024

/* Thread Priorities (lower number = higher prriority in zephyr) */
#define PRODUCER_PRIORITY 5
#define CONSUMER_PRIORITY 5
#define HIGH_PRIORITY_THREAD 3
#define LOW_PRIORITY_THREAD 7
#define WORKER_PRIORITY_THREAD 6
#define MONITOR_PRIORITY_THREAD 4

/* Message queue parameters */
#define MSGQ_MAX_MSGS 10
#define MSG_SIZE sizeof(struct data_item)

/* Data sutructures */
struct data_item {
	uint32_t id;
	uint32_t value;
	char message[32];
};

/* Thraeds stacks */
K_THREAD_STACK_DEFINE(producer_stack, PRODUCER_STACK_SIZE);
K_THREAD_STACK_DEFINE(consumer_stack, CONSUMER_STACK_SIZE);
K_THREAD_STACK_DEFINE(high_prio_stack, PRIORITY_STACK_SIZE);
K_THREAD_STACK_DEFINE(low_prio_stack, PRIORITY_STACK_SIZE);
K_THREAD_STACK_DEFINE(worker_stack, WORKER_STACK_SIZE);
K_THREAD_STACK_DEFINE(monitor_stack, MONITOR_STACK_SIZE);

/* Thread control blocks */
struct k_thread producer_thread;
struct k_thread consumer_thread;
struct k_thread high_prio_thread;
struct k_thread low_prio_thread;
struct k_thread worker_thread;
struct k_thread monitor_thread;

/* Synchronization primitives */
K_MUTEX_DEFINE(shared_resource_mutex);
K_SEM_DEFINE(producer_sem, 0, 1);
K_SEM_DEFINE(consumer_sem, 0, 1);
K_MSGQ_DEFINE(data_msgq, MSG_SIZE, MSGQ_MAX_MSGS, 4);

/* Shared resources */
static uint32_t shared_counter = 0;
static uint32_t produced_items = 0;
static uint32_t consumed_items = 0;

/* Work Queue items */
K_WORK_DEFINE(my_work, NULL); // Will set handler later

/* Timer */
struct k_timer periodic_timer;

/* Forward declarations */
void producer_entry(void* p1, void* p2, void* p3);
void consumer_entry(void* p1, void* p2, void* p3);
void high_priority_entry(void* p1, void* p2, void* p3);
void low_priority_entry(void* p1, void* p2, void* p3);
void worker_entry(void* p1, void* p2, void* p3);
void monitor_entry(void* p1, void* p2, void* p3);
void timer_handler(struct k_timer *timer);
void work_handler(struct k_work* work);

/*
* CONCEPT 1 : PRODUCER-CONSUMER PATTERN withh Message Queue
* Demonstrates : Message queues, thread communication 
*/
void producer_entry(void *p1, void *p2, void *p3){
	struct data_item item;
	printk("Producer thread started (ID : %p)\n", k_current_get());
}

void consumer_entry(void* p1, void *p2, void *p3){
	struct data_item item;
	printk("Consumer thread started (ID : %p)\n",k_current_get());
}


int main(void){
	
	/* create producer thread */
	k_thread_create(&producer_thread, producer_stack,
			K_THREAD_STACK_SIZEOF(producer_stack),
			producer_entry,
			NULL, NULL, NULL,
			PRODUCER_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&producer_thread, "producer");

	/* Create Consumer thread */
	k_thread_create(&consumer_thread, consumer_stack,
			K_THREAD_STACK_SIZEOF(consumer_stack),
			consumer_entry,
			NULL, NULL, NULL,
			CONSUMER_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&consumer_thread, "consumer");
	
	while(1){
		k_msleep(1000);
	}
	return 0;
}

