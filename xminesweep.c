char copyright[]=
   "xminesweep Version 3.0  (c) Copyright Ashley Roll, all rights reserved.";
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


#define MAIN
#include "xminesweep.h"
#include "patchlevel.h"
#include "bitmaps/icon.xbm"

#include <stdlib.h>

/* functions in callbacks.c */
extern void DoQuit();
extern void DoRestart();
extern void Expose_Event();
extern void B1_Event();
extern void Shift_B1_Event();
extern void B2_Event();
extern void B3_Event();
extern void DoAjustMines();

/* functions in misc.c */
extern Widget MakeMenu();
extern Widget AddMenuItem();
extern void init_bitmaps();
extern void init_world();
extern void create_buttons();
extern void show_mines_left();
extern void Start_Clock();
extern void Stop_Clock();
extern void expose();

#ifdef CHEAT_OK
void Cheat();
#endif


/* -------------------------- */
static Atom  wm_delete_window;      /* insure that delete window works */

static XtActionsRec  actions[] = {  /* declare possible actions */
/* action_name, routine */
  { "quit",   DoQuit  },
  { "AjustMines", DoAjustMines },
  { "Button1_Event", B1_Event },
  { "ShiftButton1_Event", Shift_B1_Event },
  { "Button2_Event", B2_Event },
  { "Button3_Event", B3_Event },
#ifdef CHEAT_OK
  { "Cheat_Toggle", Cheat },
#endif
  { "ExposeEvent", Expose_Event },
};

static char text_trans[] =
  "<Key>Return:  AjustMines() \n\
   Ctrl<Key>M:   AjustMines() ";

#ifdef CHEAT_OK
static char Translations[] =
  "Shift <Btn1Down>:  ShiftButton1_Event() \n\
   <Btn1Down>:  Button1_Event() \n\
   <Btn2Down>:  Button2_Event() \n\
   <Btn3Down>:  Button3_Event() \n\
   Ctrl Shift<Key>(2)C:  Cheat_Toggle() \n\
   <Expose>:    ExposeEvent()";
#else
static char Translations[] =
  "<Btn1Down>:  Button1_Event() \n\
   <Btn2Down>:  Button2_Event() \n\
   <Btn3Down>:  Button3_Event() \n\
   <Expose>:    ExposeEvent()";
#endif

/***************/
/* fallback resources */
static char *fall_back[] = {
   ".xminesweep.iconName: xminesweep",
   NULL
};

/* command line options to parse */
static XrmOptionDescRec table[] = {
    {"-mines",  "*mines", XrmoptionSepArg, NULL},
    {"-ncH",  "*numCellsHigh", XrmoptionSepArg, NULL},
    {"-ncW",  "*numCellsWide", XrmoptionSepArg, NULL},
    {"-sound", "*sound", XrmoptionNoArg, (XPointer)"True"},
    {"-nosound", "*sound", XrmoptionNoArg, (XPointer)"False"},
};

#define OFFSETOF(st,el) (size_t)(&(((st *)0)->el))

/* resources */
static XtResource resources[] = {
    {"mines", "Mines", XtRInt, sizeof(int),
         (Cardinal) OFFSETOF(Config,numMines), XtRImmediate,
	 (caddr_t) DEFAULT_NUM_MINES},
    {"numCellsHigh", "NumCellsHigh", XtRInt, sizeof(int),
         (Cardinal) OFFSETOF(Config,height), XtRImmediate,
	 (caddr_t) DEF_HEIGHT},
    {"numCellsWide", "NumCellsWide", XtRInt, sizeof(int),
         (Cardinal) OFFSETOF(Config,width), XtRImmediate,
	 (caddr_t) DEF_WIDTH},
    {"sound", "Sound", XtRBoolean, sizeof(Boolean),
	 (Cardinal) OFFSETOF(Config,sound), XtRImmediate,
	 (caddr_t) False},
};


/* colors */
static char *colors[] = {"blue","magenta","red","green","cyan","black","grey50","brown"};
/* yellow is too little contrast to background, use grey50 instead */

