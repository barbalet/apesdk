/****************************************************************

  linux.c - Linux Implementation of the Generic Platform Interface
 
 =============================================================
 
 Copyright 1996-2011 Tom Barbalet. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person 
 obtaining a copy of this software and associated documentation 
 files (the "Software"), to deal in the Software without 
 restriction, including without limitation the rights to use, 
 copy, modify, merge, publish, distribute, sublicense, and/or 
 sell copies of the Software, and to permit persons to whom the 
 Software is furnished to do so, subject to the following 
 conditions:
 
 The above copyright notice and this permission notice shall be 
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
 OTHER DEALINGS IN THE SOFTWARE.
 
 This software and Noble Ape are a continuing work of Tom Barbalet, 
 begun on 13 June 1996. No apes or cats were harmed in the writing 
 of this software.
 
 ****************************************************************/
/*NOBLEMAKE VAR=""*/
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Form.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include <time.h>
#include <stdlib.h>

#ifdef GPI_MEMORY_USED
void * plat_new(unsigned long bytes, unsigned char critical);
void plat_free(void * ptr);
#endif

/*NOBLEMAKE DEL=""*/
#include "gpi.h"
/*NOBLEMAKE END=""*/

/*NOBLEMAKE END=""*/

#ifdef GPI_MOUSE_FUNCTION
unsigned char		firedown = 0;
#endif

void plat_close();

#if GPI_MEMORY_USED
void * plat_new(unsigned long bytes, unsigned char critical){
void *	tmp = 0L;
	tmp = (void *) malloc(bytes);
	if(tmp == 0L && critical){
		exit(EXIT_SUCCESS);
	}
	return( tmp );
}

void plat_free(void * ptr){
	if( ptr != 0L )
		free(ptr);
}

#endif


/* Mac
#define	DEFINE_WHITE		0
#define	REVERSE_BIT		0 */
/* Win
#define	DEFINE_WHITE		255
#define	REVERSE_BIT		0 */

#define	DEFINE_WHITE		0
#define	REVERSE_BIT		1

#if REVERSE_BIT
  #define  BIT_SPAN(spx)  (1<<((spx)&7))
#else
  #define  BIT_SPAN(spx)  (128>>((spx)&7))
#endif


void plat_erase(unsigned char * buffer) {
  unsigned long  lp = 0;
  while (lp < ((512*512)>>3))
    buffer[lp++] = DEFINE_WHITE;
}

int plat_pointget(unsigned char * buffer, int px, int py) {
  unsigned char check_value = BIT_SPAN(px);
#if DEFINE_WHITE
  return ((buffer[((px)>>3)|((py)<<6)]&check_value) == 0);
#else
  return ((buffer[((px)>>3)|((py)<<6)]&check_value) == check_value);
#endif
}

void plat_pointset(unsigned char * buffer, int px, int py) {
#if DEFINE_WHITE
  buffer[((px)>>3)|((py)<<6)]&=255^(BIT_SPAN(px));
#else
  buffer[((px)>>3)|((py)<<6)]|=BIT_SPAN(px);
#endif
}

void plat_pointerase(unsigned char * buffer, int px, int py) {
#if DEFINE_WHITE
  buffer[((px)>>3)|((py)<<6)]|=BIT_SPAN(px);
#else
  buffer[((px)>>3)|((py)<<6)]&=255^(BIT_SPAN(px));
#endif
}


/** Functions **/
#ifdef GPI_MENU_FUNCTION

/** Menu Functions **/
void CreateMenuBar(Widget parent);

/** Menu constants **/
const int MAX_MENUITEMS = 100;

/** Menu Callbacks **/
void exitCommand(Widget w, XtPointer client_data, XtPointer callback_data);
void menuCommand(Widget w, XtPointer client_data, XtPointer callback_data);
#endif

/** Callbacks **/
#ifdef GPI_KEY_FUNCTION
void input(Widget w, XtPointer client_data, XtPointer callback_data);
#endif

void expose(Widget w, XtPointer client_data, XtPointer callback_data);
void repaint(XtPointer client_data, XtIntervalId* id);

