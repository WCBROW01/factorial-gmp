#ifndef FACTORIAL_H
#define FACTORIAL_H

// This is a macro because having a one-line function that just passes in the same parameters is useless
#define gen_factorial(rop, number) gen_factorial_section(rop, 0, number)

void gen_factorial_section(mpz_t rop, long start, long end);

#endif
