#include <stdio.h>
#include "mpi.h"
#define DO_WORK 1
#define OK_WORK 2
#define DO_KILL 3
#define TRUE 1
#define FALSE 0
main(int argc, char** argv)
{
	int my_rank;  /* Identificador do processo */
	int proc_n;   /* Número de processos */
	int source;   /* Identificador do proc.origem */
	int dest;     /* Identificador do proc. destino */
	int tag = 50; /* Tag para as mensagens */
    int i;
	char message[100]; /* Buffer para as mensagens */
	int valor;
	MPI_Status status; /* Status de retorno */

	MPI_Init (&argc , & argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

	valor = 0;
	//printf("%d:start\n",my_rank);
	//printf("%d:proc_n=%d\n",my_rank,proc_n);
    //MPI_Recv (&valor, 1, MPI_INT,my_rank - 1, tag, MPI_COMM_WORLD, &status);
    //MPI_Send (&valor, 1, MPI_INT,my_rank + 1, tag, MPI_COMM_WORLD);
    int slavesAlive = proc_n - 1;

	if (my_rank > 0)
	{
        int working = TRUE;
        while(working){
    		MPI_Recv (&valor, 1, MPI_INT , 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(status.MPI_TAG==DO_WORK){
                valor = valor + 1;
                MPI_Send(&valor,1,MPI_INT,0,OK_WORK,MPI_COMM_WORLD);
            }
            if(status.MPI_TAG==DO_KILL){
                working = FALSE;
            }
        }
	}else{
        for (i = 1; i < proc_n; i++) {
            valor = i;
            MPI_Send(&valor, 1, MPI_INT,i, DO_WORK, MPI_COMM_WORLD);
        }
        while(slavesAlive > 0){
            MPI_Recv(&valor,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD, &status);
            if(status.MPI_TAG==OK_WORK){
                printf("Received %d from %d\n",valor,status.MPI_SOURCE);
                MPI_Send(&valor, 1, MPI_INT,status.MPI_SOURCE, DO_KILL, MPI_COMM_WORLD);
                slavesAlive = slavesAlive - 1;
            }
        }
    }

	printf("%d:end\n",my_rank);

	MPI_Finalize();
}
