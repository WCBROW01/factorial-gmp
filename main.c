#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#include "factorial.h"

static const char *help =\
"Usage: factorial-gmp [OPTIONS]\n\
This program generates a factorial with parallel processing!\n\
\n\
Options:\n\
-i, --interactive\tStart in interactive mode. Default if no arguments are passed.\n\
-n, --number NUMBER\tInput number to calculate the factorial of.\n\
-p, --print\t\tPrint the generated factorial to the screen.";

static void invalid_args(const char *arg_type)
{
	fprintf(stderr, "Invalid argument for \"%s\". Check usage with \"--help\".\n", arg_type);
	exit(1);
}

static void interactive(long *number, bool *printing)
{
	int result; // this holds the number of inputs scanf found, so it is being used like a boolean
	printf("Enter number to complete factorial: ");
	result = scanf("%ld", number);
	if (!result) {
		fprintf(stderr, "Invalid number!\n");
		exit(1);
	}

	char *print_ans;
	printf("Would you like to print the result? ");
	result = scanf("%ms", &print_ans);
	if (!result) {
		fprintf(stderr, "No answer given.\n");
		exit(1);
	} else {
		*printing = *print_ans == 'y' || *print_ans == 'Y';
		free(print_ans);
	}
}

int main(int argc, char *argv[])
{
	long number = 0;
	bool printing = false;

	if (argc < 2 || strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0) {
		interactive(&number, &printing);
	} else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		puts(help);
		return 0;
	} else if (argc < 3) {
		fprintf(stderr, "Invalid number of arguments.\n");
		return 1;
	} else {
		for (int i = 1; i < argc; i++) {
			/* This is being used for a positional check.
			 * If it is still the same as the input, we know that strtol failed. */
			char *check_ptr;
			if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--number") == 0) {
				number = strtol(argv[i+1], &check_ptr, 10);
				if (check_ptr == argv[i+1]) invalid_args(argv[i]);
			} else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--print") == 0) {
				printing = true;
			}
		}
	}

	mpz_t result;
	gen_factorial(result, number);
	printf("Successfully generated %ld!\n", number);
	if (printing) gmp_printf("%Zd\n", result);

	mpz_clear(result);
	return 0;
}
