#include "mpi.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#define ARRAY_SIZE 100000
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

int *interleaving(int array[], int len)
{
	int *array_aux;
	int i1, i2,i3, i_aux;

	array_aux = (int *)malloc(sizeof(int) * len);

	i1 = 0;
	i2 = len / 3;
	i3 = (len / 3)*2;
    int lI1,lI2,lI3;
	for (i_aux = 0; i_aux < len; i_aux++)
	{
        lI1 = (i1 < (len/3));
        lI2 = (i2==((len / 3)*2));
        lI3 = (i3==len);
        if (((array[i1] <= array[i2] && array[i1] <= array[i3]) && lI1)
            || (lI2 && lI3))
			array_aux[i_aux] = array[i1++];
		else if (((array[i2] <= array[i1] && array[i2] <= array[i3]) && lI2)
                || (lI3 && lI1))
			array_aux[i_aux] = array[i2++];
        else
            array_aux[i_aux] = array[i3++];
	}

	return array_aux;
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
    int *vetor_aux;         /* ponteiro para o vetor resultantes que sera alocado dentro da rotina */
    int pai;
    int fator = 1;//fator para o calculo do delta, corresponde a quantidade de nodos do penultimo nivel da arvore
    if(proc_n==3)
        fator = 1;
    else if(proc_n==7)
        fator = 2;
    else if(proc_n==15)
        fator = 4;
    else if(proc_n==31)
        fator = 8;

    //time measure
    double t_before, t_after;

	int message[ARRAY_SIZE]; /* Buffer para as mensagens */
	MPI_Status status; /* Status de retorno */

	MPI_Init (&argc , & argv);

    t_before = MPI_Wtime();

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    int delta = ARRAY_SIZE/(fator*(proc_n+1));
	int vetor[ARRAY_SIZE];

    if ( my_rank != 0 )
    {
        #ifdef DEBUG
        printf("[%f]@slave[%d] waiting\n",curMilis(),my_rank);
        #endif
        MPI_Recv(&vetor[0],ARRAY_SIZE,MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &tam_vetor);
        pai = status.MPI_SOURCE;
        #ifdef DEBUG
        printf("[%f]@slave[%d] starting..., Array len=%d\n",curMilis(),my_rank,tam_vetor);
        #endif
    }
    else
    {
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
		printf("[%f]@master[%d] Initial array len=%d, delta=%d\n",curMilis(),my_rank,tam_vetor,delta);
		#endif
    }

    //root
    if(tam_vetor <= delta+1)
    {
        //printf("CONQUISTOU COM TAMANHO %d\n",tam_vetor);
        //conquista (bubble sort)
        bs(tam_vetor, vetor);    // conquisto
        if ( my_rank !=0 ){
            MPI_Send(&vetor,tam_vetor,MPI_INT, pai, 1, MPI_COMM_WORLD);
    		#ifdef DEBUG
            #ifdef PRINTV
            int i;
            printf("vetor@slave[%d]={",my_rank);
            for (i=0 ; i<tam_vetor; i++)              //* init array with worst case for sorting */
                printf("%d,",vetor[i] );
            printf("}\n\n");
            #endif
            printf("[%f]@slave[%d] leaving..., Array len=%d\n",curMilis(),my_rank,tam_vetor);
    		#endif
        }
    }
    else
    {
        //dividir e mandar para os filhos
        //2 * my_rank + 1 e 2 * my_rank + 2
        int tam0 = tam_vetor/2;
        int tam1 = tam_vetor/4;
        int tam2 = tam_vetor/4;
        int filhoE = 2 * my_rank + 1;
        int filhoD = 2 * my_rank + 2;
        if (tam_vetor%3 != 0)
            tam2 = tam2+1;

        #ifdef DEBUG
        #ifdef PRINTV
        int i;
        printf("vetor0={");
        for (i=0 ; i<tam0; i++)              /* init array with worst case for sorting */
            printf("%d,",vetor[i] );
        printf("}\n\n");
        printf("vetor1={");
        for (i=tam0 ; i<(tam1+tam0); i++)              /* init array with worst case for sorting */
            printf("%d,",vetor[i] );
        printf("}\n\n");
        printf("vetor2={");
        for (i=(tam1+tam0) ; i<(tam1+tam0+tam2); i++)              /* init array with worst case for sorting */
            printf("%d,",vetor[i] );
        printf("}\n\n");
        //printf("[%f]@master[%d] Initial array len=%d, delta=%d\n",curMilis(),my_rank,tam_vetor,delta);
        #endif
        #endif

        MPI_Send(&vetor[tam0], tam1, MPI_INT, filhoE, 1, MPI_COMM_WORLD);
        MPI_Send(&vetor[tam1+tam0], tam2, MPI_INT, filhoD, 1, MPI_COMM_WORLD);

        bs(tam0, vetor);

        //recebe dos filhos
        MPI_Recv(&vetor[tam0],tam1,MPI_INT, filhoE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&vetor[tam1+tam0],tam2,MPI_INT, filhoD, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        #ifdef DEBUG
        #ifdef PRINTV
        printf("vetor0={");
        for (i=0 ; i<tam0; i++)              /* init array with worst case for sorting */
            printf("%d,",vetor[i] );
        printf("}\n\n");
        printf("vetor1={");
        for (i=tam0 ; i<(tam1+tam0); i++)              /* init array with worst case for sorting */
            printf("%d,",vetor[i] );
        printf("}\n\n");
        printf("vetor2={");
        for (i=(tam1+tam0) ; i<(tam1+tam0+tam2); i++)              /* init array with worst case for sorting */
            printf("%d,",vetor[i] );
        printf("}\n\n");
        //printf("[%f]@master[%d] Initial array len=%d, delta=%d\n",curMilis(),my_rank,tam_vetor,delta);
        #endif
        #endif
        // intercalo vetor inteiro
        #ifdef UNTIL_INT
        if ( my_rank ==0 )
        {
            t_after = MPI_Wtime();
            printf("[%f]@master[%d]:Time measured=%1.3fs ;Array len=%d\n",curMilis(),my_rank, t_after - t_before,tam_vetor);
        }
        #endif
        vetor_aux = interleaving(vetor, tam_vetor);
        if ( my_rank !=0 )
        {
            MPI_Send(&vetor_aux[0],tam_vetor,MPI_INT, pai, 1, MPI_COMM_WORLD);
    		#ifdef DEBUG
    		printf("[%f]@slave[%d] leaving..., Array len=%d\n",curMilis(),my_rank,tam_vetor);
    		#endif
        }else
        {
    		#ifdef DEBUG
            int i;
            for (i=0 ; i<tam_vetor; i++){
                vetor[i] = vetor_aux[i];
            }
    		#endif
        }
    }
    // mando para o pai
    if ( my_rank ==0 )
    {
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
