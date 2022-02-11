#ifndef FACTORIAL_H
#define FACTORIAL_H

void gen_factorial_section(mpz_t rop, long start, long end, long thread_count);
void gen_factorial(mpz_t rop, long number, long thread_count);

#endif
