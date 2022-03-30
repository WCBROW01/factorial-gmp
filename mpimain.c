/* This program was created by Will Brown for the Sphinx clusters at Bellarmine University.
 * If you are reading this in a few years, you are probably a student having
 * problems with the code. Feel free to contact me at wcbrow02@gmail.com,
 * or using my university email if I am still there.
 * Orginal source can be found at: https://github.com/WCBROW01/factorial-gmp
 * Licensed under the MIT License (c) 2022 Will Brown */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <mpi.h>
#include <gmp.h>

#include "factorial.h"

#define S_RECEIVED 0b01
#define S_COMPLETE 0b10

// Result needs to be passed in as a pointer because mpz_t is an array type
struct RecvData {
	MPI_Request request;
	mpz_t section;
	mpz_t *result;
	long *section_buf;
	int count;
	int source;
	char status;
};

static const char *help =\
"Usage: mpirun [OPTIONS] factorial-mpi [OPTIONS]\n\
This program generates a factorial with parallel processing using both MPI and OS threads.\n\
\n\
Options:\n\
-n, --number NUMBER\tInput number to calculate the factorial of.\n\
-p, --print\t\tPrint the generated factorial to the screen.";

static void invalid_args(const char *arg_type, int world_rank)
{
	if (world_rank == 0) fprintf(stderr, "Invalid argument for \"%s\". Check usage with \"--help\".\n", arg_type);
	MPI_Finalize();
	exit(1);
}

void *gen_section_mpi(void *arg)
{
	long number = *(long*) arg;

	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	// Calculate what section of the factorial we want to generate.
	long start = world_rank * number / world_size;
	long end = (world_rank + 1) * number / world_size;

	// Start generating factorial
	mpz_t section;
	mpz_init(section);
	gen_factorial_section(section, start, end);

	// Export the result to an array that we can send over MPI
	size_t buf_size;
	long *section_buf = mpz_export(NULL, &buf_size, -1, sizeof(long), 0, 0, section);
	mpz_clear(section);
	MPI_Send(section_buf, buf_size, MPI_LONG, 0, 1, MPI_COMM_WORLD);
	free(section_buf);

	return NULL;
}

void *import_section(void *arg)
{
	struct RecvData *data = (struct RecvData*) arg;
	MPI_Wait(&data->request, MPI_STATUS_IGNORE);

	// Import the array back into an mpz object
	mpz_init(data->section);
	mpz_import(data->section, data->count, -1, sizeof(long), 0, 0, data->section_buf);
	free(data->section_buf);
	data->status |= S_RECEIVED;

	return NULL;
}

void *reduce_result(void *arg)
{
	struct RecvData *ndata = (struct RecvData*) arg;
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	bool finished = false;
	while (!finished) {
		finished = true;
		for (int node = 0; node < world_size; node++) {
			if (!(ndata[node].status & S_RECEIVED)) finished = false;
			else if (!(ndata[node].status & S_COMPLETE)) {
				// Multiply the recovered section into the final result
				mpz_mul(*ndata[node].result, *ndata[node].result, ndata[node].section);
				mpz_clear(ndata[node].section);
				ndata[node].status |= S_COMPLETE;
			}
		}
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	int world_size, world_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	long number = 0;
	bool printing = false;

	if (argc < 3 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		if (world_rank == 0) puts(help);
		MPI_Finalize();
		return 0;
	} else {
		for (int i = 1; i < argc; i++) {
			/* This is being used for a positional check.
			 * If it is still the same as the input, we know that strtol failed. */
			char *check_ptr;
			if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--number") == 0) {
				number = strtol(argv[i+1], &check_ptr, 10);
				if (check_ptr == argv[i+1]) invalid_args(argv[i], world_rank);
			} else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--print") == 0) {
				printing = true;
			}
		}
	}

	// If the number is too small, act as if we have one node.
	if (number < world_size * 4) {
		if (world_rank == 0) {
			double start_time = MPI_Wtime();
			mpz_t result;
			gen_factorial(result, number);
			printf("Successfully generated %ld!\n", number);
			if (printing) gmp_printf("%Zd\n", result);
			mpz_clear(result);
			double end_time = MPI_Wtime();
			printf("Time to complete: %fs\n", end_time - start_time);
		}

		MPI_Finalize();
		return 0;
	}

	if (world_rank == 0) {
		double start_time = MPI_Wtime();

		pthread_t section_thread, reduce_thread;
		pthread_t import_thread[world_size];

		// Generate the master node's section of the factorial
		pthread_create(&section_thread, NULL, gen_section_mpi, &number);

		// Set up shared storage
		struct RecvData ndata[world_size];
		memset(ndata, 0, sizeof(struct RecvData) * world_size);

		mpz_t result;
		mpz_init_set_ui(result, 1);

		pthread_create(&reduce_thread, NULL, reduce_result, ndata);

		// Get the results from all of the nodes
		for (int node = 0; node < world_size; node++) {
			// Probe for a message so we can get the necessary info to allocate a buffer and recieve it
			MPI_Status status;
			MPI_Probe(MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

			// Set up shared data for receiving
			struct RecvData *data = &ndata[status.MPI_SOURCE];
			data->result = &result;
			data->source = status.MPI_SOURCE;
			MPI_Get_count(&status, MPI_LONG, &data->count);
			data->section_buf = malloc(data->count * sizeof(long));
			sched_yield();
			MPI_Irecv(data->section_buf, data->count, MPI_LONG, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &data->request);

			// Initialize receiving thread
			pthread_create(&import_thread[data->source], NULL, import_section, data);
		}

		pthread_join(reduce_thread, NULL);

		printf("Successfully generated %ld!\n", number);
		if (printing) gmp_printf("%Zd\n", result);
		mpz_clear(result);

		double end_time = MPI_Wtime();
		printf("Time to complete: %fs\n", end_time - start_time);
	} else {
		gen_section_mpi(&number);
	}

	MPI_Finalize();
	return 0;
}
