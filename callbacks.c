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


#include "xminesweep.h"

extern void expose();
extern void show_mines_left();
extern void die();

void Shift_B1_Event();
void Start_Clock();
void Stop_Clock();
void Do_Clock();

/*
 * changed: bug identified by George Phillips <phillips@cs.ubc.ca>
 *          bug fix by George Phillips <phillips@cs.ubc.ca>.
 */
void get_pos(ex,ey,x,y)
int ex,ey,*x,*y;
{
  ex /= 21;
  if (ex < 0) ex = 0;
  if (ex >= WIDTH) ex = WIDTH - 1;

  ey /= 21;
  if (ey < 0) ey = 0;
  if (ey >= HEIGHT) ey = HEIGHT - 1;

  *x = ex;
  *y = ey;
}

/* expose a CELL */
void B1_Event(w,event,params,numparams)
Widget w;
XButtonEvent *event;
String *params;
Cardinal *numparams;
{
int x,y;

  if (!timer_started) {
    Start_Clock();
    timer_started = TRUE;
    XtVaSetValues(statuslw,XtNlabel,(XtArgVal) PLAYING ,NULL);
  }

  get_pos(event->x,event->y,&x,&y);
  if(World(x,y,status) != EXPOSED) {
    if(World(x,y,status) != MARKED) {
      if(World(x,y,is_bomb)) {
        World(x,y,clicked_on) = TRUE;
        INVSETBITMAP(x,y,bomb);
        die();
      }
      else
        expose(x,y);
    } 
    else XBell(XtDisplay(w),100);
  } else Shift_B1_Event(w,event,params,numparams); 
XFlush(disp); 
}

/* expose surounding cells */
void Shift_B1_Event(w,event,params,numparams)
Widget w;
XButtonEvent *event;
String *params;
Cardinal *numparams;
{
int x,y;
int left_ok,right_ok,up_ok,down_ok;
char count;

  if (!timer_started) {
    Start_Clock();
    timer_started = TRUE;
    XtVaSetValues(statuslw,XtNlabel,(XtArgVal) PLAYING ,NULL);
  }

  get_pos(event->x,event->y,&x,&y);

  if(World(x,y,status) == EXPOSED) {
    /* count the marked squares around it */
    count = 0;
    left_ok = (x-1 >=0) ? TRUE:FALSE;
    right_ok = (x+1 < WIDTH) ? TRUE:FALSE;
    up_ok = (y-1 >=0) ? TRUE:FALSE;
    down_ok = (y+1 < HEIGHT) ? TRUE:FALSE;

    if(left_ok) 
      if(World(x-1,y,status) == MARKED) count++;  /* left */

    if(right_ok)
      if(World(x+1,y,status) == MARKED) count++;  /* right */

    if(up_ok)
      if(World(x,y-1,status) == MARKED) count++;  /* up */

    if(down_ok)
      if(World(x,y+1,status) == MARKED) count++;  /* down */

    if(left_ok && up_ok)  
      if(World(x-1,y-1,status) == MARKED) count++;  /* up left */
        
    if(left_ok && down_ok)
      if(World(x-1,y+1,status) == MARKED) count++;  /* down left */

    if(right_ok && up_ok)
      if(World(x+1,y-1,status) == MARKED) count++;  /* up right */

    if(right_ok &&down_ok)
      if(World(x+1,y+1,status) == MARKED) count++;  /* down right */

    if(count < World(x,y,num_bombs_around)) {
      /* not all the mines have been marked */
      XBell(XtDisplay(w),100);
    } else {
      /* expose the unmarked squares - even if there is a mine there */
      if(left_ok) 
        if(World(x-1,y,status) != MARKED) /* left */
          if(World(x-1,y,is_bomb)) {
            World(x-1,y,clicked_on) = TRUE;
            INVSETBITMAP(x-1,y,bomb);
            die();
          } else expose(x-1,y);


      if(right_ok)
        if(World(x+1,y,status) != MARKED)  /* right */
          if(World(x+1,y,is_bomb)) {
            World(x+1,y,clicked_on) = TRUE;
            INVSETBITMAP(x+1,y,bomb);
            die();
          } else expose(x+1,y);


      if(up_ok)
        if(World(x,y-1,status) != MARKED)  /* up */
          if(World(x,y-1,is_bomb)) {
            World(x,y-1,clicked_on) = TRUE;
            INVSETBITMAP(x,y-1,bomb);
            die();
          } else expose(x,y-1);


      if(down_ok)
        if(World(x,y+1,status) != MARKED)  /* down */
          if(World(x,y+1,is_bomb)) {
            World(x,y+1,clicked_on) = TRUE;
            INVSETBITMAP(x,y+1,bomb);
            die();
          } else expose(x,y+1);


      if(left_ok && up_ok)  
        if(World(x-1,y-1,status) != MARKED)  /* up left */
          if(World(x-1,y-1,is_bomb)) {
            World(x-1,y-1,clicked_on) = TRUE;
            INVSETBITMAP(x-1,y-1,bomb);
            die();
          } else expose(x-1,y-1);

        
      if(left_ok && down_ok)
        if(World(x-1,y+1,status) != MARKED)  /* down left */
          if(World(x-1,y+1,is_bomb)) {
            World(x-1,y+1,clicked_on) = TRUE;
            INVSETBITMAP(x-1,y+1,bomb);
            die();
          } else expose(x-1,y+1);


      if(right_ok && up_ok)
        if(World(x+1,y-1,status) != MARKED)  /* up right */
          if(World(x+1,y-1,is_bomb)) {
            World(x+1,y-1,clicked_on) = TRUE;
            INVSETBITMAP(x+1,y-1,bomb);
            die();
          } else expose(x+1,y-1);


      if(right_ok &&down_ok)
        if(World(x+1,y+1,status) != MARKED)  /* down right */
          if(World(x+1,y+1,is_bomb)) {
            World(x+1,y+1,clicked_on) = TRUE;
            INVSETBITMAP(x+1,y+1,bomb);
            die();
          } else expose(x+1,y+1);

 
    }
  } else XBell(XtDisplay(w),100);
XFlush(disp);
}

