/* $Id$
*/


#include "plplot/plDevs.h"

#if defined(GRX_DO_TIFF) || defined(GRX_DO_BMP) || defined(GRX_DO_JPEG) || defined (PLD_gnusvga)

#include "plplot/plplotP.h"
#include "plplot/drivers.h"
#include "plplot/plevent.h"

#include <grx20.h>

#ifndef bzero   /* not stanrard ANSI (boo hoo) */
#define bzero(a,b) memset(a,0,b)
#endif

#ifdef PLD_gnusvga
#undef _POSIX_SOURCE    /* Turn off POSIX to get around "problem" with <keys.h> header. 
                           Ugly, but it works *sigh* */
#include <keys.h>
#endif

#include <time.h>       /* for the 'newname' funtion */
#include <strings.h>


/* Prototypes:	Since GNU CC, we can rest in the safety of ANSI prototyping. */

#ifdef PLD_gnusvga

static void	plpause		(PLStream *);
static void	svga_text	(PLStream *);
static void	svga_graph	(PLStream *);
static void	WaitForPage	(PLStream *pls);
static void	EventHandler	(PLStream *pls, GrMouseEvent *event);
static void     TranslateEvent  (PLStream *pls, GrMouseEvent *event, PLGraphicsIn *gin);

#endif

static void	fill_polygon	(PLStream *pls);
static void	setcmap		(PLStream *pls);
static void     plD_init_gnu_grx_dev(PLStream *pls);
static void     XorMod          (PLStream *pls, PLINT *mod);
 
#if defined(GRX_DO_TIFF) || defined(GRX_DO_BMP) || defined(GRX_DO_JPEG) || defined(PLD_tiff) || defined(PLD_jpeg) || defined(PLD_bmp)
#if GRX_VERSION_API >= 0x0229 
char            *newname        (char *ext);
#endif
#endif

/* gmf; should probably query this on start up... Maybe later. */

static exit_eventloop = 0;

#define CLEAN 0
#define DIRTY 1

static page_state;

/* 
 *  The "device structure" for the gnusvga driver holding all sorts of stuff
 *  that once would have been "global".
 */

typedef struct {

#ifdef PLD_gnusvga
       
        PLGraphicsIn gin;                       /* Graphics input structure     */
        GrMouseEvent mevent;                    /* mouse event handler          */
        GrLineOption gnusvgaline;               /* Line Options                 */
        GrContext *switch_screen;               /* A copy of the GRX screen in case of switching */
        GrContext *double_buffer;               /* Screen pointer for double buffering  */
        GrContext *visual_screen;               /* Screen pointer for visual screen  */
        GrContext *top_line;                    /* Screen pointer for top line  */

/*
 *  Originally I didn't realise it was possible to "XOR" a line, so I
 *  blitted the cross hair in. Then by accident (I read the instructions, 
 *  for something else I wanted to do, and stumbled across it !),
 *  I found out how to XOR a line, so got rid of the blitting. I have kept
 *  it in case it's quicker. It can be put back in a jiff by defining
 *  "BLIT_CROSSHAIR".
 */

#ifdef BLIT_CROSSHAIR   
        GrContext *Xhair_X;                    /* Screen pointer for XhairX line  */
        GrContext *Xhair_Y;                    /* Screen pointer for XhairY line  */
#endif

#endif
        GrVideoDriver *Old_Driver_Vector;       /* Pointer for old driver      */
 
        PLINT vgax;               
        PLINT vgay;

        int colour;                             /* Current Colour               */
        int totcol;                             /* Total number of colours      */
        int fg;                                 /* Pointer (in colour index) to a "safe" foreground colour */

        int last_x;                             /* Used to "wipe out" the X-Hair */
        int last_y;                             /* Used to "wipe out" the X-Hair */
        int locate_mode;                        /* Set while in locate mode     */
        int draw_mode;                          /* used by XOR mode                 */

        char dont_copy_screen;                  /* set to non-zero before existing */
        char Xhair_on;                          /* set if the cross hair is on and displayed */
        char toggle_xhair;                      /* set if the cross hair is being "toggled" off for fixing top of screen */
        
             } gnu_grx_Dev;

#ifdef PLD_gnusvga

/* More Prototypes (dependent on  "gnu_grx_Dev") */

static void     init_double_buffer      ( gnu_grx_Dev *dev );
static void     CreateXhair 	        ( gnu_grx_Dev *dev, GrMouseEvent *event );
static void     DestroyXhair            ( gnu_grx_Dev *dev );
static void     SaveTopOfScreen         ( gnu_grx_Dev *dev );
static void     RestoreTopOfScreen      ( gnu_grx_Dev *dev );

#endif

        
/*--------------------------------------------------------------------------*\
 * plD_init_gnu_grx_dev()
 *
\*--------------------------------------------------------------------------*/

static void
plD_init_gnu_grx_dev(PLStream *pls)
{
    gnu_grx_Dev *dev;

/* Allocate and initialize device-specific data */

    if (pls->dev != NULL)
	plwarn("plD_init_gnu_grx_dev: \"gnu_grx_Dev\" device pointer is already set");

    pls->dev = calloc(1, (size_t) sizeof(gnu_grx_Dev));
    if (pls->dev == NULL)
	plexit("plD_init_gnu_grx_dev: Out of memory.");

    bzero(pls->dev,sizeof(gnu_grx_Dev)); /* I'm lazy - quick way of setting everything to 0 */

    dev = (gnu_grx_Dev *) pls->dev;

    if ( (pls->dev_compression<=0)||(pls->dev_compression>99) )
       pls->dev_compression=90;
    
    dev->colour=1;
    dev->totcol=16;
    dev->vgax = 1023;
    dev->vgay = 767;

}

#ifdef PLD_gnusvga

/*----------------------------------------------------------------------*\
 * init_double_buffer()
 *
 * Initialize the second buffer for double buffering
\*----------------------------------------------------------------------*/

static void init_double_buffer ( gnu_grx_Dev *dev )
{
  if ( (dev->double_buffer==NULL)&&(dev->visual_screen==NULL) ) 
     {
      dev->visual_screen=GrSaveContext(NULL);
      dev->double_buffer=GrCreateContext(GrScreenX(), GrScreenY(),NULL,NULL);
      GrSetContext(dev->double_buffer);
      GrClearContext(0);
     }
  else
     {
      plexit("An undetermined error happened allocating the resources for double buffering.");
     }

}

/*----------------------------------------------------------------------*\
 * plD_init_vga()
 *
 * Initialize device.
\*----------------------------------------------------------------------*/

