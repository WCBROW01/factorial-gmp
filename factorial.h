#ifndef FACTORIAL_H
#define FACTORIAL_H

// This is a macro because having a one-line function that just passes in the same parameters is useless
#define gen_factorial(rop, number, thread_count) gen_factorial_section(rop, 0, number, thread_count)

void gen_factorial_section(mpz_t rop, long start, long end, long thread_count);

#endif