/* mark / unmark as a question */
void B2_Event(w,event,params,numparams)
Widget w;
XButtonEvent *event;
String *params;
Cardinal *numparams;
{
int x,y;

  if (!timer_started) {
    Start_Clock();
    timer_started = TRUE;
    XtVaSetValues(statuslw,XtNlabel,(XtArgVal) PLAYING ,NULL);
  }

  if(!dead) {
    get_pos(event->x,event->y,&x,&y);
    if(World(x,y,status) != EXPOSED && World(x,y,status) != MARKED) {
        if(World(x,y,status) == QUESTION) { /* un mark as a question */
          World(x,y,status) = UNKNOWN;
          SETBITMAP(x,y,unknown);
        } else {
          World(x,y,status) = QUESTION;
          SETBITMAP(x,y,question);
        }
    } else XBell(XtDisplay(w),100);
  }
XFlush(disp);
}

/* mark / unmark a CELL */
void B3_Event(w,event,params,numparams)
Widget w;
XButtonEvent *event;
String *params;
Cardinal *numparams;
{
int x,y;

  if (!timer_started) {
    Start_Clock();
    timer_started = TRUE;
    XtVaSetValues(statuslw,XtNlabel,(XtArgVal) PLAYING ,NULL);
  }

  if(!dead) {
    get_pos(event->x,event->y,&x,&y);
    if(World(x,y,status) != EXPOSED) {
      if(World(x,y,status) == MARKED) { /* reset (unmark) the CELL */
        World(x,y,status) = UNKNOWN; 
        SETBITMAP(x,y,unknown);
        mines_left++;
        show_mines_left();
      } else {                           /* set marked */
        World(x,y,status) = MARKED;
        SETBITMAP(x,y,marked);
        mines_left--;
        show_mines_left();
      } 
      if (mines_left == 0) die();
    } else XBell(XtDisplay(w),100);
  } else
    XBell(XtDisplay(w),100);

XFlush(disp);
}

/*
 * handle an expose event 
 */