void
plD_init_vga(PLStream *pls)
{
    gnu_grx_Dev *dev;

 
    pls->termin = 1;		/* is an interactive terminal */
    pls->icol0 = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->graphx = TEXT_MODE;
    pls->dev_fill0 = 1;

    if (!pls->colorset)
	pls->color = 1;

    if (pls->dev==NULL)
       plD_init_gnu_grx_dev(pls);

    dev=(gnu_grx_Dev *)pls->dev;

/* Set up device parameters */

    svga_graph(pls);		/* Can't get current device info unless in
				   graphics mode. */

    dev->vgax = GrSizeX() - 1;	/* should I use -1 or not??? */
    dev->vgay = GrSizeY() - 1;

    if (pls->xdpi==0) 
       {
        plP_setpxl(2.5, 2.5);	/* My best guess.  Seems to work okay. */
       }
    else
       {
        pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
        plP_setpxl(pls->xdpi/25.4,pls->ydpi/25.4); /* Convert DPI to pixels/mm */
       } 
 
    plP_setphy(0, dev->vgax, 0, dev->vgay);

    dev->gnusvgaline.lno_width=pls->width;
    dev->gnusvgaline.lno_pattlen=0;

/* Check and set up for Double buffering */    
    
    if (pls->db!=0) 
       init_double_buffer(dev);

}

#endif

/*----------------------------------------------------------------------*\
 * plD_line_vga()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void
plD_line_vga(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
    int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;
    y1 = dev->vgay - y1;
    y2 = dev->vgay - y2;

   dev->gnusvgaline.lno_color=dev->colour|dev->draw_mode; /* dev->draw_mode for "XOR" */
   GrCustomLine(x1,y1,x2,y2,&dev->gnusvgaline);

    page_state = DIRTY;
}

/*----------------------------------------------------------------------*\
 * plD_polyline_vga()
 *
 * Draw a polyline in the current color.
\*----------------------------------------------------------------------*/

void
plD_polyline_vga(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_vga(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}


/*----------------------------------------------------------------------*\
 * fill_polygon()
 *
 * Fill polygon described in points pls->dev_x[] and pls->dev_y[].
\*----------------------------------------------------------------------*/

static void
fill_polygon(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    int i;
    int points[PL_MAXPOLY][2];

    if (pls->dev_npts < 1)
	return;

/* Select the fill pattern */

/* Specify boundary */

    for (i = 0; i < pls->dev_npts; i++) {
	points[i][0] = pls->dev_x[i];
        points[i][1] = dev->vgay - pls->dev_y[i];
    }
    GrFilledPolygon(pls->dev_npts, points, dev->colour);

}

/*----------------------------------------------------------------------*\
 * setcmap()
 *
 * Sets up color palette.
\*----------------------------------------------------------------------*/

static void
setcmap(PLStream *pls)
{
    int i, ncol1, ncol0;
    PLColor cmap1col;
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (GrNumColors()<pls->ncol0)
       ncol0=GrNumColors();
    else
       ncol0=pls->ncol0;

/* Initialize cmap 0 colors */
       
    for (i = 0; i < ncol0; i++)
        {
        GrAllocCell();
	GrSetColor(i, pls->cmap0[i].r, pls->cmap0[i].g, pls->cmap0[i].b);
        }
/* 
 * Do a kludge to add a "black" colour back to the palette if the
 * background isn't black (ie changed through -bg), and at the same time,
 * fix the mouse up colours so they will work with "non-black" 
 * backgrounds.
 * 
 * Also includes an "optional" change to swap the red colour with the 
 * black colour, which is on by default. (I don't like the red being
 * the 'default' colour "1" on a "white" background, or for that matter
 * yellow being "2", but I can live more with yellow at number two.)
 * Just use "-hack" from the command line to make it take effect.
 *
 */

if ((pls->cmap0[0].r>227)&&(pls->cmap0[0].g>227)&&(pls->cmap0[0].b>227))
   {
    if (pls->hack!=1)
       {
       GrSetColor(15, 0, 0, 0);
#ifdef PLD_gnusvga
    if (pls->termin==1)
       GrMouseSetColors(0,15);
#endif
       dev->fg=15;
       }
    else
       {
        GrSetColor(15, pls->cmap0[1].r, pls->cmap0[1].g, pls->cmap0[1].b);
        GrSetColor(1, 0, 0, 0);
#ifdef PLD_gnusvga
        if (pls->termin==1)
           GrMouseSetColors(0,1);
#endif
        dev->fg=1;
      }
   }
else
   {
#ifdef PLD_gnusvga
    if (pls->termin==1)
       GrMouseSetColors(15,0);
#endif
   dev->fg=15;
   }

/* Initialize any remaining slots for cmap1 */

    ncol1 = GrNumFreeColors();
    for (i = 0; i < ncol1; i++) {
	plcol_interp(pls, &cmap1col, i, ncol1);
	GrAllocCell();
	GrSetColor(i + pls->ncol0, cmap1col.r, cmap1col.g, cmap1col.b);
    }


}

/*--------------------------------------------------------------------------*\
 * XorMod()
 *
 * Enter xor mode ( mod != 0) or leave it ( mode = 0)
\*--------------------------------------------------------------------------*/

static void
XorMod(PLStream *pls, PLINT *mod)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (*mod == 0)
      dev->draw_mode=GrWRITE;
    else
      dev->draw_mode=GrXOR;
}

/*----------------------------------------------------------------------*\
 * plD_state_vga()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*----------------------------------------------------------------------*/

void 
plD_state_vga(PLStream *pls, PLINT op)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    switch (op) {

    case PLSTATE_WIDTH:
        dev->gnusvgaline.lno_width=pls->width;
	break;

    case PLSTATE_COLOR0:
	dev->colour = pls->icol0;
	if (dev->colour == PL_RGB_COLOR) {
	    int r = pls->curcolor.r;
	    int g = pls->curcolor.g;
	    int b = pls->curcolor.b;
	    if (dev->totcol < GrNumColors()) 
	       {
	        GrAllocCell();
		GrSetColor(++dev->totcol, r, g, b);
		dev->colour = dev->totcol;
	       }
	}
	break;

    case PLSTATE_COLOR1:
        { int icol1, ncol1, r, g, b;
        if ((ncol1 = MIN(GrNumColors() - pls->ncol0, pls->ncol1)) < 1)
            break;

        icol1 = pls->ncol0 + (pls->icol1 * (ncol1-1)) / (pls->ncol1-1);

        r = pls->curcolor.r;
        g = pls->curcolor.g;
        b = pls->curcolor.b;
        GrAllocCell();
        GrSetColor(icol1, r, g, b);
        dev->colour = icol1;
	}
	break;

    case PLSTATE_CMAP0:
    case PLSTATE_CMAP1:
	if (pls->color)
	    setcmap(pls);
	break;
    }
}

#ifdef PLD_gnusvga

/*----------------------------------------------------------------------*\
 * plD_eop_vga()
 *
 * End of page.
\*----------------------------------------------------------------------*/

