/*
*****************************************************************************
** xminesweep version 3.0  (c) Copyright Ashley Roll, all rights reserved.
** DATE: 23-June-1994
**
** xminesweep 3.0 is NOT Public Domain. I reserve all rights to it.
** However, I grant the right to copy, modify and distribute to
** anyone for NON-COMMERICAL purposes. On the condition that this and all
** other copyright notices remain intact. If you wish to use xminesweep in
** a commerical package, or charge a fee for distribution, then you will
** require a written consent from me (Ashley Roll).
**
** This software comes with NO warranty whatsoever. I therefore take no
** responsibility for any damages, losses or problems that the program may
** cause.
**
** I can be reached via E-Mail at A.Roll@cit.gu.edu.au or 
**                                ash@splat.paxnet.com.au
*****************************************************************************
*/


#include <X11/Xlib.h>  /* Xt programming includes */
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdio.h>

#ifndef SYSTEM_V
#include <strings.h>
#else
#include <string.h>
#endif

#include <dirent.h> 
#include <X11/cursorfont.h>
#include <X11/Intrinsic.h>      
#include <X11/StringDefs.h>

#include <X11/Xaw/Form.h>      /* widget includes */
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Dialog.h> 
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Viewport.h>

/**************************************************************************
    USER CHANGABLE STUFF
 **************************************************************************/

/* define the size of the 'playing field' */
#define DEFAULT_NUM_MINES 40
#define DEF_WIDTH 20
#define DEF_HEIGHT 20

/* messages displayed to the user */
#define MISSED_SOME "You missed some."
#define YOU_WON "You Won!!"
#define YOU_DIED "You just exploded!"

#define PLAYING "Playing"
#define READY "Ready - Click to begin"

/***************************************************************************
    PROGRAM STUFF - CHANGE IT AT YOUR OWN RISK
 ***************************************************************************/

#ifdef MAIN
#define ext
#else
#define ext extern
#endif
ext XtAppContext appcon;
ext XtIntervalId timer;
ext Widget toplevel,mainpw,corew,countw,timelw,statuslw,numbombtw;
ext Pixmap unknown,marked,bomb,wrongmarked,question,b0,b1,b2,b3,b4,b5,b6,b7,b8;
ext int mines_left;
ext char num_mines_string[5];
ext Display *disp;
ext GC gc,igc;
ext GC cgc[8];
ext Window corewwin;
ext int do_expose_zeros;
ext int dead,timer_started;
ext int Crnt_Time;
ext Boolean sound;

#ifdef CHEAT_OK
ext int cheat;
#endif

/* define the states that a square can have */
#define UNKNOWN 0
#define MARKED 1
#define EXPOSED 2
#define QUESTION 3

#define TRUE 1
#define FALSE 0

/* define an array to hold the 'world' */
typedef struct cell {
  int x,y;  /* top left corner of the bitmap in the core widget */
  char num_bombs_around;
  unsigned status:3;
  unsigned is_bomb:1;
  unsigned clicked_on:1; 
} CELL;

ext CELL *world;

/* this was suggested by jimp@eaglet.messaging.cs.mci.com (Jim Patterson)
 * so that xminesweep would work on 64-bit machines 
 */
typedef struct config_S {
 int numMines;     /* number of mines */
 int width;        /* width of world */
 int height;       /* height of world */
 Boolean sound;    /* sound on=True, off=False */
} Config;

ext Config config;

/* some macros */
#define WIDTH config.width
#define HEIGHT config.height
#define NUM_MINES config.numMines

#define World(w,h,e) (world + ((h)*WIDTH) + (w))->e

#define RANDOM(x) (my_rand() % x)
#define SETBITMAP(w,h,b) XCopyPlane(disp,b,corewwin,gc,0,0,21,21,World(w,h,x), \
                                 World(w,h,y), (unsigned long)1)

#define SETCBITMAP(w,h,b,c) XCopyPlane(disp,b,corewwin,c,0,0,21,21,World(w,h,x), \
                                 World(w,h,y), (unsigned long)1)

#define INVSETBITMAP(w,h,b) XCopyPlane(disp,b,corewwin,igc,0,0,21,21, \
                                 World(w,h,x),World(w,h,y), (unsigned long)1)

