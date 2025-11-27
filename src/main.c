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

int main(void){
	while(1){
		k_msleep(1000);
	}
	return 0;
}