void
plD_eop_vga(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (page_state == DIRTY) 
       {
        if ( (dev->double_buffer!=NULL)&&(pls->db!=0)&&(dev->visual_screen!=NULL) ) 
           {
            GrSetContext(dev->visual_screen);
            GrBitBlt(NULL,0,0,dev->double_buffer,0,0,
                      GrScreenX(), GrScreenY(),GrWRITE);
           }
        else if ( (pls->db!=0) && ((dev->double_buffer==NULL)||(dev->visual_screen==NULL)) ) 
           {
            plexit("Wacko error with double buffering I think");
           }

	if ( ! pls->nopause ) 
	    WaitForPage(pls);
      }

    page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
 * plD_bop_vga()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void
plD_bop_vga(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if ( dev->Xhair_on == 1 ) DestroyXhair ( dev );

    pls->page++;
    plD_eop_vga(pls);

    if ( (pls->db==1) && (dev->double_buffer!=NULL) )
       {
        GrSetContext(dev->double_buffer);
        GrClearContext(0);
       }
    else    
        GrClearScreen(0);

}

/*----------------------------------------------------------------------*\
 * plD_tidy_vga()
 *
 * Close graphics file or otherwise clean up.
 * Checks to see if memory was allocated to a "saved screen" and 
 * destroys if necessary.
 * Since I use the same "svga_text()" function to rest the screen as I
 * do for swapping between text and graphics, I use a "dev->don't_copy_screen" 
 * field to make sure the "svga_text()" doesn't copy the screen.
\*----------------------------------------------------------------------*/

void
plD_tidy_vga(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if ( dev->Xhair_on == 1 ) DestroyXhair ( dev );

    GrMouseUnInit();
    dev->dont_copy_screen=1;
    svga_text(pls);
    
    if (dev->switch_screen!=NULL) 
       {
        GrDestroyContext(dev->switch_screen);
        dev->switch_screen=NULL;
       } 

    if (dev->visual_screen!=NULL) 
       {
        GrDestroyContext(dev->visual_screen);
        dev->visual_screen=NULL;
       }
       
    if (dev->double_buffer!=NULL) 
       {
        GrDestroyContext(dev->double_buffer);
        dev->double_buffer=NULL;
       } 

    if (pls->dev!=NULL)
       {
        free(pls->dev);
        pls->dev=NULL;
       } 
}



/*--------------------------------------------------------------------------*\
 * gnusvga_GrMouseGetEvent ()
 *
 * Wrapper function for mouse events.
 * Function basically sits there and does the cross-hair thing if necessary,
 * otherwise does nothing but hand over to GRX20's mouse function.
\*--------------------------------------------------------------------------*/

void gnusvga_GrMouseGetEvent (int flags,PLStream *pls, GrMouseEvent *event)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
int ret=0;

do {
    if (dev->Xhair_on==1)
       {

        GrMouseGetEvent(flags|GR_M_MOTION,event);
        
        if (event->flags & GR_M_MOTION)
           {

#ifdef BLIT_CROSSHAIR

            GrBitBlt(NULL,0,dev->last_y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
            GrBitBlt(NULL,dev->last_x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);
        
            GrBitBlt(NULL,0,event->y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
            GrBitBlt(NULL,event->x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);

#else

            GrHLine(0,GrScreenX(),dev->last_y,dev->fg|GrXOR);
            GrVLine(dev->last_x,0,GrScreenY(),dev->fg|GrXOR);

            GrHLine(0,GrScreenX(),event->y,dev->fg|GrXOR);
            GrVLine(event->x,0,GrScreenY(),dev->fg|GrXOR);

#endif
            dev->last_x=event->x;
            dev->last_y=event->y;
           }
        if (flags & event->flags)
           {
            ret=1;
           }
        }
    else
       {
       GrMouseGetEvent(flags,event);
       ret=1;
       }
 } while (ret==0);
}

/*--------------------------------------------------------------------------*\
 * gnusvga_GetCursorCmd()
 *
 * Waits for a graphics input event and returns coordinates.
\*--------------------------------------------------------------------------*/

static void
gnusvga_GetCursorCmd(PLStream *pls, PLGraphicsIn *ptr)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

    plGinInit(gin);
    dev->locate_mode=1;
    while ( (gin->pX < 0) && (dev->locate_mode) ) {
        gnusvga_GrMouseGetEvent(GR_M_BUTTON_DOWN+GR_M_KEYPRESS,pls,&dev->mevent);
        TranslateEvent(pls, &dev->mevent, gin);
    }
    *ptr = *gin;
}

/*--------------------------------------------------------------------------*\
 * ConfigBufferingCmd()
 *
 * Based on X-windows driver
\*--------------------------------------------------------------------------*/

static void ConfigBufferingCmd( PLStream *pls, PLBufferingCB *ptr )
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

switch (ptr->cmd) {

    case PLESC_DOUBLEBUFFERING_ENABLE:
            if (pls->db==0)
               {
                pls->db = 1;
                init_double_buffer(dev);
               }
	break;

    case PLESC_DOUBLEBUFFERING_DISABLE:
        /*
         *   First of all make sure that any active double buffer is
         *   copied across to the visual screen, then clear any memory.
         */
        if ( (pls->db==1)  &&
             (dev->double_buffer!=NULL) &&
             (dev->visual_screen!=NULL)    )
           {
            GrSetContext(dev->visual_screen);
            GrBitBlt(NULL,0,0,dev->double_buffer,0,0,
                      GrScreenX(), GrScreenY(),GrWRITE);
            GrDestroyContext(dev->visual_screen);
            dev->visual_screen=NULL;
            GrDestroyContext(dev->double_buffer);
            dev->double_buffer=NULL;
            }
	pls->db = 0;
	break;

    case PLESC_DOUBLEBUFFERING_QUERY:
	ptr->result = pls->db;
	break;

    default:
	printf( "Unrecognized buffering request ignored.\n" );
	break;
    }
}


/*----------------------------------------------------------------------*\
 * plD_esc_vga()
 *
 * Escape function.
 *
 * Functions:
 *
 *	PLESC_FILL	        Fill polygon
 *	PLESC_GETC	        Get coordinates upon mouse click
 *      PLESC_DOUBLEBUFFERING   Activate double buffering
 * 	PLESC_XORMOD 	        set/reset xor mode
\*----------------------------------------------------------------------*/

void
plD_esc_vga(PLStream *pls, PLINT op, void *ptr)
{


    switch (op) {
      case PLESC_TEXT:  /* Set to text mode */
	svga_text(pls);
	break;

      case PLESC_GRAPH: /* set to graphics mode */
	svga_graph(pls);
	break;

    case PLESC_GETC:    
        gnusvga_GetCursorCmd(pls, (PLGraphicsIn *) ptr);
	break; 

      case PLESC_FILL:  /* fill */
	fill_polygon(pls);
	break;

    case PLESC_DOUBLEBUFFERING:
	ConfigBufferingCmd(pls, (PLBufferingCB *) ptr );
	break;

    case PLESC_XORMOD:
	XorMod(pls, (PLINT *) ptr);
	break;

#ifdef GRX_DO_JPEG
    case PLESC_SET_COMPRESSION:
         if ( ((int) ptr>0)&&((int) ptr<100) )
            {
             pls->dev_compression=(int) ptr;
            }
        break;
#endif

    }
}


