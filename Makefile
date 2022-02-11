CC=cc
CFLAGS=-Wall -Wextra
MPICC=mpicc

factorial-gmp: main.c factorial.c
	$(CC) $(CFLAGS) -o factorial-gmp main.c factorial.c -lgmp -O3

factorial-mpi: mpimain.c factorial.c
	$(MPICC) $(CFLAGS) -o factorial-mpi mpimain.c factorial.c -lgmp -O3
