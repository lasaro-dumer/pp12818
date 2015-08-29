/**
 ** Draw.c 
 **
 ** Contem funcoes para criar uma simples janela e desenhar nela 
 ** 
 **/

#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>		/* getenv(), etc. */
#include <unistd.h>		/* sleep(), etc.  */

#define BOOL char
#define TRUE  1
#define FALSE 0
#define T_COLOR unsigned long

typedef struct {
  unsigned long x,y;
  T_COLOR c;
 }T_Point;


  Display* display;		/* pointer to X Display structure.           */
  int screen_num;		/* number of screen to place the window on.  */
  unsigned int display_width,
               display_height;	/* height and width of the X display.        */

  Window win;			/* pointer to the newly created window.      */
  GC gc;			/* GC (graphics context) used for drawing    */
				/*  in our window.			     */

// Funcoes que devem ser importadas
void create_simple_window(int width, int height, int x, int y);
void DrawPoint(unsigned int x,unsigned int y,T_COLOR Color,BOOL boolFlush);

void DrawPointIndexMaxNumColor(unsigned int x,unsigned int y,unsigned int indexColor,unsigned int MaxColors,BOOL boolFlush); 
void DrawPointIndexColorArray(unsigned int x,unsigned int y,unsigned int indexColor,T_COLOR ColorArray[],BOOL boolFlush);

void MakeColorArray(unsigned int dNroColors,T_COLOR ColorArray[]);
void destroy_display(void);
T_COLOR HashColor(unsigned int dIndex,unsigned int dNroColors);

// Funcoes internas
void create_gc(Window win, int reverse_video);
void init(void);



/*
 * function: create_simple_window. Creates a window with a white background
 *           in the given size.
 * input:    display, size of the window (in pixels), and location of the window
 *           (in pixels).
 * output:   the window's ID.
 * notes:    window is created with a black border, 2 pixels wide.
 *           the window is automatically mapped after its creation.
 */
void 
create_simple_window(int width, int height, int x, int y)
{
  init();
  {
   int screen_num = DefaultScreen(display);
   int win_border_width = 2;

   /* create a simple window, as a direct child of the screen's */
   /* root window. Use the screen's black and white colors as   */
   /* the foreground and background colors of the window,       */
   /* respectively. Place the new window's top-left corner at   */
   /* the given 'x,y' coordinates.                              */
   win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                             x, y, width, height, win_border_width,
                             BlackPixel(display, screen_num),
                             WhitePixel(display, screen_num));

   /* make the window actually appear on the screen. */
   XMapWindow(display, win);

   /* flush all pending requests to the X server. */
   XFlush(display);

   /* allocate a new GC (graphics context) for drawing in the window. */
   create_gc(win, 0);
   XSync(display, False);
  }
}

void
create_gc(Window win, int reverse_video)
{
  unsigned long valuemask = 0;		/* which values in 'values' to  */
					/* check when creating the GC.  */
  XGCValues values;			/* initial values for the GC.   */
  unsigned int line_width = 2;		/* line width for the GC.       */
  int line_style = LineSolid;		/* style for lines drawing and  */
  int cap_style = CapButt;		/* style of the line's edje and */
  int join_style = JoinBevel;		/*  joined lines.		*/
  int screen_num = DefaultScreen(display);

  gc = XCreateGC(display, win, valuemask, &values);
  if (gc < 0) {
	fprintf(stderr, "XCreateGC: \n");
  }

  /* allocate foreground and background colors for this GC. */
  if (reverse_video) {
    XSetForeground(display, gc, WhitePixel(display, screen_num));
    XSetBackground(display, gc, BlackPixel(display, screen_num));
  }
  else {
    XSetForeground(display, gc, BlackPixel(display, screen_num));
    XSetBackground(display, gc, WhitePixel(display, screen_num));
  }

  /* define the style of lines that will be drawn using this GC. */
  XSetLineAttributes(display, gc,
                     line_width, line_style, cap_style, join_style);

  /* define the fill style for the GC. to be 'solid filling'. */
  XSetFillStyle(display, gc, FillSolid);

  return ;
}




void 
init(void)
{
  char *display_name = getenv("DISPLAY");  /* address of the X display.      */
  
  /* open connection with the X server. */
  display = XOpenDisplay(display_name);
  if (display == NULL) {
    fprintf(stderr, "Program: cannot connect to X server '%s'\n",
            display_name);
    exit(1);
  }

  /* get the geometry of the default screen for our display. */
  screen_num = DefaultScreen(display);
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);

  /* make the new window occupy 1/9 of the screen's size. */
//  width = (display_width / 3);
//  height = (display_height / 3);
//  printf("window width - '%d'; height - '%d'\n", width, height);
}

void 
destroy_display(void)
{
  /* close the connection to the X server. */
  XCloseDisplay(display);
}


void 
DrawPoint(unsigned int x,unsigned int y,T_COLOR Color,BOOL boolFlush)
{
 XSetForeground(display, gc, Color );
 
 XDrawPoint(display, win, gc, x, y);
 if (boolFlush)
    XFlush(display);   
}

void DrawPointIndexMaxNumColor(unsigned int x,unsigned int y,unsigned int indexColor,unsigned int MaxColors,BOOL boolFlush) 
{
 return DrawPoint(x,y,HashColor(indexColor, MaxColors),boolFlush);
}
void DrawPointIndexColorArray(unsigned int x,unsigned int y,unsigned int indexColor,T_COLOR ColorArray[],BOOL boolFlush) 
{
 return DrawPoint(x,y,ColorArray[indexColor],boolFlush); 
}

void MakeColorArray(unsigned int dNroColors,T_COLOR ColorArray[])
{
 int i;
 for ( i=0 ; i < dNroColors ; i++ ) {
    ColorArray[i] = HashColor(i,dNroColors);
 }
}

T_COLOR HashColor(unsigned int dIndex,unsigned int dNroColors)
{
 T_COLOR ReturnColor;
 dIndex = dIndex % dNroColors;
 if (dNroColors == 1) dNroColors = 2;
 ReturnColor = (T_COLOR)((((float)dIndex/(dNroColors-1)))*(0xFFF))*(0x10); 
// printf("(%d / (%d - 1)) = %x\n",dIndex,dNroColors,ReturnColor);
 return (T_COLOR)(ReturnColor);
}


