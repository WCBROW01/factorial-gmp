/* This program was created by Will Brown for the Sphinx clusters at Bellarmine University.
 * If you are reading this in a few years, you are probably a student having
 * problems with the code. Feel free to contact me at wcbrow02@gmail.com,
 * or using my university email if I am still there.
 * Orginal source can be found at: https://github.com/WCBROW01/factorial-gmp
 * Licensed under the MIT License (c) 2022 Will Brown */

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <gmp.h>

// Flags for thread status
#define S_COMPLETE 0b01
#define S_RETURNED 0b10

typedef struct FactorialThread {
	pthread_t thread;
	long start;
	long end;
	mpz_t section;
	long *thread_list;
	long level;
	char status;
} FactorialThread;

static void *run_thread(void *arg)
{
	FactorialThread *tdata = (FactorialThread*) arg;

	mpz_init_set_si(tdata->section, 1);
	if (tdata->level < tdata->thread_list[0]) {
		long length = tdata->end - tdata->start;
		long thread_count = tdata->thread_list[tdata->level];

		// Initialize each thread and give it a section of the factorial.
		FactorialThread threads[thread_count];
		for (long thread_num = 0; thread_num < thread_count; thread_num++) {
			threads[thread_num] = (FactorialThread){
				.start = thread_num * length / thread_count + tdata->start + 1,
				.end = (thread_num + 1) * length / thread_count + tdata->start,
				.thread_list = tdata->thread_list,
				.level = tdata->level + 1,
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
					mpz_mul(tdata->section, tdata->section, threads[thread_num].section);
					mpz_clear(threads[thread_num].section);
					threads[thread_num].status |= S_RETURNED;
				}
			}
		}
	} else {
		for (long count = tdata->start; count <= tdata->end; count++) {
			mpz_mul_si(tdata->section, tdata->section, count);
		}
	}

	tdata->status |= S_COMPLETE;
	return NULL;
}

static inline long lsqrt(long n)
{
	n = labs(n);
	long x = n / 2;
	long xLast = x;
	long xLast2;

	do {
		xLast2 = xLast;
		xLast = x;
		x = (x + n / x) / 2;
	} while (x != xLast && x != xLast2);

	return x;
}

static void _gen_thread_list(long n, long **list, long count)
{
	// This array will be used to generate a tree of threads
	if (n >= 16) {
		_gen_thread_list(lsqrt(lsqrt(n)), list, count + 1);
	} else {
		*list = calloc(count + 1, sizeof(long));
		(*list)[0] = 0;
	}

	long len = ++(*list)[0]; // First index stores the length, so it is incremented
	(*list)[len] = n; // len is also the index we will use.
}

static long *gen_thread_list(long section_length)
{
	long *list;
	_gen_thread_list(section_length, &list, 1);
	return list;
}

void gen_factorial_section(mpz_t rop, long start, long end)
{
	if ((end - start) > 16) {
		long *thread_list = gen_thread_list(end - start);
		FactorialThread main_thread = {
			.start = start,
			.end = end,
			.thread_list = thread_list,
			.level = 1,
			.status = 0
		};

		/* This is not how this function would normally be used but we would
		 * just be joining the thread immediately anyways. */
		run_thread(&main_thread);
		free(thread_list);

		mpz_init(rop);
		mpz_set(rop, main_thread.section);
		mpz_clear(main_thread.section);
	} else {
		mpz_init_set_si(rop, 1);
		start = start < 1 ? 1 : start;
		for (long count = start; count <= end; count++) {
			mpz_mul_si(rop, rop, count);
		}
	}
}