#ifdef GPI_MOUSE_FUNCTION
void mouseMove(Widget w, XtPointer client_data, XEvent* event, Boolean *cont);
#endif

/** Globals **/
Pixmap pixmap, pixmap_buffer;
GC gc;
XtAppContext app;
Display* display;
Window rootw;
Pixel fg, bg;
unsigned int depth;
Widget drawingArea;

unsigned char* draw_buffer;

int main(int argc, char** argv)
{
  Widget topWidget, form;
  int screen;
  unsigned int width;
  XGCValues gcv;
  long int fill_pixel = 1;

  topWidget = XtVaAppInitialize(&app, GPI_WINDOW_NAME, NULL, 0, &argc, argv, NULL, NULL);

  XtVaSetValues(topWidget, XtNwidth, GPI_DIMENSION_X /*+ 6*/, 
 		           XtNheight, GPI_DIMENSION_Y /*+ 32*/, 
		           XtNtitle, GPI_WINDOW_NAME,
		           NULL);

  display = XtDisplay(topWidget);
  rootw = XDefaultRootWindow(display);
  screen = XDefaultScreen(display);
  depth = XDefaultDepth(display, screen);

  form = XmCreateForm(topWidget, "Main Form Window", NULL, 0);

#ifdef GPI_MENU_FUNCTION
  CreateMenuBar(form);
#endif

  drawingArea = XmCreateDrawingArea(form, "Drawing Area", NULL, 0);

  XtVaSetValues(drawingArea, XmNbackground, WhitePixelOfScreen(XtScreen(topWidget)), 
		             XtNwidth, GPI_DIMENSION_X,
		             XtNheight, GPI_DIMENSION_Y,
		             NULL);

#ifdef GPI_MENU_FUNCTION
  XtVaSetValues(drawingArea, XtNy, 32, NULL);
#endif

#ifdef GPI_MOUSE_FUNCTION
  XtAddEventHandler(drawingArea, Button1MotionMask, FALSE, mouseMove, NULL);
#endif
			      
  XtManageChild(drawingArea);

  XtVaGetValues(drawingArea, XmNforeground, &fg,
		XmNbackground, &bg, NULL);

  /** This will mostly be moved to the init function... **/
  draw_buffer = (unsigned char*)GPI_INIT_FUNCTION(time(NULL));

  /** Create a GC.. Attach it to the drawing area.. **/
  gcv.foreground = BlackPixelOfScreen(XtScreen(drawingArea));
  gc = XCreateGC(XtDisplay(drawingArea), rootw, GCForeground, &gcv);

  pixmap = XCreatePixmap(display, rootw, GPI_DIMENSION_X, GPI_DIMENSION_Y, depth);
  pixmap_buffer = XCreatePixmap(display, rootw, GPI_DIMENSION_X, GPI_DIMENSION_Y, depth);

  XSetForeground(XtDisplay(drawingArea), gc, WhitePixelOfScreen(XtScreen(drawingArea)));
  XFillRectangle(display, pixmap, gc, 0, 0, GPI_DIMENSION_X, GPI_DIMENSION_Y);
  XSetForeground(display, gc, fill_pixel);

  XtVaSetValues(topWidget, XmNworkWindow, drawingArea, NULL);

#ifdef GPI_KEY_FUNCTION
  XtAddCallback(drawingArea, XmNinputCallback, input, NULL);
#endif
  XtAddCallback(drawingArea, XmNexposeCallback, expose, NULL);

  /** Initialize the timer... **/
  XtAppAddTimeOut(app, (unsigned long)50, repaint, NULL);

  XtManageChild(form);
  XtRealizeWidget(topWidget);
  XtAppMainLoop(app);
}

