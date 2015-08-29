/* mandel.c
 * -- fixme!
 */

/*
 * Coloque em 1 para habilitar a saida grafica.
 */
#define GRAFICO 0

/* novamente, FIXME... */
#include "manager.c"
#include "globals.h"
#include "worker.c"

MPI_Datatype MandelRenderType;

void Setup_Types (void){
  int blockcounts[] = { 4, 5 };
  MPI_Aint offsets[] = { 0, 4 * sizeof(double) };
  MPI_Datatype types[2];
  types[0] = MPI_DOUBLE;
  types[1] = MPI_INT;
  MPI_Type_struct (2, blockcounts, offsets, types, &MandelRenderType);
  MPI_Type_commit (&MandelRenderType);
}

int main (int argc, char **argv){
  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &numnodes);
  Setup_Types();
  height=600;
  width=800;
  numcolors = 256;
#if GRAFICO && defined(COLORARRAY)
  ColorArray = (T_COLOR *) malloc(numcolors * sizeof(T_COLOR));
  MakeColorArray(numcolors,ColorArray);
#endif
  if (!rank){
#if GRAFICO
    create_simple_window(width, height, 0, 0);
#endif
    Manager();
#if GRAFICO
    destroy_display();
#endif
  }else{
    Worker();
  }
  MPI_Finalize ();
  exit(0);
}
