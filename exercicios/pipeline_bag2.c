#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"

int valI = 10,valJ = 5;
int valor[];
int valores[valI][valJ];

main(int argc, char** argv)
{
    int my_rank;  /* Identificador do processo */
    int proc_n;   /* Número de processos */
    int source;   /* Identificador do proc.origem */
    int dest;     /* Identificador do proc. destino */
    int tag = 50; /* Tag para as mensagens */

    char message[100]; /* Buffer para as mensagens */
    int i = 0,j=0;

    srand(time(NULL));

    for (i = 0; i < valI; i++) {
        for (j = 0; j < valJ; j++) {
            valores[i][j] = (i+j) * 100;
        }
    }

    MPI_Status status; /* Status de retorno */

    MPI_Init (&argc , & argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    double t1,t2;
    t1 = MPI_Wtime();  // inicia a contagem do tempo

    for (i = 0; i < valI; i++) {
        //primeiro, envia
        //meio, recebe, calcula, envia
        //final, pega imprime

        if (my_rank > 0)
        {
            printf("[%d] esperando\n",my_rank );
            MPI_Recv (valor, 1 * valJ, MPI_INT , my_rank - 1, tag, MPI_COMM_WORLD, &status);
            printf("recebido %d\n",i );
        }else{
            //printf("valores[%d]=%d\n",i,valores[i] );
            //printf("primeiro enviando i=%d\n", i);
            valor = valores[i];
            memcpy(&valor, &valores[i], sizeof a);
        }

        for (j = 0; j < valJ; j++) {
            valor[j] = valor[j] + 1;
            //printf("valor[%d]=%d\n",j,valor[j] );
        }

        if(my_rank < proc_n-1){
            MPI_Send (valor, 1 * valJ, MPI_INT,my_rank + 1, tag, MPI_COMM_WORLD);
            //printf("enviado [%d]\n",i );
        }else{
            printf("terminando %d:i=%d\n",my_rank,i);
            for (j = 0; j < valJ; j++) {
                valor[j] = valor[j] + 1;
                printf("valor[%d]=%d\n",j,valor[j] );
            }
        }

        //printf("%d:end\n",my_rank);
    }

    t2 = MPI_Wtime(); // termina a contagem do tempo
    printf("\n[%d]Tempo de execucao: %f\n\n",my_rank, t2-t1);

    MPI_Finalize();
}
