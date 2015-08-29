#include "mpi.h"
#include <stdio.h>

int main(int argc, char **argv)
{
   char host[10];
   int myrank, //who am i
       numprocs; //how many process
   
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
   MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
   printf("Hostname: \"%s\" Rank: \"%d\" Size: \"%d\"\n",getenv("HOSTNAME"),myrank,numprocs);
   MPI_Finalize();
   return 0;
}
