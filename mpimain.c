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
#include <mpi.h>
#include <gmp.h>

#include "factorial.h"

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

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	int world_rank;
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
			mpz_init(result);
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

		// Generate the master node's section of the factorial
		mpz_t result;
		mpz_init(result);
		gen_factorial_section(result, 0, number / world_size);

		// Get the results from all of the other nodes
		for (int node = 1; node < world_size; node++) {
			// Probe for a message so we can get the necessary info to allocate a buffer and recieve it
			MPI_Status status;
			int count;
			MPI_Probe(MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_LONG, &count);
			long *section_buf = malloc(count * sizeof(long));
			MPI_Recv(section_buf, count, MPI_LONG, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			// Import the array back into an mpz object
			mpz_t section;
			mpz_init(section);
			mpz_import(section, count, -1, sizeof(long), 0, 0, section_buf);
			free(section_buf);

			// Multiply the recovered section into the final result
			mpz_mul(result, result, section);
			mpz_clear(section);
		}

		printf("Successfully generated %ld!\n", number);
		if (printing) gmp_printf("%Zd\n", result);
		mpz_clear(result);

		double end_time = MPI_Wtime();
		printf("Time to complete: %fs\n", end_time - start_time);
	} else {
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
	}

	MPI_Finalize();
	return 0;
}