#ifdef GPI_MENU_FUNCTION
void CreateMenuBar(Widget parent)
{
  /** Hierarchy
     parent -- File
               -- New
	       -- <Separator>
	       -- Exit

	    -- Edit
	       -- Undo
	       -- <Separator>
	       -- Cut
	       -- Copy
	       -- Paste

	    -- Control
	       -- Brain
	       -- Terrain
	       -- Map
	       -- <Separator>
	       -- Previous Ape
	       -- Next Ape
	       -- <Separator>
	       -- Pause
	       -- <Separator>
	       -- About
  **/

  /** Declarations **/
  Widget menuBar, fileSubMenu, editSubMenu, controlSubMenu;
  Widget newMenuButton, exitMenuButton;
  Widget undoMenuButton, cutMenuButton, copyMenuButton, pasteMenuButton;
  Widget brainMenuButton, terrainMenuButton, mapMenuButton, prevMenuButton, nextMenuButton, 
    pauseMenuButton, aboutMenuButton;

  /** No freeing. Freeing done on app. close by the OS... **/
  int *menu_item_index = (int*)malloc( sizeof(int) * 100); 

  /** Set menu key numbers to pass to GPI_MENU_FUNCTION **/
  menu_item_index[0] = 1;    /* New */
  menu_item_index[1] = 3;    /*Exit */ 
  menu_item_index[2] = 256;  /*Undo */
  menu_item_index[3] = 257;  /*Cut */
  menu_item_index[4] = 258;  /*Copy */
  menu_item_index[5] = 259;  /*Paste */
  menu_item_index[6] = 17;   /*Brain */
  menu_item_index[7] = 18;   /*Terrain */
  menu_item_index[8] = 19;   /*Map */
  menu_item_index[9] = 21;   /*Previous Ape */
  menu_item_index[10] = 22;  /*Next Ape */
  menu_item_index[11] = 24;  /*Pause */
  menu_item_index[12] = 255; /*About */

  menuBar = XmCreateMenuBar(parent, "Menu Bar", NULL, 0);

  /** File Menu **/
  fileSubMenu = XmCreatePulldownMenu(menuBar, "fileSubmenu", NULL, 0);
  XtVaCreateManagedWidget("File", xmCascadeButtonWidgetClass, menuBar, 
				    XmNsubMenuId, fileSubMenu, NULL);

  newMenuButton = XtCreateManagedWidget("New", xmPushButtonWidgetClass, fileSubMenu, NULL, 0);
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass, fileSubMenu, NULL, 0);
  exitMenuButton = XtCreateManagedWidget("Exit", xmPushButtonWidgetClass, fileSubMenu, NULL, 0);

  XtAddCallback(newMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[0]);
  XtAddCallback(exitMenuButton, XmNactivateCallback, exitCommand, &menu_item_index[1]);

  /** Edit Menu **/
  editSubMenu = XmCreatePulldownMenu(menuBar, "editSubmenu", NULL, 0);
  XtVaCreateManagedWidget("Edit", xmCascadeButtonWidgetClass, menuBar, 
			  XmNsubMenuId, editSubMenu, NULL);

  undoMenuButton = XtCreateManagedWidget("Undo", xmPushButtonWidgetClass, editSubMenu, NULL, 0);
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass, editSubMenu, NULL, 0);
  cutMenuButton = XtCreateManagedWidget("Cut", xmPushButtonWidgetClass, editSubMenu, NULL, 0);
  copyMenuButton = XtCreateManagedWidget("Copy", xmPushButtonWidgetClass, editSubMenu, NULL, 0);
  pasteMenuButton = XtCreateManagedWidget("Paste", xmPushButtonWidgetClass, editSubMenu, NULL, 0);

  XtAddCallback(undoMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[2]);
  XtAddCallback(cutMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[3]);
  XtAddCallback(copyMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[4]);
  XtAddCallback(pasteMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[5]);

  /** Control Menu **/
  controlSubMenu = XmCreatePulldownMenu(menuBar, "controlSubmenu", NULL, 0);
  XtVaCreateManagedWidget("Control", xmCascadeButtonWidgetClass, menuBar, 
			  XmNsubMenuId, controlSubMenu, NULL);

  brainMenuButton = XtCreateManagedWidget("Brain", xmPushButtonWidgetClass, controlSubMenu, NULL, 0);
  terrainMenuButton = XtCreateManagedWidget("Terrain", xmPushButtonWidgetClass, controlSubMenu, NULL, 0);
  mapMenuButton = XtCreateManagedWidget("Map", xmPushButtonWidgetClass, controlSubMenu, NULL, 0);
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass, controlSubMenu, NULL, 0);
  prevMenuButton = XtCreateManagedWidget("Previous Ape", xmPushButtonWidgetClass, controlSubMenu, NULL, 0);
  nextMenuButton = XtCreateManagedWidget("Next Ape", xmPushButtonWidgetClass, controlSubMenu, NULL, 0);
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass, controlSubMenu, NULL, 0);
  pauseMenuButton = XtCreateManagedWidget("Pause", xmPushButtonWidgetClass, controlSubMenu, NULL, 0);
  XtCreateManagedWidget("separator", xmSeparatorWidgetClass, controlSubMenu, NULL, 0);
  aboutMenuButton = XtCreateManagedWidget("About", xmPushButtonWidgetClass, controlSubMenu, NULL, 0);

  XtAddCallback(brainMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[6]);
  XtAddCallback(terrainMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[7]);
  XtAddCallback(mapMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[8]);
  XtAddCallback(prevMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[9]);
  XtAddCallback(nextMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[10]);
  XtAddCallback(pauseMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[11]);
  XtAddCallback(aboutMenuButton, XmNactivateCallback, menuCommand, &menu_item_index[12]);

  XtManageChild(menuBar);
}
#endif

