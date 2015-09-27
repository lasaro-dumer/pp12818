#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define NUM_ARRAYS 1000
#define ARRAYS_SIZE 10000
#define DEBUG 0

int compare (const void * a, const void * b){
    return ( *(int*)a - *(int*)b );
}
const double curMilis(){
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    return ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000.0) +0.5; // convert tv_sec & tv_usec to millisecond
}

main(int argc, char** argv){
    int **saco;
    int i,j,next;
    double t1, t2;

    srand(time(NULL));
    int r = rand();

    t1 = curMilis();        // contagem de tempo inicia neste ponto
    saco = (int **)malloc(NUM_ARRAYS * sizeof(int *));
    if(saco == NULL)
    {
        printf("out of memory\n");
        return -1;
    }else{
        int valor = ARRAYS_SIZE * NUM_ARRAYS;
        for(i = 0; i < NUM_ARRAYS; i++)
        {
            saco[i] =(int *) malloc(ARRAYS_SIZE * sizeof(int));
            if(saco[i] == NULL)
            {
                printf("out of memory row %d\n",i);
                return -1;
            }else{
                if(DEBUG){
                    printf("array[%d]={",i);
                }
                for(j = 0; j< ARRAYS_SIZE; j++)
                {
                    saco[i][j] = valor;
                    if(DEBUG){
                        printf("%d,",saco[i][j]);
                    }
                    valor--;
                }
                if(DEBUG){
                    printf("}\n");
                }
            }
        }
    }

    next=0;
    printf("[%f]@master ordering...\n",curMilis());
    while(next<NUM_ARRAYS){
        qsort (saco[next], ARRAYS_SIZE, sizeof(int), compare);
        next++;
    }
    printf("[%f]@master done ordering. total=%d/%d...\n",curMilis(),next,NUM_ARRAYS);

    printf("[%f]@master leaving...\n",curMilis());

    t2 = curMilis();        // contagem de tempo termina neste ponto

    printf("[%f]@MPI_Wtime measured work time to be: %1.2f\n",curMilis(), t2-t1);

}
