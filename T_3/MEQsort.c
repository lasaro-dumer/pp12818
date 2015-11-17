#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "mpi.h"
#define GET_WORK 0
#define WORK_DONE 1
#define WORK 2
#define SUICIDE 3
#define WORK_INDEX 4
#define NUM_ARRAYS 4000
#define ARRAYS_SIZE 100000
#define DEBUG 0

int compare (const void * a, const void * b){return ( *(int*)a - *(int*)b );}
const char * printTag(int tag){
    if(tag== GET_WORK){return "GET_WORK";}
	else if(tag==WORK_DONE){return "WORK_DONE";}
	else if(tag==WORK){return "WORK";}
	else if(tag==SUICIDE){return "SUICIDE";}
	else if(tag==WORK_INDEX){return "WORK_INDEX";}
	else{return "Invalid Tag";}
}

const double curMilis(){
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000.0) +0.5; // convert tv_sec & tv_usec to millisecond
}

main(int argc, char** argv){
    int my_rank;
    int proc_n;
    int **saco;
    int toOrder[ARRAYS_SIZE];
    int ordered[ARRAYS_SIZE];
    int i,j,s;
    double t1, t2;

    srand(time(NULL));
    int r = rand();

    MPI_Status status; /* Status de retorno */
    MPI_Init(&argc , & argv); // funcao que inicializa o MPI, todo o cÃ³digo paralelo esta abaixo
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    int slavesAlive = proc_n-1;

    if ( my_rank == 0 ){
        t1 = MPI_Wtime();        // contagem de tempo inicia neste ponto
        saco = (int **)malloc(NUM_ARRAYS * sizeof(int *));//alocação do vetor para os vetores
        if(saco == NULL)
        {
            printf("out of memory\n");
            return -1;
        }else{
            int valor = ARRAYS_SIZE * NUM_ARRAYS;//inicialização do valor
            for(i = 0; i < NUM_ARRAYS; i++)
            {
                saco[i] =(int *) malloc(ARRAYS_SIZE * sizeof(int));//alocação de cada vetor e seu lugar
                if(saco[i] == NULL)
                {
                    printf("out of memory row %d\n",i);
                    return -1;
                }else{
                    for(j = 0; j< ARRAYS_SIZE; j++)
                    {
                        saco[i][j] = valor;//atribuição do valor ao vetor
                        valor--;
                    }
                }
            }
        }

        int next = 0;
		//mestre faz primeiro envio de tarefas para os escravos
        for(s=1;s<=slavesAlive;s++){
            if(next>=NUM_ARRAYS){//se o numero de tarefas ja se esgotou, termina o escravo
                MPI_Send(&next, 1, MPI_INT,s, SUICIDE, MPI_COMM_WORLD);
                slavesAlive--;
            }else {
                MPI_Send(&next,1,MPI_INT,s,WORK_INDEX,MPI_COMM_WORLD);//envia o indice do vetor para o escravo 's'
                MPI_Send(saco[next], ARRAYS_SIZE, MPI_INT,s, WORK, MPI_COMM_WORLD);//envia o vetor para o escravo 's'
                next++;
            }
        }
        int orderedI = 0;
        while(slavesAlive > 0){//enquanto existirem escravos vivos, fica esperando mensagens
    		MPI_Recv(&orderedI,1, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//recebe uma mensagem do escravo e abaixo verifica se é um indice de vetor
    		if(status.MPI_TAG == WORK_INDEX){
                MPI_Recv(ordered, ARRAYS_SIZE, MPI_INT,status.MPI_SOURCE, WORK_DONE, MPI_COMM_WORLD, &status);  //espera o vetor do mesmo escravo q enviou o indice
                memcpy(saco[orderedI],ordered,ARRAYS_SIZE*sizeof(int));//coloca o vetor ordenado na matriz
    			if(next>=NUM_ARRAYS){//se o numero de tarefas ja se esgotou, termina o escravo
    				MPI_Send(&next, 1, MPI_INT,status.MPI_SOURCE, SUICIDE, MPI_COMM_WORLD);
    				slavesAlive--;
                }else {//se nao envia a proxima tarefa
                    MPI_Send(&next,1,MPI_INT,status.MPI_SOURCE,WORK_INDEX,MPI_COMM_WORLD);//envia o indice do vetor para o escravo
                    MPI_Send(saco[next], ARRAYS_SIZE, MPI_INT,status.MPI_SOURCE, WORK, MPI_COMM_WORLD);//envia o vetor para o escravo
                    next++;
                }
    		}
        }
        printf("[%f]@master leaving...\n",curMilis());
        t2 = MPI_Wtime();        // contagem de tempo termina neste ponto
        printf("[%f]@[%d]MPI_Wtime measured work time to be: %1.2f\n",curMilis(),my_rank, t2-t1);
    }
    else
    {
        int tag = WORK;
        int toOrderI=0;
        do{
            MPI_Recv(&toOrderI, 1, MPI_INT, 0, MPI_ANY_TAG,MPI_COMM_WORLD,&status);//recebe o comando do mestr
            tag = status.MPI_TAG;
            if(tag == WORK_INDEX){//se o comando recebido foi um indice...
                MPI_Recv(toOrder, ARRAYS_SIZE, MPI_INT,0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//...fica esperando o vetor em seguida
        		tag = status.MPI_TAG;
            }
    		if(tag == WORK){//recebeu um vetor para ordenar
                qsort (toOrder, ARRAYS_SIZE, sizeof(int), compare);//ordena o vetor
                MPI_Send(&toOrderI,1, MPI_INT,0, WORK_INDEX, MPI_COMM_WORLD);//envia o indice do vetor para o mestre
                MPI_Send(toOrder,ARRAYS_SIZE, MPI_INT,0, WORK_DONE, MPI_COMM_WORLD);//envia o vetor para o mestre
            }
        }while(tag != SUICIDE);//se a ultima tag foi a de suicidio, termina execução
    }
    MPI_Finalize();
}