/*----------------------------------------------------------------------*\
 * svga_text()
 *
 * Switch to text mode.
 *
 * This code first saves a copy of the video memory, so if it is
 * necessary to do a redraw, then everything is there to be pulled up
 * quickly.
\*----------------------------------------------------------------------*/

static void
svga_text(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (pls->graphx == GRAPHICS_MODE) 
       {
        if ( dev->Xhair_on == 1 ) DestroyXhair ( dev );
           
        if ( (dev->switch_screen==NULL)&&(dev->dont_copy_screen==0) ) 
           {
            dev->switch_screen=GrCreateContext(GrScreenX(), GrScreenY(),NULL,NULL);
            GrBitBlt(dev->switch_screen,0,0,NULL,0,0,GrScreenX(), GrScreenY(),GrWRITE);               
           }

	GrSetMode(GR_default_text);
	pls->graphx = TEXT_MODE;
	page_state = CLEAN;
      }
}

/*----------------------------------------------------------------------*\
 * svga_graph()
 *
 * Switch to graphics mode.
 * Will restore a copy of the previous graphics screen if one has been
 * saved.
\*----------------------------------------------------------------------*/

static void
svga_graph(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (pls->graphx == TEXT_MODE) {
            if ((pls->xlength!=0)&&(pls->ylength!=0))
               {
               GrSetMode(GR_width_height_graphics,pls->xlength,pls->ylength);
               }
            else
               {
               GrSetMode(GR_default_graphics);
               }
            dev->totcol = 16;
            setcmap(pls);
	if (dev->switch_screen!=NULL) /* Ok, seems like we saved a copy of the screen */
           {
            GrBitBlt(NULL,0,0,dev->switch_screen,0,0,GrScreenX(), GrScreenY(),GrWRITE);               
            GrDestroyContext(dev->switch_screen);
            dev->switch_screen=NULL;
           }
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;
    }
}

/*----------------------------------------------------------------------*\
 * plpause()
 *
 * Wait for a keystroke or mouse click.
 * Prompts user.
 * Saves screen's top line so prompt is not destructive
\*----------------------------------------------------------------------*/

static void
plpause(PLStream *pls)
{
GrMouseEvent event;
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (pls->nopause) 
	return;

    SaveTopOfScreen(dev);    

    GrTextXY(0, 0, "Pause->", dev->fg, 0);       /* Prompt for pausing */
    gnusvga_GrMouseGetEvent(GR_M_LEFT_DOWN+GR_M_KEYPRESS,pls,&event);       /* Wait for a mouse event */
    
    RestoreTopOfScreen(dev);    

}


/*----------------------------------------------------------------------*\
 * WaitForPage()
 *
 * This routine waits for the user to advance the plot, while handling
 * all other events.
\*----------------------------------------------------------------------*/

static void
WaitForPage(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    while ( ! exit_eventloop ) 
          {
           gnusvga_GrMouseGetEvent(GR_M_LEFT_DOWN+GR_M_KEYPRESS,pls,&dev->mevent);
  	   EventHandler(pls, &dev->mevent); 
         }
    exit_eventloop = FALSE;
}

/*----------------------------------------------------------------------*\
 * do_location()
 *
 * Prints the World Co-ordinate in the top of the display and then wait
 * for a keypress or mouse button before continuing.
 * Function constantly updates the coordinates (damn it's a nice function)
\*----------------------------------------------------------------------*/

void do_location(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
    PLGraphicsIn *gin = &(dev->gin);
    char buf[80];
    GrMouseEvent event;
    
    bzero(buf,80);

/* Call user locate mode handler if provided */

    if (pls->LocateEH != NULL)
	(*pls->LocateEH) (gin, pls->LocateEH_data, &dev->locate_mode);

/* Use default procedure */

    else {
    
          SaveTopOfScreen(dev);
          
          for (;;) {
          	
          	if (plTranslateCursor(gin)) 
                     {
                       sprintf(buf,"Value at cursor is : X = %f, Y = %f   ", gin->wX, gin->wY);
                       GrTextXY(0, 0, buf, dev->fg, 0);
                     }
          	else
                     {
                       GrTextXY(0, 0, "Cannot translate                                   ", dev->fg, 0);
                     }

                 gnusvga_GrMouseGetEvent(GR_M_MOTION+GR_M_LEFT_DOWN+GR_M_KEYPRESS,pls,&event);       /* Wait for a mouse event */

                 if ((event.flags & GR_M_MOTION)!=1)    /* leave on anything *but* a movement */
                     break;
                 
                 gin->pX = event.x;
                 gin->pY = event.y;
                 gin->dX = (PLFLT) event.x / dev->vgax;
                 gin->dY = 1.0 - (PLFLT) event.y / dev->vgay;
    
                   } /* End loop */
                   
           RestoreTopOfScreen( dev );
         
          }

}


/*----------------------------------------------------------------------*\
 * EventHandler()
 *
 * Event handler routine.
 * Reacts to keyboard or mouse input.
\*----------------------------------------------------------------------*/

static void
EventHandler(PLStream *pls, GrMouseEvent *event)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    TranslateEvent(pls, event, &dev->gin);

#ifdef DEBUG
    {
      char buf[200];
      sprintf(buf, "flags 0x%x, buttons 0x%x, key 0x%x                ",
	      event->flags, event->buttons, event->key);
      GrTextXY(0, 0, buf, 15, 0);
      sprintf(buf, "Input char 0x%x, Keycode 0x%x, string: %s               ",
	      event->key, dev->gin.keysym, dev->gin.string);
      GrTextXY(0, 50, buf, 15, 0);
      sprintf(buf, "button %d, pX %d, pY %d, state 0x%x                     ", 
	      dev->gin.button, dev->gin.pX, dev->gin.pY, dev->gin.state);
      GrTextXY(0, 100, buf, 15, 0);
    }
#endif

/* Call user event handler */
/* Since this is called first, the user can disable all plplot internal
   event handling by setting key.code to 0 and key.string to '\0' */

    if (dev->gin.button) {
        if (pls->ButtonEH != NULL)
	    (*pls->ButtonEH) (&dev->gin, pls->ButtonEH_data, &exit_eventloop);
    } 
    else {
        if (pls->KeyEH != NULL)
            (*pls->KeyEH) (&dev->gin, pls->KeyEH_data, &exit_eventloop);
    }

/* Handle internal events */


/* Advance to next page (i.e. terminate event loop) on a <eol> */

    if (dev->gin.button == 1)
        exit_eventloop = TRUE;

