#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mpi.h"
#define GET_WORK 0
#define WORK_DONE 1
#define WORK 2
#define SUICIDE 3
#define NUM_ARRAYS 1000
#define ARRAYS_SIZE 10000

int compare (const void * a, const void * b){
  return ( *(int*)a - *(int*)b );
}

const char * printTag(int tag){
    if(tag== GET_WORK){
        return "GET_WORK";
    }else if(tag==WORK_DONE){
        return "WORK_DONE";
    }else if(tag==WORK){
        return "WORK";
    }else if(tag==SUICIDE){
        return "SUICIDE";
    }else{
        return "Invalid Tag";
    }
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
    int i,j,s;
    int * val=(int*)0;
    double t1, t2;

	srand(time(NULL));
	int r = rand();

    MPI_Status status; /* Status de retorno */

    MPI_Init(&argc , & argv); // funcao que inicializa o MPI, todo o cÃ³digo paralelo esta abaixo


    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
	int dones = 0;
    int slavesAlive = proc_n-1;

    if ( my_rank == 0 ){
        t1 = MPI_Wtime();        // contagem de tempo inicia neste ponto
        //printf("[%f]@master starting...\n",curMilis());
        //printf("[%f]@creating works to do...\n",curMilis());
        saco = (int **)malloc(NUM_ARRAYS * sizeof(int *));
        if(saco == NULL)
        {
            printf("out of memory\n");
            return -1;
        }else{
            for(i = 0; i < NUM_ARRAYS; i++)
            {
                saco[i] =(int *) malloc(ARRAYS_SIZE * sizeof(int));
                if(saco[i] == NULL)
                {
                    printf("out of memory row %d\n",i);
                    return -1;
                }else{
                    for(j = 0; j< ARRAYS_SIZE; j++)
                    {
                        saco[i][j] = ARRAYS_SIZE - j;
                    }
                }
            }
        }
        //printf("[%f]@works created...\n",curMilis());

        int next = 0;
        //printf("[%f]@sending works\n",curMilis());
        for(s=1;s<=slavesAlive;s++){
            if(next>=NUM_ARRAYS){
                MPI_Send(val, 8, MPI_INT,s, SUICIDE, MPI_COMM_WORLD);
                slavesAlive--;
            }else {
                //printf("[%f]@sending work to slave %d\n",curMilis(),s);
                MPI_Send(saco[next], ARRAYS_SIZE, MPI_INT,s, WORK, MPI_COMM_WORLD);
                next++;
            }
        }
        //printf("[%f]@works sended; next=%d\n",curMilis(), next);

	/*
        if(next==0){
            printf("[%f]@master ordering...\n",curMilis());
            while(next<NUM_ARRAYS){
                qsort (saco[next], ARRAYS_SIZE, sizeof(int), compare);
                next++;
            }
            printf("[%f]@master done ordering. total=%d/%d...\n",curMilis(),next,NUM_ARRAYS);
        }
	//*/
	while(slavesAlive > 0){
		MPI_Recv(toOrder, ARRAYS_SIZE, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  // recebo por ordem de chegada com any_source
		if(status.MPI_TAG == WORK_DONE){
			saco[dones]=toOrder;
			dones++;
		}else if(status.MPI_TAG == GET_WORK){
	        	if(next>=NUM_ARRAYS){
				MPI_Send(toOrder, ARRAYS_SIZE, MPI_INT,status.MPI_SOURCE, SUICIDE, MPI_COMM_WORLD);
				slavesAlive--;
    			}else {
				MPI_Send(saco[next], ARRAYS_SIZE, MPI_INT,status.MPI_SOURCE, WORK, MPI_COMM_WORLD);
	            		next++;
    			}
		}
	}
        printf("[%f]@master leaving...\n",curMilis());

        t2 = MPI_Wtime();        // contagem de tempo termina neste ponto

        printf("[%f]@[%d]MPI_Wtime measured work time to be: %1.2f\n",curMilis(),my_rank, t2-t1);
        /*
        printf("ordenados\n");
        for(i = 0; i < NUM_ARRAYS; i++)
        {
            printf("array[%d]={",i);
            for(j = 0; j< ARRAYS_SIZE; j++)
            {
                printf("%d,",saco[i][j]);
            }
            printf("}\n");
        }//*/
    }
    else
    {
        int tag = WORK;
        do{
            //printf("[%f]@slave[%d] waiting...\n",curMilis(),my_rank);
            MPI_Recv(toOrder, ARRAYS_SIZE, MPI_INT,0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            //printf("[%f]@receiving work from master with tag %s\n",curMilis(),printTag(status.MPI_TAG));
    		tag = status.MPI_TAG;
    		if(tag == WORK){
                qsort (toOrder, ARRAYS_SIZE, sizeof(int), compare);
    			MPI_Send(toOrder,  ARRAYS_SIZE, MPI_INT,0, WORK_DONE, MPI_COMM_WORLD);
    		}
            if(tag != SUICIDE){
                MPI_Send(toOrder,ARRAYS_SIZE, MPI_INT,0, GET_WORK, MPI_COMM_WORLD);
            }
        }while(tag != SUICIDE);
        printf("[%f]@slave[%d] leaving...\n",curMilis(),my_rank);
    }

    MPI_Finalize();

}
