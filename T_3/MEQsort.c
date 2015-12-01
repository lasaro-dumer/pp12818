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
#define NUM_ARRAYS 2
#define ARRAYS_SIZE 20
#define FALSE 0
#define TRUE 1
//#define DEBUG 1
//#define PRINTV 0

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
    int *vetor_aux;
    double t1, t2;

    srand(time(NULL));
    int r = rand();

    int qkSort = FALSE;
    int parallel = TRUE;
    size_t optind;
    for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
        switch (argv[optind][1]) {
            case 'q': qkSort = TRUE; break;
            case 's': parallel = FALSE; break;
            default:
            fprintf(stderr, "Usage: %s [-q]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // *argv points to the remaining non-option arguments.
    // If *argv is NULL, there were no non-option arguments.

    // ...

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
            #ifdef PRINTVb
            printf("vetor={");
            for (i=0 ; i<ARRAYS_SIZE; i++)              /* init array with worst case for sorting */
                printf("%d,",ordered[i] );
            printf("}\n");
            #endif
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
                if(parallel){
                    omp_set_num_threads(4); // disparar 4 threads pois se trata de uma m�quina Quad-Core
                    if(qkSort)
                    {
                        int th_id;
                        int nthreads;

                        int vetor_auxiliar[(ARRAYS_SIZE/4)];
                        int toOrderCopy[ARRAYS_SIZE];

                        for (i=0;i<=ARRAYS_SIZE;i++)
                        {
                            toOrderCopy[i] = toOrder[i];
                        }

                        #ifdef PRINTV
                        printf("vetor={");
                        for (i=0 ; i<ARRAYS_SIZE; i++)              /* init array with worst case for sorting */
                            printf("%d,",toOrder[i] );
                        printf("}\n");
                        #endif

                        #pragma omp parallel private(th_id, nthreads, vetor_auxiliar) shared(toOrder)
                        {
                            th_id = omp_get_thread_num();
                            nthreads = omp_get_num_threads();
                            int ini = 0;
                            int end = 0;

                            ini = (th_id)*(ARRAYS_SIZE/nthreads);
                            end = ini+(ARRAYS_SIZE/nthreads);

                            for (i=0;i<(ARRAYS_SIZE/nthreads);i++)
                            {
                                vetor_auxiliar[i] = toOrderCopy[ini+i];
                            }

                            //printf("t[%d]:%d -> %d\n",th_id,ini,end );
                            #ifdef PRINTV
                            printf("vetor_AUXILIAR={");
                            for (i=0 ; i<(ARRAYS_SIZE/nthreads); i++)              /* init array with worst case for sorting */
                                printf("%d,",vetor_auxiliar[i] );
                            printf("}\n");
                            #endif


                            qsort (&vetor_auxiliar[0], (ARRAYS_SIZE/nthreads), sizeof(int), compare);//ordena o vetor

                            #pragma omp critical
                            {
                                for (i=0;i<(ARRAYS_SIZE/nthreads);i++)
                                {
                                    ini = ARRAYS_SIZE-((th_id+1)*(ARRAYS_SIZE/nthreads));
                                    toOrder[ini+i] = vetor_auxiliar[i];
                                }
                            }
                        }

                        #ifdef PRINTV
                        printf("vetor AFTER={");
                        for (i=0 ; i<ARRAYS_SIZE; i++)              /* init array with worst case for sorting */
                            printf("%d,",toOrder[i] );
                        printf("}\n");
                        #endif
                        //interleaving(toOrder,ARRAYS_SIZE,4);
                        //vetor_aux = interleavingNew(toOrder,ARRAYS_SIZE,4);

                    }
                    else // BUBBLESORT
                    {
                        int th_id;
                        int nthreads;

                        int vetor_auxiliar[(ARRAYS_SIZE/4)];
                        int toOrderCopy[ARRAYS_SIZE];

                        for (i=0;i<=ARRAYS_SIZE;i++)
                        {
                            toOrderCopy[i] = toOrder[i];
                        }


                        #ifdef PRINTV
                        printf("vetor={");
                        for (i=0 ; i<ARRAYS_SIZE; i++)              /* init array with worst case for sorting */
                            printf("%d,",toOrder[i] );
                        printf("}\n");
                        #endif

                        #pragma omp parallel private(th_id, nthreads, vetor_auxiliar) shared(toOrder)
                        {
                            th_id = omp_get_thread_num();
                            nthreads = omp_get_num_threads();
                            int ini = 0;
                            int end = 0;

                            ini = (th_id)*(ARRAYS_SIZE/nthreads);
                            end = ini+(ARRAYS_SIZE/nthreads);

                            for (i=0;i<(ARRAYS_SIZE/nthreads);i++)
                            {
                                vetor_auxiliar[i] = toOrderCopy[ini+i];
                            }

                            //#ifdef PRINTV
                            //printf("vetor_AUXILIAR={");
                            //for (i=0 ; i<(ARRAYS_SIZE/nthreads); i++)              /* init array with worst case for sorting */
                            //    printf("%d,",vetor_auxiliar[i] );
                            //printf("}\n");
                            //#endif

                            //printf("t[%d]:%d -> %d\n",th_id,ini,end );
                            bs ((ARRAYS_SIZE/nthreads),&vetor_auxiliar[0]);//ordena o vetor

                            //#ifdef PRINTV
                            //printf("vetor_AUXILIAR DPS={");
                            //for (i=0 ; i<(ARRAYS_SIZE/nthreads); i++)              /* init array with worst case for sorting */
                            //    printf("%d,",vetor_auxiliar[i] );
                            //printf("}\n");
                            //#endif

                            #pragma omp critical
                            {
                                for (i=0;i<(ARRAYS_SIZE/nthreads);i++)
                                {
                                    ini = ARRAYS_SIZE-((th_id+1)*(ARRAYS_SIZE/nthreads));
                                    toOrder[ini+i] = vetor_auxiliar[i];
                                }
                            }
                        }

                        #ifdef PRINTV
                        printf("vetor AFTER={");
                        for (i=0 ; i<ARRAYS_SIZE; i++)              /* init array with worst case for sorting */
                            printf("%d,",toOrder[i] );
                        printf("}\n");
                        #endif
                        //interleaving(toOrder,ARRAYS_SIZE,4);
                        //vetor_aux = interleavingNew(toOrder,ARRAYS_SIZE,4);
                    }
                }
                else{
                    if(qkSort){
                        qsort (toOrder, ARRAYS_SIZE, sizeof(int), compare);//ordena o vetor
                    }
                    else{
                        bs(ARRAYS_SIZE,toOrder);
                        /*
                        for (i = ARRAYS_SIZE-1; i >=1 ; i--){
                            for (j = 0; j < i; j++){
                                if (toOrder[j] > toOrder[j+1])
                                {
                                    s = toOrder[j];
                                    toOrder[j] = toOrder[j+1];
                                    toOrder[j+1] = s;
                                }
                            }
                        }//*/
                    }
                }
                MPI_Send(toOrder,ARRAYS_SIZE, MPI_INT,0, WORK_DONE, MPI_COMM_WORLD);//envia o vetor para o mestre
            }
        }while(tag != SUICIDE);//se a ultima tag foi a de suicidio, termina execução
    }
    MPI_Finalize();
}