switch(dev->gin.keysym)
      {

    case PLK_Linefeed:
    case PLK_Return:
	exit_eventloop = TRUE;
	break;

    case 'l':
        dev->gin.pX = event->x;
        dev->gin.pY = event->y;
        dev->gin.dX = (PLFLT) event->x / dev->vgax;
        dev->gin.dY = 1.0 - (PLFLT) event->y / dev->vgay;
        do_location(pls);
        break;

    case 'x':
        plGinInit(&dev->gin);
        if (dev->Xhair_on==0)
           {
            CreateXhair( dev, event );
           }
        else
           {
            DestroyXhair( dev );
           }
        break;
        
#if GRX_VERSION_API >= 0x0229 
#ifdef GRX_DO_TIFF
       case PLK_F10:  // F10 
       SaveContextToTiff(NULL, newname("tif"), 0,"Created by GNUSVGA");
       break;
#endif

#ifdef GRX_DO_JPEG
       case PLK_F11:  // F11
       SaveContextToJpeg(NULL, newname("jpg"), pls->dev_compression);
       break;
#endif

#ifdef GRX_DO_BMP
       case PLK_F12:   // F12
       GrSaveBmpImage(newname("bmp"), NULL, 0, 0, GrScreenX(), GrScreenY());
       break;
#endif
#endif
      }


/* Terminate on a 'Q' (not 'q', since it's too easy to hit by mistake) */

    if (dev->gin.string[0] == 'Q') {
	pls->nopause = TRUE;
	plexit("");
    }
}


/*--------------------------------------------------------------------------*\
 * TranslateEvent()
 *
 * Fills in the PLGraphicsIn from a MouseEvent.  All keys are not yet 
 * translated correctly.
\*--------------------------------------------------------------------------*/

static void
TranslateEvent(PLStream *pls, GrMouseEvent *event, PLGraphicsIn *gin)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
    gin->string[0] = '\0';

/* Translate DJGPP GrMouseEvent into PlPLot event */

    if (event->flags & GR_M_KEYPRESS) 
       {
        gin->keysym = event->key; 
	if ((isprint(event->key))&&(event->key<255)) 
	   {
	    gin->string[0] = event->key;
	    gin->string[1] = '\0';
           }
	else 
	   {
	    switch (event->key) {

	    case K_F1: case K_F2: case K_F3: 
	    case K_F4: case K_F5: case K_F6:
	    case K_F7: case K_F8: case K_F9: 
	    case K_F10:
	      gin->keysym = event->key  + (PLK_F1- K_F1);
	      break;

	    case K_F11: // F11
  	         gin->keysym=PLK_F11;
      	         break;

	    case K_F12: // F12
  	         gin->keysym=PLK_F12;
      	         break;

	    case K_Home: 
            case K_EHome:
	      gin->keysym=PLK_Home;
	      break;

	    case K_Up: case K_EUp:
	      gin->keysym = PLK_Up;
	      break;

	    case K_Right: case K_ERight:
	      gin->keysym=PLK_Right;
	      break;

	    case K_Down: case K_EDown:
	      gin->keysym = PLK_Down;
	      break;

	    case K_Left: case K_ELeft:
	      gin->keysym = PLK_Left;
	      break;

	    case K_PageDown: case K_EPageDown:
	      gin->keysym = PLK_Prior; 
	      break;

	    case K_PageUp: case K_EPageUp:
	      gin->keysym = PLK_Next;
	      break;

	    case K_Insert: case K_EInsert:
	      gin->keysym = PLK_Insert;
	      break;
	    default: gin->keysym = event->key;
	      break;

	    }
	}
	gin->button = 0;
	gin->state = event->kbstat;
    }

    else if (event->flags & GR_M_BUTTON_DOWN) 
       {
         switch (event->flags) 
                {
                case GR_M_LEFT_DOWN:
                     gin->button = 1;
                     break;
                case GR_M_MIDDLE_DOWN:
                     gin->button = 2;
                     break;
                case GR_M_RIGHT_DOWN:
                     gin->button = 3;
                     break;
              }
      gin->keysym = 0x20;
      gin->pX = event->x;
      gin->pY = event->y;
      gin->dX = (PLFLT) event->x / dev->vgax;
      gin->dY = 1.0 - (PLFLT) event->y / dev->vgay;
      gin->state = event->kbstat;
    }

}

/*----------------------------------------------------------------------*\
 * CreateXhair()
 *
 * Creates the cross hairs. 
 * Cross hairs can be implimented as "blits" or lines.
\*----------------------------------------------------------------------*/

