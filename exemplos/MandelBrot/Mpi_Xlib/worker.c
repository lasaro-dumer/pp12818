#include "worker.h"

static void CalculateAndSendSection (MandelRender *r,int SendTo)
{
  int width = r->right - r->left, height = r->bottom - r->top;
  int numpoints = width * height;
  int x, y;
  coordinate dr = (r->x2 - r->x1) / (r->right - r->left),
             di = (r->y2 - r->y1) / (r->bottom - r->top),
             cr, ci = r->y1;
  T_Point *points = (T_Point *) malloc ((numpoints * sizeof(T_Point))),
          *point = points;
	   
  for (y = r->top; y < r->bottom; y++, (ci += di)) {
    cr = r->x1;
    for (x = r->left; x < r->right; x++, (cr += dr)) {
      int iter;
      register double zr = 0, zi = 0;
      point->x = x;
      point->y = y;
      for (iter = r->maxiter; iter && zr * zr + zi * zi < 4.0; iter--) {
        register double nr = zr * zr - zi * zi + cr;
        zi = zr * zi;
        zi += zi + ci;
        zr = nr;
      }    
#ifdef COLORARRAY
      point->c = iter ? ColorArray[iter % numcolors] : 0;
#else
      point->c = iter ? iter % numcolors : 0;
#endif
      point++;
    }
  }
 MPI_Send (points, numpoints * 3, MPI_UNSIGNED_LONG, SendTo, tag_youdrawnow,
        MPI_COMM_WORLD);
  free (points);
}

void Worker (void){
  MandelRender r;
  MPI_Status status;
  while (1) {
    MPI_Probe (0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == tag_renderthis) {
      MPI_Recv (&r, 1, MandelRenderType, status.MPI_SOURCE, tag_renderthis,
                MPI_COMM_WORLD, &status);
      CalculateAndSendSection (&r,status.MPI_SOURCE);
    } else if (status.MPI_TAG == tag_flush) {
      MPI_Recv (NULL, 0, MPI_INT, status.MPI_SOURCE, tag_flush,
               MPI_COMM_WORLD, &status);
    } else if (status.MPI_TAG == tag_shutdown) {
      MPI_Recv (NULL, 0, MPI_INT, status.MPI_SOURCE, tag_shutdown,
               MPI_COMM_WORLD, &status);
      return;
    } else {
      fprintf (stderr, "Hey!  Unknown tag %d on node %d.\n",
              status.MPI_TAG, rank);
      fflush (stderr);
    }
  }
}
