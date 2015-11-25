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
#define NUM_ARRAYS 1000
#define ARRAYS_SIZE 10000
//#define DEBUG 0

int compare (const void * a, const void * b){return ( *(int*)a - *(int*)b );}
const char * printTag(int tag){
    if(tag== GET_WORK){return "GET_WORK";}
	else if(tag==WORK_DONE){return "WORK_DONE";}
	else if(tag==WORK){return "WORK";}
	else if(tag==SUICIDE){return "SUICIDE";}
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
	int workDist[proc_n][1];

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
                MPI_Send(saco[next], ARRAYS_SIZE, MPI_INT,s, WORK, MPI_COMM_WORLD);//envia o vetor para o escravo 's'
				workDist[s][0]=next;
                next++;
            }
        }
		
		#ifdef DEBUG
		for(i = 0; i < proc_n; i++)
        {
			printf("workDist[%d]=%d\n",i,workDist[i][0]);
        }
		#endif
        while(slavesAlive > 0){//enquanto existirem escravos vivos, fica esperando mensagens
            MPI_Recv(ordered, ARRAYS_SIZE, MPI_INT,MPI_ANY_SOURCE, WORK_DONE, MPI_COMM_WORLD, &status);  //espera o vetor do mesmo escravo q enviou o indice
            memcpy(saco[workDist[status.MPI_SOURCE][0]],ordered,ARRAYS_SIZE*sizeof(int));//coloca o vetor ordenado na matriz
			if(next>=NUM_ARRAYS){//se o numero de tarefas ja se esgotou, termina o escravo
				MPI_Send(&next, 1, MPI_INT,status.MPI_SOURCE, SUICIDE, MPI_COMM_WORLD);
				slavesAlive--;
            }else {//se nao envia a proxima tarefa
                MPI_Send(saco[next], ARRAYS_SIZE, MPI_INT,status.MPI_SOURCE, WORK, MPI_COMM_WORLD);//envia o vetor para o escravo
				workDist[status.MPI_SOURCE][0]=next;
				#ifdef DEBUG
				printf("workDist[%d]=%d\n",status.MPI_SOURCE,workDist[status.MPI_SOURCE][0]);
				#endif
                next++;
            }
        }
        printf("[%f]@master leaving...\n",curMilis());
        t2 = MPI_Wtime();        // contagem de tempo termina neste ponto
        printf("[%f]@[%d]MPI_Wtime measured work time to be: %1.2f\n",curMilis(),my_rank, t2-t1);
    }
    else
    {
        int tag = WORK;
        do{
            MPI_Recv(toOrder, ARRAYS_SIZE, MPI_INT,0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//...fica esperando o vetor em seguida
    		tag = status.MPI_TAG;
            if(tag == WORK){//recebeu um vetor para ordenar
                int th_id, nthreads;
			   	// omp_set_num_threads(4); // disparar 4 threads pois se trata de uma m�quina Quad-Core
			   	#pragma omp parallel private(th_id, nthreads) num_threads(4)
			   	{
			 		th_id = omp_get_thread_num();
			 		nthreads = omp_get_num_threads();
			 		int ini = (th_id*(nthreads));
			 		int end = (th_id*(nthreads))+(ARRAYS_SIZE/nthreads);
			 		#ifdef DEBUG
			 		printf("%d:[%d/%d]ini=%d, end=%d\n",my_rank, th_id, nthreads,ini,end);
			 		for(j = ini; j< end; j++)
                    {
				 		printf("%d:[%d/%d]v[%d]=%d\n",my_rank, th_id, nthreads,j,toOrder[j]);
                    }
			 		#endif
		 			qsort (&toOrder[ini], ini-end, sizeof(int), compare);//ordena o vetor                
			   	}
            	MPI_Send(toOrder,ARRAYS_SIZE, MPI_INT,0, WORK_DONE, MPI_COMM_WORLD);//envia o vetor para o mestre
            }
        }while(tag != SUICIDE);//se a ultima tag foi a de suicidio, termina execução
    }
    MPI_Finalize();
}