static void CreateXhair ( gnu_grx_Dev *dev, GrMouseEvent *event )
{
#ifdef BLIT_CROSSHAIR

GrContext *swap_context=NULL;

 /*
  *  create the contexts holding the two cross hairs
  */
  
 dev->Xhair_X=GrCreateContext(GrScreenX(),1,NULL,NULL);
 dev->Xhair_Y=GrCreateContext(1, GrScreenY(),NULL,NULL);
 
 swap_context=GrSaveContext(NULL);    /* save a temp copy of the current context */
 GrSetContext(dev->Xhair_X);          /* Set the context to the "X" line */
 GrClearContext(dev->fg);             /* Clear it out with forground colour */
 GrSetContext(dev->Xhair_Y);
 GrClearContext(dev->fg);
 GrSetContext(swap_context);          /* Set the context back */

#endif

 if (dev->toggle_xhair==0)
    {
#ifdef BLIT_CROSSHAIR

     GrBitBlt(NULL,0,event->y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
     GrBitBlt(NULL,event->x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);

#else

    GrHLine(0,GrScreenX(),event->y,dev->fg|GrXOR);
    GrVLine(event->x,0,GrScreenY(),dev->fg|GrXOR);

#endif

     dev->last_x=event->x;
     dev->last_y=event->y;
    } 
 else
    {
#ifdef BLIT_CROSSHAIR

     GrBitBlt(NULL,0,dev->last_y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
     GrBitBlt(NULL,dev->last_x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);
     
#else

    GrHLine(0,GrScreenX(),dev->last_y,dev->fg|GrXOR);
    GrVLine(dev->last_x,0,GrScreenY(),dev->fg|GrXOR);

#endif

    }



 dev->Xhair_on=1;

}

/*----------------------------------------------------------------------*\
 * DestroyXhair()
 *
 * Creates the cross hairs.
\*----------------------------------------------------------------------*/

static void DestroyXhair ( gnu_grx_Dev *dev )
{

   
#ifdef BLIT_CROSSHAIR   

  GrBitBlt(NULL,0,dev->last_y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
  GrBitBlt(NULL,dev->last_x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR); 

  if (dev->Xhair_X!=NULL)
    {
    GrDestroyContext(dev->Xhair_X);
    dev->Xhair_X=NULL;
    }
  if (dev->Xhair_Y!=NULL)
    {
    GrDestroyContext(dev->Xhair_Y);
    dev->Xhair_Y=NULL;
    }

#else

  GrHLine(0,GrScreenX(),dev->last_y,dev->fg|GrXOR);
  GrVLine(dev->last_x,0,GrScreenY(),dev->fg|GrXOR);

#endif
 
 if (dev->toggle_xhair==0)
    {
     dev->last_x=0;
     dev->last_y=0;
   }  

 
 dev->Xhair_on=0;

}

/*----------------------------------------------------------------------*\
 * SaveTopOfScreen()
 *
 * Function saves the top "line" of the screen to a buffer (using
 * blitting) so writing can be done to it and later restored.
\*----------------------------------------------------------------------*/

static void SaveTopOfScreen ( gnu_grx_Dev *dev )

{
          /* Turn off Cross Hairs if they were turned on (nasty if not) 
           * since it causes problems with the top of the line
           */

          if (dev->Xhair_on==1)
             {
             dev->toggle_xhair=1;
             DestroyXhair(dev);
             }

          /*  Save the top bit of the screen so we can overwrite
           *  without too many problems 
           */

          dev->top_line=GrCreateContext(GrScreenX(), 16,NULL,NULL);
          GrBitBlt(dev->top_line,0,0,NULL,0,0,GrScreenX(), 16,GrWRITE);
          
          /*
           * Turn back on cross hair now if necessary 
           */

          if (dev->toggle_xhair==1)
             {
             CreateXhair(dev, NULL);
             dev->toggle_xhair=0;
             }

}

/*----------------------------------------------------------------------*\
 * RestoreTopOfScreen()
 *
 * Function restores the saved top "line" of the screen to a buffer 
 * (using blitting) so damage caused by writing can be undone.
\*----------------------------------------------------------------------*/

static void RestoreTopOfScreen ( gnu_grx_Dev *dev )

{
            if (dev->top_line!=NULL)    /* Ok, seems like we saved a copy of the top line */
               {                        
               
                if (dev->Xhair_on==1)   /* Turn off cross hair while fixing top */
                   {
                    dev->toggle_xhair=1;
                    DestroyXhair(dev);
                   }
               
                GrBitBlt(NULL,0,0,dev->top_line,0,0,GrScreenX(), 16,GrWRITE); /* So we will restore anything the top line overwrote */
                GrDestroyContext(dev->top_line);        /* remove copy of original top line */
                dev->top_line=NULL;

                /*
                 * Turn back on cross hair now if necessary
                 */
               if (dev->toggle_xhair==1)
                  {
                   CreateXhair(dev, NULL);
                   dev->toggle_xhair=0;
                  }
                
               }
}

#endif

#if defined(GRX_DO_TIFF) || defined(GRX_DO_BMP) || defined(GRX_DO_JPEG) || defined(PLD_tiff) || defined(PLD_jpeg) || defined(PLD_bmp)

const char gnu_alphabet[]="0123456789abcdefghijklmnopqrstuvwxyz";

/*
 *    newname ()
 *
 *    Function returns a new name
 *    Works from the current time, year etc..
 *    Joins them all together and makes a string which as long as the system
 *    clock increases, should remain unique each time. Can be called up to
 *    36 times a second, after that the program delays for 1/10 of a sec
 *    and tries again for a unique file name. Function written for
 *    DOS, but should be portable.
 *    Function is Y2K+ compatible, but won't work with dates before
 *    2000. Good until 2036, then it will start reallocating names.
 *    Only a problem if you plan to use the same computer, program,
 *    directory, and files for 36 years :)
 */

char *newname ( char *ext)
{

static char name[13];
static char last_name[13];
time_t x;
struct tm *t;
static int i;
static int old_time;

bzero(name,13);

x=time(NULL);
t = gmtime(&x);
while ((old_time==x)&&(i==35))
   {
    delay(100);
    x=time(NULL);
    t = gmtime(&x);
   }

name[0]=gnu_alphabet[(t->tm_year-100)%36];
name[1]=gnu_alphabet[t->tm_mon+10];
name[2]=gnu_alphabet[t->tm_mday+4];
name[3]=gnu_alphabet[t->tm_hour];
name[4]=(t->tm_min>30 ? gnu_alphabet[t->tm_min-29] : gnu_alphabet[t->tm_min+1]);
name[5]=(t->tm_min>30 ? (t->tm_sec>30 ? 'a' : 'b') : (t->tm_sec>30 ? 'c' : 'd'));
name[6]=(t->tm_sec>30 ? gnu_alphabet[t->tm_sec-29] : gnu_alphabet[t->tm_sec+1]);

if(strncmp(last_name,name,7)==0)
  {
   ++i;
  }
else
  {
   i=0;
  }
name[7]=gnu_alphabet[i];
name[8]='.';

if (ext!=NULL) strncpy(&name[9],ext,3);
strcpy(last_name,name);
old_time=x;
return(name);
}


/*----------------------------------------------------------------------*\
 * gnusvga_get_a_file_name()
 *
 * Checks to see if there is a file name already, if there isn't then 
 * it asks the user for one, allocates memory for the name, and makes 
 * one.
\*----------------------------------------------------------------------*/

void gnusvga_get_a_file_name ( PLStream *pls )
{
int i=0;
   if (pls->BaseName==NULL)  /* Check to see if a File name has been defined yet */
     {
     
      /* 
       *  Allocate memory for the file name.
       *  Should I be doing this like this ?
       *  I *think* this will be cleaned up later on by plplot, so i will do it anyway 
       */
       
      if ((pls->BaseName=malloc(80))==NULL)
         plexit("Could not allocate some memory");
      if ((pls->FileName=malloc(80))==NULL)
         plexit("Could not allocate some memory");
         
      do { /* Continue to ask for the file name until we get an answer we like */
          fprintf(stderr,"Enter file name (include \"0\" or \"00\" if defining a family, or use \"auto\" to \ndo automatic naming): ");
          fgets(pls->BaseName, 79, stdin);
          if (pls->BaseName!=NULL)
             {
              while (pls->BaseName[i]!=0)   /* strip off the trailing CR/LF or LF sequence, since we really don't want that stuffing things up */
                    {
                     if ((pls->BaseName[i]==10)||(pls->BaseName[i]==13))
                        pls->BaseName[i]=0;
                     ++i; 
                     if (i>79) break;
                    }
         
              strncpy(pls->FileName,pls->BaseName,79); /* Copy BaseName to FileName */
              if (strchr(pls->BaseName,'0')!=NULL) /* Test to see if we might have Family support */
                 pls->family=1;
             }
          } while (pls->BaseName==NULL);
     }
}

/*----------------------------------------------------------------------*\
 * gnusvga_expand_BaseName()
 *
 * Function takes the BaseName and sees if there is a family option set.
 * If there is, then it makes sure the file name format is 'legal' for
 * expanding the 'family' out, and then it expands the name. Otherwise,
 * it prompts the user for another file name.
 * Basically the algorithm replaces the last occurrence of "0" with
 * an alpha-numeric character. That makes for a total of 36
 * possibilities with a single "0", ie "image0.tif". When there are
 * two 0's, ie "image00.tif", then there are 1296 possible file names,
 * which should really be enough for anyone I hope.
\*----------------------------------------------------------------------*/

void gnusvga_expand_BaseName( PLStream *pls )
{
char *zero=NULL;
char yn[60];

if (pls->page>1)
   {
    if (pls->family==0) /* See if families aren't specified */
       {
        if (strchr(pls->BaseName,'0')!=NULL) /* if they gave a name supporting families, then check if they anted them */
           {
            do {
                fprintf(stderr,"Do you want to enable family support (y/n) : ");
                fgets(yn, sizeof(yn), stdin);
               } while ((yn[0]=='y')&&(yn[0]=='Y')&&(yn[0]=='n')&&(yn[0]=='N'));
            if ((yn[0]=='y')||(yn[0]=='Y'))
               {
               pls->family=1;
               }
           }
       }
            
    if (pls->family==0) /* See if families aren't specified */
       {
        free(pls->BaseName);
        pls->BaseName=NULL;
        free(pls->FileName);
        gnusvga_get_a_file_name ( pls );
       }
    else
       {
        strcpy(pls->FileName,pls->BaseName);
        zero=(char *)strrchr(pls->FileName,'0');
        if (zero==NULL)
           plabort("Incorrect format for family name given (must have a \"0\" or \"00\" in the name)");
        else
        {
        if (zero[-1]=='0')
           {
            zero[-1]=gnu_alphabet[(pls->page-1)/36];
           }
        else if ((pls->page-1)>35)
           plabort("Number of files exceeded (next time try \"00\" in filename instead of \"0\")");
   
        zero[0]=gnu_alphabet[(pls->page-1)%36];
        }
     }
                 
   }
}
#endif

#ifdef PLD_tiff

void plD_init_tiff(PLStream *pls);
void plD_tidy_tiff(PLStream *pls);
void plD_bop_tiff(PLStream *pls);
void plD_esc_tiff(PLStream *pls, PLINT op, void *ptr);
void plD_eop_tiff(PLStream *pls);


/*----------------------------------------------------------------------*\
 * plD_init_tiff()
 *
 * Initialize device.
\*----------------------------------------------------------------------*/

void plD_init_tiff(PLStream *pls)
{
    gnu_grx_Dev *dev=NULL;

    pls->termin = 0;		/* is an interactive terminal */
    pls->icol0 = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->graphx = TEXT_MODE;
    pls->dev_fill0 = 1;

    if (!pls->colorset)
	pls->color = 1;

    if (pls->dev==NULL)
       plD_init_gnu_grx_dev(pls);

      dev=(gnu_grx_Dev *)pls->dev;
      dev->Old_Driver_Vector=GrDriverInfo->vdriver;
      GrSetDriver ("memory");

      if (pls->pageset!=1)
         {
	  pls->xlength = plGetInt("Enter desired horizontal size in pixels  : ");
          pls->ylength = plGetInt("Enter desired vertical   size in pixels  : ");
         }

        if ( (pls->ncol1 < 3) && (pls->ncol0 < 3) )
           {
            GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 2);
           }
        else if ( (pls->ncol1 > 256) || (pls->ncol0 > 256) )
           {
            GrSetMode (GR_width_height_graphics, pls->xlength, pls->ylength);
           }
        else
           {
            GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 256);
           }
           
        setcmap(pls);
	dev->totcol = 16;		/* Reset RGB map so we don't run out of
				   indicies */
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;
		

    dev->vgax = GrSizeX() - 1;	/* should I use -1 or not??? */
    dev->vgay = GrSizeY() - 1;

    if (pls->xdpi==0) 
       {
        plP_setpxl(2.5, 2.5);	/* My best guess.  Seems to work okay. */
       }
    else
       {
        pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
        plP_setpxl(pls->xdpi/25.4,pls->ydpi/25.4); /* Convert DPI to pixels/mm */
       } 

    plP_setphy(0, dev->vgax, 0, dev->vgay);

dev->gnusvgaline.lno_width=pls->width;
dev->gnusvgaline.lno_pattlen=0;

}



/*----------------------------------------------------------------------*\
 * plD_esc_tiff()
 *
 * Escape function.
\*----------------------------------------------------------------------*/

void plD_esc_tiff(PLStream *pls, PLINT op, void *ptr)
{
    switch (op) {

      case PLESC_FILL:  /* fill */
	fill_polygon(pls);
	break;
    }
}




/*----------------------------------------------------------------------*\
 * plD_eop_tiff()
 *
 * End of page.
\*----------------------------------------------------------------------*/

void plD_eop_tiff(PLStream *pls)
{

    if (page_state == DIRTY) 
       {
  
        gnusvga_get_a_file_name(pls);

        if ( (strncasecmp(pls->FileName,"auto",4)==0) )
           {
            SaveContextToTiff(NULL, newname("tif"), 0,"Created by GNUSVGA");
           }
       else
           {
            gnusvga_expand_BaseName(pls);
            SaveContextToTiff(NULL,pls->FileName, 0,"Created by GNUSVGA");
           }
      }

    page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
 * plD_bop_tiff()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void plD_bop_tiff(PLStream *pls)
{
    pls->page++;
    plD_eop_tiff(pls);
    GrClearScreen(0);
}

/*----------------------------------------------------------------------*\
 * plD_tidy_tiff()
 *
 * Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void plD_tidy_tiff(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

      if (dev->Old_Driver_Vector) 
         {
          GrSetDriver (dev->Old_Driver_Vector->name);
          dev->Old_Driver_Vector=NULL;
         }

   free (pls->dev);
   pls->dev=NULL;

}
#endif

#ifdef PLD_jpeg

void plD_init_jpeg(PLStream *pls);
void plD_tidy_jpeg(PLStream *pls);
void plD_bop_jpeg(PLStream *pls);
void plD_esc_jpeg(PLStream *pls, PLINT op, void *ptr);
void plD_eop_jpeg(PLStream *pls);



/*----------------------------------------------------------------------*\
 * plD_init_jpeg()
 *
 * Initialize device.
\*----------------------------------------------------------------------*/

void plD_init_jpeg(PLStream *pls)
{
    gnu_grx_Dev *dev=NULL;
    pls->termin = 0;		/* is an interactive terminal */
    pls->icol0 = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->graphx = TEXT_MODE;

    pls->dev_fill0 = 1;

    if (!pls->colorset)
	pls->color = 1;

    if (pls->dev==NULL)
       plD_init_gnu_grx_dev(pls);

      dev=(gnu_grx_Dev *)pls->dev;

/* Set up device parameters */
      dev->Old_Driver_Vector=GrDriverInfo->vdriver;
      GrSetDriver ("memory");

      if (pls->pageset!=1)
         {
	  pls->xlength = plGetInt("Enter desired horizontal size in pixels  : ");
          pls->ylength = plGetInt("Enter desired vertical   size in pixels  : ");
         }


        GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 256);

        setcmap(pls);
	dev->totcol = 16;		/* Reset RGB map so we don't run out of
				   indicies */
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;
		

    dev->vgax = GrSizeX() - 1;	/* should I use -1 or not??? */
    dev->vgay = GrSizeY() - 1;

    if (pls->xdpi==0) 
       {
        plP_setpxl(2.5, 2.5);	/* My best guess.  Seems to work okay. */
       }
    else
       {
        pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
        plP_setpxl(pls->xdpi/25.4,pls->ydpi/25.4); /* Convert DPI to pixels/mm */
       } 

    plP_setphy(0, dev->vgax, 0, dev->vgay);

    dev->gnusvgaline.lno_width=pls->width;
    dev->gnusvgaline.lno_pattlen=0;

}



/*----------------------------------------------------------------------*\
 * plD_esc_jpeg()
 *
 * Escape function.
\*----------------------------------------------------------------------*/

void plD_esc_jpeg(PLStream *pls, PLINT op, void *ptr)
{
    switch (op) {

    case PLESC_FILL: 
	fill_polygon(pls);
	break;

    case PLESC_SET_COMPRESSION:
         if ( ((int) ptr>0)&&((int) ptr<100) )
            {
             pls->dev_compression=(int) ptr;
            }
        break;
	
    }
}


/*----------------------------------------------------------------------*\
 * plD_eop_jpeg()
 *
 * End of page.
\*----------------------------------------------------------------------*/

void plD_eop_jpeg(PLStream *pls)
{

    if (page_state == DIRTY) 
       {
  
        gnusvga_get_a_file_name(pls);

        if ( (strncasecmp(pls->FileName,"auto",4)==0) )
           {
            SaveContextToJpeg(NULL, newname("jpg"), pls->dev_compression);
           }
       else
           {
            gnusvga_expand_BaseName(pls);
            SaveContextToJpeg(NULL,pls->FileName, pls->dev_compression);
           }
      }

    page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
 * plD_bop_jpeg()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void plD_bop_jpeg(PLStream *pls)
{
    pls->page++;
    plD_eop_jpeg(pls);
    GrClearScreen(0);
}

/*----------------------------------------------------------------------*\
 * plD_tidy_jpeg()
 *
 * Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void plD_tidy_jpeg(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

      if (dev->Old_Driver_Vector) 
         {
          GrSetDriver (dev->Old_Driver_Vector->name);
          dev->Old_Driver_Vector=NULL;
         }
free(pls->dev);
pls->dev=NULL;
}
#endif

#ifdef PLD_bmp

void plD_init_bmp(PLStream *pls);
void plD_tidy_bmp(PLStream *pls);
void plD_bop_bmp(PLStream *pls);
void plD_esc_bmp(PLStream *pls, PLINT op, void *ptr);
void plD_eop_bmp(PLStream *pls);

/*----------------------------------------------------------------------*\
 * plD_init_bmp()
 *
 * Initialize device.
\*----------------------------------------------------------------------*/

void plD_init_bmp(PLStream *pls)
{
    gnu_grx_Dev *dev=NULL;
    pls->termin = 0;		/* is an interactive terminal */
    pls->icol0 = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->graphx = TEXT_MODE;

    pls->dev_fill0 = 1;

    if (!pls->colorset)
	pls->color = 1;

    if (pls->dev==NULL)
       plD_init_gnu_grx_dev(pls);

      dev=(gnu_grx_Dev *)pls->dev;

/* Set up device parameters */
      dev->Old_Driver_Vector=GrDriverInfo->vdriver;
      GrSetDriver ("memory");

      if (pls->pageset!=1)
         {
	  pls->xlength = plGetInt("Enter desired horizontal size in pixels  : ");
          pls->ylength = plGetInt("Enter desired vertical   size in pixels  : ");
         }
       
        if ( (pls->ncol1 < 3) && (pls->ncol0 < 3) )
           {
            GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 2);
           }
        else if ( (pls->ncol1 > 256) || (pls->ncol0 > 256) )
           {
            GrSetMode (GR_width_height_graphics, pls->xlength, pls->ylength);
           }
        else
           {
            GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 256);
           }

        setcmap(pls);
	dev->totcol = 16;		/* Reset RGB map so we don't run out of
				   indicies */
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;
		

    dev->vgax = GrSizeX() - 1;	/* should I use -1 or not??? */
    dev->vgay = GrSizeY() - 1;

    if (pls->xdpi==0) 
       {
        plP_setpxl(2.5, 2.5);	/* My best guess.  Seems to work okay. */
       }
    else
       {
        pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
        plP_setpxl(pls->xdpi/25.4,pls->ydpi/25.4); /* Convert DPI to pixels/mm */
       } 

    plP_setphy(0, dev->vgax, 0, dev->vgay);

    dev->gnusvgaline.lno_width=pls->width;
    dev->gnusvgaline.lno_pattlen=0;

}



