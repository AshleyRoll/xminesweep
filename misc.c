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

#ifndef SYSTEM_V
#include <sys/time.h>
#else
#include <sys/time.h>
#endif

#include "xminesweep.h"
#include "bitmaps/unknown.xbm"
#include "bitmaps/marked.xbm"
#include "bitmaps/bomb.xbm"
#include "bitmaps/wrongmarked.xbm"
#include "bitmaps/question.xbm"
#include "bitmaps/b0.xbm"
#include "bitmaps/b1.xbm"
#include "bitmaps/b2.xbm"
#include "bitmaps/b3.xbm"
#include "bitmaps/b4.xbm"
#include "bitmaps/b5.xbm"
#include "bitmaps/b6.xbm"
#include "bitmaps/b7.xbm"
#include "bitmaps/b8.xbm"

extern void Start_Clock();
extern void Stop_Clock();

void place_mines();
void expose();
void make_noise();
void make_other_noise();

static int seed; /* global seed */

void seed_my_rand(val)
int val;
{
  seed = val;
}


int  my_rand()
{
static int  a = 16807,
         m = 2147483647,
         q = 12773, /* q = m div a */
         r = 2836, /* r = m mod a */
         lo,hi,test;

  hi = seed / q;
  lo = seed % q;
  test = a * lo - r * hi;
  if (test > 0)
    seed = test;
  else
    seed = test + m;

 return(seed );
}

/*
 * create all the bitmaps 
 */
void init_bitmaps()
{
  unknown = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            unknown_bits,unknown_width,unknown_height);

  marked = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            marked_bits,marked_width,marked_height);

  bomb = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            bomb_bits,bomb_width,bomb_height);

  wrongmarked = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            wrongmarked_bits,wrongmarked_width,wrongmarked_height);

  question = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            question_bits,question_width,question_height);

  b0 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b0_bits,b0_width,b0_height);

  b1 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b1_bits,b1_width,b1_height);

  b2 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b2_bits,b2_width,b2_height);

  b3 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b3_bits,b3_width,b3_height);

  b4 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b4_bits,b4_width,b4_height);

  b5 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b5_bits,b5_width,b5_height);

  b6 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b6_bits,b6_width,b6_height);

  b7 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b7_bits,b7_width,b7_height);

  b8 = XCreateBitmapFromData(XtDisplay(toplevel),
            DefaultRootWindow(XtDisplay(toplevel)),
            b8_bits,b8_width,b8_height);

}

/*
 * initialise the world
 */
void init_world()
{
register int width,height;


  /* place the mines */
  place_mines();
  
  /* set all the CELL status's to UNKNOWN */
  /* and sett the bitmap to 'unknown' */
  for(width=0; width < WIDTH; width++)
    for(height=0; height < HEIGHT; height++)
    {
      SETBITMAP(width,height,unknown); 
      World(width,height,status) = UNKNOWN;
      World(width,height,clicked_on) = FALSE;
    }

    XFlush(disp);
}

void create_buttons()
{
register int width,height;



  /* and create all the label widgets  */
  for(width=0; width < WIDTH; width++)
    for(height=0; height < HEIGHT; height++)
    {
      World(width,height,x) = width * 21;
      World(width,height,y) = height * 21;
    }


}
/*
 * Place the mines in the world and calculate the number of 
 * mines around each CELL
 */
