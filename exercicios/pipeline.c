#include <stdio.h>
#include "mpi.h"

main(int argc, char** argv)
{
	int my_rank;  /* Identificador do processo */
	int proc_n;   /* Número de processos */
	int source;   /* Identificador do proc.origem */
	int dest;     /* Identificador do proc. destino */
	int tag = 50; /* Tag para as mensagens */

	char message[100]; /* Buffer para as mensagens */
	int valor;
	MPI_Status status; /* Status de retorno */

	MPI_Init (&argc , & argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

	valor = 0;
	//printf("%d:start\n",my_rank);
	//printf("%d:proc_n=%d\n",my_rank,proc_n);

	if (my_rank > 0)
	{
		MPI_Recv (&valor, 1, MPI_INT , my_rank - 1, tag, MPI_COMM_WORLD, &status);
	}

	valor = valor + 1;

	if(my_rank < proc_n-1){
		MPI_Send (&valor, 1, MPI_INT,my_rank + 1, tag, MPI_COMM_WORLD);
	}else{
		printf("%d:valor=%d\n",my_rank, valor);
	}

	//printf("%d:end\n",my_rank);

	MPI_Finalize();
}
