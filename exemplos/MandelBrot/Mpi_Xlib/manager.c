#include "manager.h"
#include <sys/time.h>
#define NP 600 //numero de fatias - ESTE VALOR SERA ALTERADO NO EXERCICIO
#define IT 17500 //numero maximo de iteracoes
#define REP 10 // numero de repeticoes
char *workersbusy;
int workersfree;

void DrawThis(T_Point *points,int numpoints);

static int ReceiveDonePacket (void){
  int numpoints = (width * height) / NP;   
  T_Point *points;
  MPI_Status status;
  points = (T_Point *) malloc (numpoints * sizeof(T_Point));	    
  MPI_Recv (points, numpoints * 3, MPI_UNSIGNED_LONG, MPI_ANY_SOURCE, tag_youdrawnow,
            MPI_COMM_WORLD, &status);
#if GRAFICO
  DrawThis(points,numpoints);    
#endif
  workersbusy[status.MPI_SOURCE]--;
  workersfree++;
  return status.MPI_SOURCE;
}

static void Manager_Draw (MandelRender *r){
  int sectHeight;
  coordinate sectDeltaY;
  MandelRender section;
  int who;
  sectHeight = height / NP; 
  if (sectHeight <= 0)
    sectHeight = 1;
  sectDeltaY = (r->y2 - r->y1) / (r->bottom - r->top) * sectHeight;
  section = *r;
  while (1) {
    section.bottom = section.top + sectHeight;
    section.y2 = section.y1 + sectDeltaY;
    if (section.bottom > r->bottom) {
      section.bottom = r->bottom;
      section.y2 = r->y2;
    }
    if (workersfree) 
      for (who = 1; who < numnodes && workersbusy[who] == 2; who++);
    else 
      who = ReceiveDonePacket ();      
    MPI_Send (&section, 1, MandelRenderType, who, tag_renderthis,
              MPI_COMM_WORLD);
    workersbusy[who]++;
    workersfree--;
    if (section.bottom == r->bottom)
      break;
    section.top = section.bottom;
    section.y1 = section.y2;
  }
  for (who = 1; who < numnodes; who++)
    MPI_Send (NULL, 0, MPI_INT, who, tag_flush, MPI_COMM_WORLD);
  while (workersfree != (numnodes - 1) * 2)
    ReceiveDonePacket ();
}

double time2double (struct timeval *t){
	return (t->tv_sec + t->tv_usec / 1000000.0);
}

void Manager (void){
  MandelRender whole = { -2.0, 1.5, 1.0, -1.5,
                         0, 0, -1, -1, IT };
  MandelRender r;
  float pontos[15][4]={{-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5},
                        {-2.0,1.0,1.5,-1.5}};
  int ponto=0;		     		       
  
  struct timeval ti, tf;
  double fTempoTotal = 0.0;
  workersbusy = (char *) calloc (sizeof(char), numnodes);
  workersfree = (numnodes - 1) * 2;   
  whole.right = width;
  whole.bottom = height;
  r = whole;
  for (;ponto < REP; ponto++) {
     r.x1=pontos[0][0]; 
     r.x2=pontos[0][1];             
     r.y1=pontos[0][2];
     r.y2=pontos[0][3];  
     gettimeofday (&ti, NULL);    
     Manager_Draw (&r);   
     gettimeofday (&tf, NULL);    
     printf("%d\t%f\n",ponto,(time2double (&tf) - time2double (&ti))); fflush(stdout);
     fTempoTotal += ((time2double (&tf) - time2double (&ti)));
  }
  printf("\nTotal\t%f\n",fTempoTotal);
  printf("Media\t%f\n",fTempoTotal/REP);
  
  { int who;
    for (who = 1; who < numnodes; who++)
      MPI_Send (NULL, 0, MPI_INT, who, tag_shutdown, MPI_COMM_WORLD);
  }
}

void DrawThis(T_Point *points,int numpoints){
 int i;
 for ( i=0 ; i<numpoints ; i++ ){
#ifdef COLORARRAY 
  DrawPoint(points->x,points->y,points->c,!TRUE);  
#else
  DrawPointIndexMaxNumColor(points->x,points->y,points->c,numcolors,!TRUE); 
#endif
  points++;
 }
  DrawPointIndexMaxNumColor(points->x,points->y,points->c,numcolors,TRUE); 
}
