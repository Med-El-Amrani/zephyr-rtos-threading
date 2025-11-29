#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* RW-lock maison : 
 * - Un mutex pour protéger reader_count
 * - Un semaphore binaire pour exclure les writers
 */
K_MUTEX_DEFINE(reader_count_mutex);
K_SEM_DEFINE(rw_sem, 1, 1);  // 1 = libre, 0 = occupé

static int reader_count = 0;
static int shared_data = 0;

/* ===========================
 *       READER THREAD
 * =========================== */
void reader_thread(void *p1, void *p2, void *p3)
{
    int thread_id = (int)p1;

    while (1) {

        /* ENTRY SECTION */
        k_mutex_lock(&reader_count_mutex, K_FOREVER);
        reader_count++;
        bool first_reader = (reader_count == 1);
        k_mutex_unlock(&reader_count_mutex);

        if (first_reader) {
            /* Le premier lecteur bloque le writer */
            k_sem_take(&rw_sem, K_FOREVER);
        }

        /* CRITICAL SECTION (lecture partagée) */
        printk("[READER-%d] Reading data: %d, TIME=%lld\n",
               thread_id, shared_data, k_uptime_get());

        k_msleep(100);  // lecture simulée

        /* EXIT SECTION */
        k_mutex_lock(&reader_count_mutex, K_FOREVER);
        reader_count--;
        bool last_reader = (reader_count == 0);
        k_mutex_unlock(&reader_count_mutex);

        if (last_reader) {
            /* Le dernier lecteur libère le writer */
            k_sem_give(&rw_sem);
        }

        k_msleep(1000);
    }
}

void writer_thread(void *p1, void *p2, void *p3){

    int thread_id = (int)p1;

    while (1) {


        /* SECTION CRITIQUE (exclusion totale) */
        k_sem_take(&rw_sem, K_FOREVER);

        shared_data++;
        printk("[WRITER-%d] Writing data: %d\n", thread_id, shared_data);

        k_sem_give(&rw_sem);

        k_msleep(1000);
    }
	
}

/* Thread stacks */
K_THREAD_STACK_DEFINE(reader1_stack, 1024);
K_THREAD_STACK_DEFINE(reader2_stack, 1024);
K_THREAD_STACK_DEFINE(reader3_stack, 1024);
K_THREAD_STACK_DEFINE(writer1_stack, 1024);


struct k_thread reader1_data;
struct k_thread reader2_data;
struct k_thread reader3_data;
struct k_thread writer1_data;



int main(void){
	printk("\n=== Reader-Writer Lock Demo ===\n\n");

	/* Create  3 readers*/
	k_thread_create(&reader1_data, reader1_stack,
			K_THREAD_STACK_SIZEOF(reader1_stack),
			reader_thread, (void *)1,  NULL, NULL,
			5, 0, K_NO_WAIT);
	
	k_thread_create(&reader2_data, reader2_stack,
			K_THREAD_STACK_SIZEOF(reader2_stack),
			reader_thread, (void *)2,  NULL, NULL,
			5, 0, K_NO_WAIT);

	k_thread_create(&reader3_data, reader3_stack,
			K_THREAD_STACK_SIZEOF(reader3_stack),
			reader_thread, (void *)3,  NULL, NULL,
			5, 0, K_NO_WAIT);

	k_thread_create(&writer1_data, writer1_stack,
			K_THREAD_STACK_SIZEOF(writer1_stack),
			writer_thread, (void *)1,  NULL, NULL,
			5, 0, K_NO_WAIT);


	

	printk("Threads started\n\n");
	while(1){
		k_msleep(1000);
	}
	return 0;

}