/* -------------------------- */


main(argc, argv)
  int argc;
  char **argv;
{
int t;
Pixmap icon;
char title[40];

  do_expose_zeros = TRUE;
  dead = FALSE;
  Crnt_Time = 0;
  timer_started = FALSE;

  sprintf(title,"XMinesweep Ver %s.%s",VERSION,PATCHLEVEL);

  toplevel = XtVaAppInitialize(
                 &appcon, "XMinesweep",    /* app context, ClassName */
                 table, XtNumber(table),   /* app command line options */
                 &argc, argv,              /* command line */
                 fall_back,                /* Fall back resources */
                 XtNtitle,(XtArgVal)title, /* the window title */

                 XtNresize,(XtArgVal)False, /* no resizing the window */
                 NULL);                    /* End Va resource list */
  XtAppAddActions(appcon, actions, XtNumber(actions));

  icon = XCreateBitmapFromData(XtDisplay(toplevel),
                               DefaultRootWindow(XtDisplay(toplevel)),
                               icon_bits, icon_width, icon_height);

 XtVaSetValues(toplevel,XtNiconPixmap, icon,NULL);

  /* get the resourse values from the database */
      XtGetApplicationResources(toplevel, (caddr_t) &config,
                              resources, XtNumber(resources),
                              NULL, (Cardinal) 0);


  if((NUM_MINES >= (WIDTH*HEIGHT)/3) || NUM_MINES < 1)
    NUM_MINES = DEFAULT_NUM_MINES;

  mines_left = NUM_MINES;

  /* allocate space for the world 'array' */
  world = (CELL *) malloc((WIDTH+1)*(HEIGHT+1)*sizeof(CELL));

/* create a paned widget to put everything into */
  mainpw  = XtVaCreateManagedWidget("main",panedWidgetClass,toplevel,
            NULL);

{
Widget button,pw;
  pw = XtVaCreateManagedWidget("box1",panedWidgetClass,mainpw,
                  XtNshowGrip,(XtArgVal)False,
                  XtNorientation,(XtArgVal)XtorientHorizontal,
                  NULL);
/* create the Buttons */
  button = XtVaCreateManagedWidget("quit",commandWidgetClass,pw,
                  XtNlabel,(XtArgVal)"Quit",
                  XtNshowGrip,(XtArgVal)False,NULL);
  XtAddCallback(button,"callback",DoQuit,NULL);

  button = XtVaCreateManagedWidget("restart",commandWidgetClass,pw,
                XtNlabel,(XtArgVal)"Restart",
                XtNshowGrip,(XtArgVal)False,NULL);
  XtAddCallback(button,"callback",DoRestart,NULL);

  timelw =  XtVaCreateManagedWidget("time",labelWidgetClass,pw,
                XtNlabel,(XtArgVal)"Time: 00000",
                XtNshowGrip,(XtArgVal)False,NULL);

  statuslw = XtVaCreateManagedWidget("status",labelWidgetClass,pw,
                XtNlabel,(XtArgVal)"Ready - Click to begin",
                XtNshowGrip,(XtArgVal)False,NULL);

}

{
Widget pw;

  pw = XtVaCreateManagedWidget("box2",panedWidgetClass,mainpw,
                  XtNshowGrip,(XtArgVal)False,
                  XtNorientation,(XtArgVal)XtorientHorizontal,
                  NULL);

/* label widget to show how many bombs to go */

  countw = XtVaCreateManagedWidget("counter",labelWidgetClass,pw,
              XtNshowGrip,(XtArgVal)False,
              XtNallowResize,(XtArgVal)True,
              XtNskipAdjust,(XtArgVal)True,NULL);

  (void) XtVaCreateManagedWidget("textlabel",labelWidgetClass,pw,
              XtNshowGrip,(XtArgVal)False,
              XtNlabel,(XtArgVal)"Number of Mines",
              XtNskipAdjust,(XtArgVal)True,NULL);

  sprintf(num_mines_string,"%d",NUM_MINES);
  numbombtw = XtVaCreateManagedWidget("numbombtw",asciiTextWidgetClass,pw,
              XtNshowGrip,(XtArgVal)False,
              XtNeditType,(XtArgVal)XawtextEdit,
              XtNuseStringInPlace,(XtArgVal)True,
              XtNstring,(XtArgVal)num_mines_string,
              XtNlength,(XtArgVal)4,
              XtNskipAdjust,(XtArgVal)False,NULL);

  XtOverrideTranslations(numbombtw, XtParseTranslationTable(text_trans));

}
/* create the form widget to put the buttons in */
  corew = XtVaCreateManagedWidget("buttons",coreWidgetClass,mainpw,
              XtNshowGrip,(XtArgVal)False,
              XtNwidth,(XtArgVal) (WIDTH * 21) - 1,
              XtNheight,(XtArgVal) (HEIGHT * 21) - 1,
              XtNskipAdjust,(XtArgVal)True,NULL);
  XtOverrideTranslations(corew, XtParseTranslationTable(Translations));

/* create the bitmaps */
  init_bitmaps();
  create_buttons();
  XtRealizeWidget(toplevel);
  disp = XtDisplay(toplevel);

  /* setup the GC */
  corewwin = XtWindow(corew);
  {
  XGCValues gcv;
  int mask;

    gcv.foreground = BlackPixelOfScreen(XtScreen(toplevel));
    gcv.background = WhitePixelOfScreen(XtScreen(toplevel));
    mask = GCForeground | GCBackground;
    gc = XCreateGC(disp,DefaultRootWindow(XtDisplay(toplevel)),mask,&gcv);

    gcv.foreground = WhitePixelOfScreen(XtScreen(toplevel));
    gcv.background = BlackPixelOfScreen(XtScreen(toplevel));
    igc = XCreateGC(disp,DefaultRootWindow(XtDisplay(toplevel)),mask,&gcv);

    /* try colors for the num boards */
  switch (DefaultVisualOfScreen(XtScreen(toplevel))->class)
  {
	  int loop;
	  XColor def, get;
  case StaticGray:
  case GrayScale:
	  cgc[0] = cgc[1] = cgc[2] = cgc[3] = cgc[4] = cgc[5] = cgc[6] = cgc[7] = gc;
	  break;
  default:
	  for (loop=0; loop < 8; loop++)
	  {
		  XLookupColor(disp,DefaultColormapOfScreen(XtScreen(toplevel)),colors[loop],&def,&get);
		  if(XAllocColor(disp,DefaultColormapOfScreen(XtScreen(toplevel)), &get))
			  gcv.foreground = get.pixel;
		  else
			  gcv.foreground = BlackPixelOfScreen(XtScreen(toplevel));
		  gcv.background = WhitePixelOfScreen(XtScreen(toplevel));
		  cgc[loop] = XCreateGC(disp,DefaultRootWindow(XtDisplay(toplevel)),mask,&gcv);
	  }
  }}

  init_world();
  show_mines_left();


  /* Set the window to call quit() action if `deleted' */
  XtOverrideTranslations(toplevel,
          XtParseTranslationTable("<Message>WM_PROTOCOLS:quit()") );
  wm_delete_window = XInternAtom(XtDisplay(toplevel),"WM_DELETE_WINDOW",False);
  (void) XSetWMProtocols(XtDisplay(toplevel), XtWindow(toplevel),
                                                 &wm_delete_window, 1);

  XtAppMainLoop(appcon);
exit(0);
}

#ifdef CHEAT_OK
void Cheat()
{
int got_one,w,h;

  /* find an unexposed square with zero mines around it */
  got_one = w = h = 0;
  for (w = 0; w < WIDTH  && !got_one; w++)
    for (h = 0; h < HEIGHT && !got_one; h++)
      if(World(w,h,status) == UNKNOWN && World(w,h,num_bombs_around) == 0
         && !World(w,h,is_bomb)) {
        expose(w,h);
        got_one = 1;
      }
}
#endif

