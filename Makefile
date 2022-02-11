CC=cc
CFLAGS=-Wall -Wextra -std=c99
LIBS=-pthread -lgmp
MPICC=mpicc

factorial-gmp: main.c factorial.c
	$(CC) $(CFLAGS) -o factorial-gmp main.c factorial.c $(LIBS) -O3

factorial-mpi: mpimain.c factorial.c
	$(MPICC) $(CFLAGS) -o factorial-mpi mpimain.c factorial.c $(LIBS) -O3
