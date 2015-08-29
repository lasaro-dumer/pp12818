CCFLAGS=-Werror -Wall
LIBS=-L/usr/X11R6/lib -lX11
EXECENV=/usr/local
TARGET=.
all: mpigm mpiether

clean:
	rm -f *.o  mandel.mpi.*

mpigm:
	$(EXECENV)/mpich-gm/bin/mpicc $(LIBS) mandel.c   -o $(TARGET)/mandel.mpi.gm
mpiether:
	$(EXECENV)/mpich-ether/bin/mpicc $(LIBS) mandel.c   -o $(TARGET)/mandel.mpi.ether
