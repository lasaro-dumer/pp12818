#include "mpi.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#define ARRAY_SIZE 1000000
//#define DEBUG 1
void bs(int n, int * vetor)
{
    int c=0, d, troca, trocou =1;

    while (c < (n-1) & trocou )
    {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                    troca      = vetor[d];
                    vetor[d]   = vetor[d+1];
                    vetor[d+1] = troca;
                    trocou = 1;
                }
        c++;
    }
}

const double curMilis()
{
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	return ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000.0) +0.5; // convert tv_sec & tv_usec to millisecond
}

main(int argc, char** argv)
{
	int my_rank;  /* Identificador do processo */
	int proc_n;   /* Número de processos */
	int tam_vetor = 0;
	//time measure
	double t_before, t_after;

	MPI_Init (&argc , & argv);
	t_before = MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
	if(my_rank==0){
        int vetor[ARRAY_SIZE];
        tam_vetor = ARRAY_SIZE;
        // sou a raiz e portanto gero o vetor - ordem reversa
        int i;
        for (i=0 ; i<ARRAY_SIZE; i++)              /* init array with worst case for sorting */
            vetor[i] = ARRAY_SIZE-i;
        #ifdef DEBUG
        #ifdef PRINTV
        printf("vetor={");
        for (i=0 ; i<ARRAY_SIZE; i++)              /* init array with worst case for sorting */
            printf("%d,",vetor[i] );
        printf("}\n");
        #endif
    	printf("[%f]@master[%d] Initial array len=%d\n",curMilis(),my_rank,tam_vetor);
    	#endif

        //conquista (bubble sort)
        bs(tam_vetor, vetor);    // conquisto
        t_after = MPI_Wtime();
        //#ifdef DEBUG
        printf("[%f]@master[%d]:Time measured=%1.3fs ;Array len=%d\n",curMilis(),my_rank, t_after - t_before,tam_vetor);
        //#endif
        #ifdef DEBUG
        #ifdef PRINTV
        int i;
        printf("vetor={");
        for (i=0 ; i<ARRAY_SIZE; i++)              /* init array with worst case for sorting */
            printf("%d,",vetor[i] );
        printf("}\n");
        #endif
    	printf("[%f]@master leaving...\n",curMilis());
    	#endif
    }
    MPI_Finalize();
}

