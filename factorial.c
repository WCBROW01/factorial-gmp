#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <gmp.h>

// Flags for thread status
#define S_COMPLETE 0b01
#define S_RETURNED 0b10

typedef struct FactorialThread {
	pthread_t thread;
	long start;
	long end;
	mpz_t section;
	char status;
} FactorialThread;

static void *run_thread(void *arg)
{
	FactorialThread *tdata = (FactorialThread*) arg;

	mpz_init_set_si(tdata->section, 1);
	for (long count = tdata->start; count <= tdata->end; count++) {
		mpz_mul_si(tdata->section, tdata->section, count);
	}

	tdata->status |= S_COMPLETE;
	return NULL;
}

void gen_factorial_section(mpz_t rop, long start, long end, long thread_count)
{
	long length = end - start;
	mpz_set_si(rop, 1);

	/* If the input is less than 1 (either 0 or a negative number),
	 * grab the number of available processors and use that. */
	if (thread_count < 1) thread_count = sysconf(_SC_NPROCESSORS_ONLN);

	/* If we have a greater quantity of threads than numbers to multiply,
	 * fall back to 1 thread. */
	if (length < thread_count) thread_count = 1;

	// Initialize each thread and give it a section of the factorial.
	FactorialThread threads[thread_count];
	for (long thread_num = 0; thread_num < thread_count; thread_num++) {
		threads[thread_num] = (FactorialThread){
			.start = thread_num * length / thread_count + start + 1,
			.end = (thread_num + 1) * length / thread_count + start,
			.status = 0
		};

		pthread_create(&threads[thread_num].thread, NULL, run_thread, &threads[thread_num]);
	}

	// Spin each thread until all of them have reported that they are complete.
	bool finished = false;
	while (!finished) {
		finished = true;
		for (long thread_num = 0; thread_num < thread_count; thread_num++) {
			if (!threads[thread_num].status & S_COMPLETE) finished = false;
			else if (!(threads[thread_num].status & S_RETURNED)) {
				mpz_mul(rop, rop, threads[thread_num].section);
				mpz_clear(threads[thread_num].section);
				threads[thread_num].status |= S_RETURNED;
			}
		}
	}
}