/*----------------------------------------------------------------------*\
 * plD_esc_bmp()
 *
 * Escape function.
\*----------------------------------------------------------------------*/

void plD_esc_bmp(PLStream *pls, PLINT op, void *ptr)
{
    switch (op) {

      case PLESC_FILL:  /* fill */
	fill_polygon(pls);
	break;
    }
}


/*----------------------------------------------------------------------*\
 * plD_eop_bmp()
 *
 * End of page.
\*----------------------------------------------------------------------*/

void plD_eop_bmp(PLStream *pls)
{

    if (page_state == DIRTY) 
       {
  
        gnusvga_get_a_file_name(pls);

        if ( (strncasecmp(pls->FileName,"auto",4)==0) )
           {
            GrSaveBmpImage(newname("bmp"), NULL, 0, 0, GrScreenX(), GrScreenY());
           }
       else
           {
            gnusvga_expand_BaseName(pls);
            GrSaveBmpImage(pls->FileName, NULL, 0, 0, GrScreenX(), GrScreenY());
           }
      }


    page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
 * plD_bop_bmp()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void plD_bop_bmp(PLStream *pls)
{
    pls->page++;
    plD_eop_bmp(pls);
    GrClearScreen(0);
}

/*----------------------------------------------------------------------*\
 * plD_tidy_bmp()
 *
 * Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void plD_tidy_bmp(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

      if (dev->Old_Driver_Vector) 
         {
          GrSetDriver (dev->Old_Driver_Vector->name);
          dev->Old_Driver_Vector=NULL;
         }
free(pls->dev);
pls->dev=NULL;
}
#endif


#else
int 
pldummy_gnusvga()
{
    return 0;
}

#endif				/* GNUSVGA */