void expose(Widget w, XtPointer client_data, XtPointer callback_data)
{
  XmDrawingAreaCallbackStruct* cb = (XmDrawingAreaCallbackStruct*)callback_data;
  XCopyArea(cb->event->xexpose.display, pixmap, cb->window, gc, 
	    0, 0, GPI_DIMENSION_X, GPI_DIMENSION_Y, 0, 0);
}

void repaint(XtPointer client_data, XtIntervalId* id)
{
  Pixmap temp;

  XCopyArea(XtDisplay(drawingArea), pixmap, XtWindow(drawingArea), gc, 
		      0, 0, GPI_DIMENSION_X, GPI_DIMENSION_Y, 0, 0);

  GPI_UPDATE_FUNCTION(1);

  pixmap_buffer = XCreatePixmapFromBitmapData(display, rootw, draw_buffer, GPI_DIMENSION_X, GPI_DIMENSION_Y, 
				       (unsigned long)fg, (unsigned long)bg, depth);
  pixmap = pixmap_buffer;  
  XtAppAddTimeOut(app, (unsigned long)50, repaint, NULL);

  GPI_UPDATE_FUNCTION(0);
}

#ifdef GPI_MOUSE_FUNCTION
void mouseMove(Widget w, XtPointer client_data, XEvent* event, Boolean* cont)
{
  if (GPI_MOUSE_FUNCTION(event->xmotion.x, event->xmotion.y))
  {}
  return;
}
#endif

void plat_close()
{
#ifdef GPI_EXIT_FUNCTION
  GPI_EXIT_FUNCTION();
#endif
  exit(EXIT_SUCCESS);
}



#ifdef GPI_MENU_FUNCTION
void menuCommand(Widget w, XtPointer client_data, XtPointer callback_data)
{
  if (client_data != NULL)
    {
      int key_value = *(int *)(client_data);
      GPI_MENU_FUNCTION(key_value);
    }
}

void exitCommand(Widget w, XtPointer client_data, XtPointer callback_data)
{
  plat_close();
}
#endif



#ifdef GPI_KEY_FUNCTION
void input(Widget w, XtPointer client_data, XtPointer callback_data)
{
  KeySym key;
  char buf[10];
  int len;
  XmDrawingAreaCallbackStruct* cb = (XmDrawingAreaCallbackStruct*)callback_data;
  if (cb->event->xany.type == KeyPress)
    {
      len = XLookupString((XKeyEvent*)cb->event, buf, 10, &key, NULL);
      
      switch(key)
	{
	case XK_Left:   /**(65361) **/
	case XK_Down:   /**(65364) **/
	  GPI_KEY_FUNCTION(key - 65333);
	  break;
	case XK_Up:     /**(65362) **/
	  GPI_KEY_FUNCTION(key - 65332);
	  break;
	case XK_Right:  /**(65363) **/
	  GPI_KEY_FUNCTION(key - 65334);
	  break;
	default:
	  break;
	}
    }
}
#endif
