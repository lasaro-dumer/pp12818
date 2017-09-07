#include <time.h>
#include <stdio.h>
#include "mpi.h"

#define DO_WORK 1
#define OK_WORK 2
#define DO_KILL 3
#define GET_WORK 4

#define TRUE 1
#define FALSE 0
main(int argc, char** argv)
{
	int my_rank;  /* Identificador do processo */
	int proc_n;   /* Número de processos */
	int i;
	int value;
	int lenV = 200;
    int valores[lenV];
	MPI_Status status; /* Status de retorno */
    srand(time(NULL));

	MPI_Init (&argc , & argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

	value = 0;
	//printf("%d:start\n",my_rank);
	//printf("%d:proc_n=%d\n",my_rank,proc_n);
    //MPI_Recv (&value, 1, MPI_INT,my_rank - 1, tag, MPI_COMM_WORLD, &status);
    //MPI_Send (&value, 1, MPI_INT,my_rank + 1, tag, MPI_COMM_WORLD);
    int slavesAlive = proc_n - 1;

	if (my_rank > 0)
	{
        int working = TRUE;
        while(working){
    		MPI_Recv (&value, 1, MPI_INT , 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(status.MPI_TAG==DO_WORK){
                value = value + 1;
                MPI_Send(&value,1,MPI_INT,0,OK_WORK,MPI_COMM_WORLD);
            }
            if(status.MPI_TAG==DO_KILL){
                working = FALSE;
            }
        }
	}else{
		for (i = 0; i < lenV; i++) {
			values[i]=rand()%10;
		}
		int next = 0;
        while(slavesAlive > 0){
            MPI_Recv(&value,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD, &status);
            if(status.MPI_TAG==GET_WORK){
                //printf("Received %d from %d\n",value,status.MPI_SOURCE);
				value = values[next]
                MPI_Send(&value, 1, MPI_INT,status.MPI_SOURCE, DO_KILL, MPI_COMM_WORLD);
                slavesAlive = slavesAlive - 1;
            }
        }
    }

	printf("%d:end\n",my_rank);

	MPI_Finalize();
}
