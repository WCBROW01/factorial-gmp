/* This program was created by Will Brown for the Sphinx clusters at Bellarmine University.
 * If you are reading this in a few years, you are probably a student having
 * problems with the code. Feel free to contact me at wcbrow02@gmail.com,
 * or using my university email if I am still there.
 * Orginal source can be found at: https://github.com/WCBROW01/factorial-gmp
 * Licensed under the MIT License (c) 2022 Will Brown */

#ifndef FACTORIAL_H
#define FACTORIAL_H

// This is a macro because having a one-line function that just passes in the same parameters is useless
#define gen_factorial(rop, number) gen_factorial_section(rop, 0, number)

void gen_factorial_section(mpz_t rop, long start, long end);

#endif
