#ifndef __globals_h__
#define __globals_h__

/* Includes: */
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "draw.c"

/* Constants: */

/* Ignore... :-)
 *
 *enum {
 * defaultWidth = 800,
 * defaultHeight = 600
 *};
 */

/* Variables: */
int rank, numnodes;
//MPE_XGraph w;
int width, height;
int numcolors;
T_COLOR *ColorArray;

/* Types: */
typedef double coordinate;

typedef struct {
  coordinate x1, y1, x2, y2;
  int left, top, right, bottom;
  int maxiter;
} MandelRender;

extern MPI_Datatype MandelRenderType;

/* Message tags: */
enum {
  tag_renderthis,
  tag_donerendering,
  tag_flush,
  tag_shutdown,
  tag_taskdone,
  tag_youdrawnow
};

#endif