void Expose_Event(w,event,params,numparams)
Widget w;
XExposeEvent *event;
String *params;
Cardinal *numparams;
{
register int x,y;

  if(event->count != 0) return;

  do_expose_zeros = FALSE;
  for(x=0; x < WIDTH; x++)
    for(y=0; y < HEIGHT; y++)
    {
      if(World(x,y,status) == MARKED  && dead && !World(x,y,is_bomb))
        SETBITMAP(x,y,wrongmarked);

      else if(World(x,y,status) == MARKED)
        SETBITMAP(x,y,marked);

      else if(World(x,y,status) == EXPOSED &&  World(x,y,is_bomb)
              && World(x,y,clicked_on))
        INVSETBITMAP(x,y,bomb);

      else if(World(x,y,status) == EXPOSED &&  World(x,y,is_bomb))
        SETBITMAP(x,y,bomb);

      else if(World(x,y,status) == EXPOSED)
        expose(x,y);

      else if(World(x,y,status) == QUESTION)
        SETBITMAP(x,y,question);

      else
        SETBITMAP(x,y,unknown);

    }
  do_expose_zeros = TRUE;
  XFlush(disp);
}

/*
** This function is called by the widgets and Windowmanager to quit the
** program
*/
void DoQuit(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  XFreePixmap(XtDisplay(toplevel),unknown);
  XFreePixmap(XtDisplay(toplevel),marked);
  XFreePixmap(XtDisplay(toplevel),bomb);
  XFreePixmap(XtDisplay(toplevel),wrongmarked);
  XFreePixmap(XtDisplay(toplevel),question);
  XFreePixmap(XtDisplay(toplevel),b0);
  XFreePixmap(XtDisplay(toplevel),b1);
  XFreePixmap(XtDisplay(toplevel),b2);
  XFreePixmap(XtDisplay(toplevel),b3);
  XFreePixmap(XtDisplay(toplevel),b4);
  XFreePixmap(XtDisplay(toplevel),b5);
  XFreePixmap(XtDisplay(toplevel),b6);
  XFreePixmap(XtDisplay(toplevel),b7);
  XFreePixmap(XtDisplay(toplevel),b8);
  free(world);
  exit(0);
}

/*
 * called to restart the game 
*/
void DoRestart(w,client_data,call_data )
Widget w;
XtPointer client_data,call_data;
{
  dead = FALSE;
  init_world();
  mines_left = NUM_MINES;
  show_mines_left();
  Crnt_Time = 0;

  if(timer_started) {
    Stop_Clock();
    timer_started = FALSE;
  }

  XtVaSetValues(timelw,XtNlabel,(XtArgVal)"Time: 00000",NULL);
  XtVaSetValues(statuslw,XtNlabel,(XtArgVal) READY ,NULL);
}

/*
 * called when the value in text widget changes
 */
void DoAjustMines(w,client_data,call_data )
Widget w;
XtPointer client_data,call_data;
{
int temp;

  sscanf(num_mines_string,"%d",&temp);

  if((temp >= (WIDTH*HEIGHT)/3) || temp < 1) {
    sprintf(num_mines_string,"%d",NUM_MINES); 
    XtVaSetValues(w,XtNstring,(XtArgVal)num_mines_string,NULL);
  } else 
      if(temp != NUM_MINES) {
         NUM_MINES = temp;
         /* do a restart */
         DoRestart(w, client_data,call_data );
      }
}


void Start_Clock()
{
  timer = XtAppAddTimeOut(appcon,(unsigned long)1000,Do_Clock,NULL);
}

void Stop_Clock()
{
  XtRemoveTimeOut(timer);
}


/* 
 * called when the timer finishes 
 */

void Do_Clock(client_data,call_data )
XtPointer client_data;
XtIntervalId *call_data;
{
char str[11];

  Crnt_Time++;

  sprintf(str,"Time: %.5d",Crnt_Time);

  XtVaSetValues(timelw,XtNlabel,(XtArgVal)str,NULL);  

  timer = XtAppAddTimeOut(appcon,(unsigned long)1000,Do_Clock,NULL);
}