void place_mines()
{
register int t,width,height;
int hits=0;

  /* seed the rand function */
  {
  time_t ti;
  seed_my_rand(time(&ti));
  }

  /* set all the CELL to !is_bomb */
  for(width=0; width < WIDTH; width++)
    for(height=0; height < HEIGHT; height++)
       World(width,height,is_bomb) = FALSE;

  /* place the bombs */
  for(t = 0; t < NUM_MINES; t++)
  {
    height = RANDOM(HEIGHT);
    width = RANDOM(WIDTH);

    while (World(width,height,is_bomb) == TRUE) {
        width = RANDOM(WIDTH);
        height = RANDOM(HEIGHT);
    }
    World(width,height,is_bomb) = TRUE;
  }

  /* calculate the number of mines around each square */
  for(width=0; width < WIDTH; width++)
    for(height=0; height < HEIGHT; height++) {
    int count;
    int left_ok,right_ok,up_ok,down_ok;
      
      count = 0;
      left_ok = (width-1 >=0) ? TRUE:FALSE;
      right_ok = (width+1 < WIDTH) ? TRUE:FALSE;
      up_ok = (height-1 >=0) ? TRUE:FALSE;
      down_ok = (height+1 < HEIGHT) ? TRUE:FALSE;

      if(left_ok) 
        if(World(width-1,height,is_bomb)) count++;  /* left */

      if(right_ok)
        if(World(width+1,height,is_bomb)) count++;  /* right */

      if(up_ok)
        if(World(width,height-1,is_bomb)) count++;  /* up */

      if(down_ok)
        if(World(width,height+1,is_bomb)) count++;  /* down */

      if(left_ok && up_ok)  
        if(World(width-1,height-1,is_bomb)) count++;  /* up left */
        
      if(left_ok && down_ok)
        if(World(width-1,height+1,is_bomb)) count++;  /* down left */

      if(right_ok && up_ok)
        if(World(width+1,height-1,is_bomb)) count++;  /* up right */

      if(right_ok &&down_ok)
        if(World(width+1,height+1,is_bomb)) count++;  /* down right */

      World(width,height,num_bombs_around) = count;
    }
}

/*
 * This achieves exposing all linked squares with Zero bombs around it
 * it recursevly calles expose() and thus my be called by expose again
 */
void expose_zeros(x,y)
int x,y;
{
int left_ok,right_ok,up_ok,down_ok;

      left_ok = (x-1 >=0) ? TRUE:FALSE;
      right_ok = (x+1 < WIDTH) ? TRUE:FALSE;
      up_ok = (y-1 >=0) ? TRUE:FALSE;
      down_ok = (y+1 < HEIGHT) ? TRUE:FALSE;

      if(left_ok)
        if(World(x-1,y,status) == UNKNOWN)   /* left */
          expose(x-1,y);

      if(right_ok)
        if(World(x+1,y,status) == UNKNOWN)   /* right */
          expose(x+1,y);

      if(up_ok)
        if(World(x,y-1,status) == UNKNOWN)   /* up */
          expose(x,y-1);

      if(down_ok)
        if(World(x,y+1,status) == UNKNOWN)   /* down */
          expose(x,y+1);

      if(left_ok && up_ok)
        if(World(x-1,y-1,status) == UNKNOWN) /* up left */
          expose(x-1,y-1);

      if(left_ok && down_ok)
        if(World(x-1,y+1,status) == UNKNOWN)  /* down left */
           expose(x-1,y+1);

      if(right_ok && up_ok)
        if(World(x+1,y-1,status) == UNKNOWN) /* up right */
          expose(x+1,y-1);

      if(right_ok &&down_ok)
        if(World(x+1,y+1,status) == UNKNOWN)  /* down right */
          expose(x+1,y+1);


}


/*
 * expose the CELL x,y 
 */
void expose(x,y)
int x,y;
{

  World(x,y,status) = EXPOSED;
  switch(World(x,y,num_bombs_around))
  {
    case 0:
      SETBITMAP(x,y,b0);
      if(do_expose_zeros)
        expose_zeros(x,y);
      break;

    case 1:
        SETCBITMAP(x,y,b1,cgc[0]);
      break;

    case 2:
      SETCBITMAP(x,y,b2,cgc[1]);
      break;

    case 3:
      SETCBITMAP(x,y,b3,cgc[2]);
      break;

    case 4:
      SETCBITMAP(x,y,b4,cgc[3]);
      break;

    case 5:
      SETCBITMAP(x,y,b5,cgc[4]);
      break;

    case 6:
      SETCBITMAP(x,y,b6,cgc[5]);
      break;

    case 7:
      SETCBITMAP(x,y,b7,cgc[6]);
      break;

    case 8:
      SETCBITMAP(x,y,b8,cgc[7]);
      break;


  }
}


