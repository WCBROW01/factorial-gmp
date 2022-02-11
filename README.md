# factorial-gmp

An (almost) one-to-one port of [factorial-rs](https://github.com/WCBROW01/factorial-rs) to C using the [GNU Multiprecision Library](https://gmplib.org/). The program behaves identically, with almost no user-facing changes.

This seems pretty redundant since it is not faster than the Rust version (maybe slower, actually due to the inability to use Rust's channels) but it was written to use as a template for an [MPI](https://en.wikipedia.org/wiki/Message_Passing_Interface) implementation of the program, which is also included in this repository. If you're not looking to use MPI and you have access to a Rust compiler, you're better off using the Rust version.

### Building the program

To build factorial-gmp, all you need is a C compiler, make, and the GMP library (and headers).

The MPI build has no extra dependencies, other than an MPI implementation of your choosing. This program was built and tested with OpenMPI.

Either run `make factorial-gmp` or `make factorial-mpi` to build.

### Usage information

To get usage information, just run the program with the `--help` flag!

`-i` or `--interactive` will start the program in interactive mode (unavailable when using MPI)

`-n` or `--number` will immediately generate the desired factorial, with the next parameter being the number you want.

`-t` or `--threads` will run the program with the desired number of threads.

`-p` or `--print` will print the generated factorial to the console.