void show_mines_left()
{
char string[20];

  sprintf(string,"Mines Left: %d",mines_left);
  XtVaSetValues(countw,XtNlabel,(XtArgVal)string,NULL);

}

/* 
 * the user exposed a mine - expose all squares.
 */
void die()
{
register int x,y;
int missed=FALSE;

  Stop_Clock();
  /* set flag not to do expose_zeros() */
  do_expose_zeros = FALSE;
  dead = TRUE;

    for(x=0; x < WIDTH; x++)
      for(y=0; y < HEIGHT; y++)
      {
        if(World(x,y,status) == MARKED && !World(x,y,is_bomb)) {
          SETBITMAP(x,y,wrongmarked); 
          missed = TRUE;
        }
        else if(World(x,y,status) == MARKED)
               SETBITMAP(x,y,marked); 
        
        else if(World(x,y,is_bomb) && World(x,y,clicked_on)) {
               World(x,y,status) = EXPOSED;
               INVSETBITMAP(x,y,bomb);
        }


        else if(World(x,y,is_bomb) ) {
               World(x,y,status) = EXPOSED;
               SETBITMAP(x,y,bomb);
        }

        else if(World(x,y,status) == UNKNOWN)
                expose(x,y);
       
      }

   if(missed) { 
     XtVaSetValues(statuslw,XtNlabel,(XtArgVal) MISSED_SOME ,NULL);
     if(sound == True) make_noise();
   }
   else if(mines_left > 0) {
     XtVaSetValues(statuslw,XtNlabel, (XtArgVal) YOU_DIED ,NULL);
     if(sound == True) make_noise();
   }
   else {
     XtVaSetValues(statuslw,XtNlabel,(XtArgVal) YOU_WON ,NULL);
     if(sound == True) make_other_noise();
   }

  /* reset flag to do expose_zeros() */
  do_expose_zeros = TRUE;
}

/* make some noise ;) */
void make_noise()
{
XKeyboardState kbs;
XKeyboardControl *kb; /* hack cause some dickhead decided to 
                         use different types */
int x,f,of, op,od;

  kb = (XKeyboardControl *)&kbs;
  XGetKeyboardControl(disp,&kbs);
  od = kbs.bell_duration;
  op = kbs.bell_percent;
  of = kbs.bell_pitch;

  kbs.bell_percent = 100;
  kbs.bell_duration = 10;
  XChangeKeyboardControl(disp, KBBellPercent|KBBellDuration, kb);

  for(f = 2000 ; f > 10; f -= 20) {

    kbs.bell_pitch = f;

    XChangeKeyboardControl(disp, KBBellPitch, kb);
    XBell(disp,100);
    XFlush(disp);
  }


  kbs.bell_duration = od;
  kbs.bell_percent = op;
  kbs.bell_pitch = of;
  XChangeKeyboardControl(disp, KBBellPercent|KBBellPitch|KBBellDuration, kb);
  XFlush(disp);

}

void make_other_noise()
{
XKeyboardState kbs;
XKeyboardControl *kb; /* hack cause some dickhead decided to 
                         use different types */
int x,f,of, op,od;

  kb = (XKeyboardControl *)&kbs;
  XGetKeyboardControl(disp,&kbs);
  od = kbs.bell_duration;
  op = kbs.bell_percent;
  of = kbs.bell_pitch;

  kbs.bell_percent = 100;
  kbs.bell_duration = 10;
  XChangeKeyboardControl(disp, KBBellPercent|KBBellDuration, kb);

for(x = 0; x < 5; x++)
  for(f = 1000 ; f > 10; f -= 100) {

    kbs.bell_pitch = f;

    XChangeKeyboardControl(disp, KBBellPitch, kb);
    XBell(disp,100);
    XFlush(disp);
  }

  kbs.bell_duration = od;
  kbs.bell_percent = op;
  kbs.bell_pitch = of;
  XChangeKeyboardControl(disp, KBBellPercent|KBBellPitch|KBBellDuration, kb);
  XFlush(disp);
}
