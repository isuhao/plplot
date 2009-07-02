/*

  This software is provided under the LGPL in March 2009 by the
  Cluster Science Centre
  QSAS team,
  Imperial College, London

  Copyright (C) 2009  Imperial College, London
  Copyright (C) 2009  Alan W. Irwin

  This is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Lesser Public License as published
  by the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  To received a copy of the GNU Library General Public License
  write to the Free Software Foundation, Inc., 
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
  
  History:

  
  March 2009:  v1.00
  Initial release.
  
*/


#include "qt.h"
#include <QMutexLocker>

extern int vectorize;
extern MasterHandler handler;

// global variables initialised in init(), used in tidy()
// QApplication* app=NULL;
static int argc; // argc and argv have to exist when tidy() is used, thus they are made global
static char **argv;
static int appCounter = 0; // to be rigorous, all uses should be placed between mutexes

// Drivers declaration
PLDLLIMPEXP_DRIVER const char* plD_DEVICE_INFO_qt = 
#if defined(PLD_bmpqt)
    "bmpqt:Qt Windows bitmap driver:0:qt:66:bmpqt\n"
#endif
#if defined(PLD_jpgqt)
    "jpgqt:Qt jpg driver:0:qt:67:jpgqt\n"
#endif
#if defined(PLD_pngqt)
    "pngqt:Qt png driver:0:qt:68:pngqt\n"
#endif
#if defined(PLD_ppmqt)
    "ppmqt:Qt ppm driver:0:qt:69:ppmqt\n"
#endif
#if defined(PLD_tiffqt)
    "tiffqt:Qt tiff driver:0:qt:70:tiffqt\n"
#endif
#if defined(PLD_svgqt) && QT_VERSION >= 0x040300
    "svgqt:Qt SVG driver:0:qt:71:svgqt\n"
#endif
#if defined(PLD_qtwidget)
    "qtwidget:Qt Widget:1:qt:72:qtwidget\n"
#endif
#if defined(PLD_epsqt)
    "epsqt:Qt EPS driver:0:qt:73:epsqt\n"
#endif
#if defined(PLD_pdfqt)
    "pdfqt:Qt PDF driver:0:qt:74:pdfqt\n"
#endif
#if defined(PLD_extqt)
    "extqt:External Qt driver:0:qt:75:extqt\n"
#endif
    ;

static DrvOpt qt_options[]={	{"text_vectorize", DRV_INT, &vectorize, "Vectorize fonts on output (0|1)"},
				{NULL, DRV_INT, NULL, NULL}};

bool initQtApp(bool isGUI)
{
  QMutexLocker locker(&QtPLDriver::mutex);
  bool res=false;
  ++appCounter;
  if(qApp==NULL && appCounter==1)
  {
    argc=1;
    argv=new char*[2];
    argv[0]=new char[10];
    argv[1]=new char[1];
    snprintf(argv[0], 10, "qt_driver");
    argv[1][0]='\0';
    new QApplication(argc, argv, isGUI);
    res=true;
  }
  return res;
}

void closeQtApp()
{
  QMutexLocker locker(&QtPLDriver::mutex);
  --appCounter;
  if(qApp!=NULL && appCounter==0)
  {
    delete qApp;
    delete[] argv[0];
    delete[] argv[1];
    delete[] argv;
    argv=NULL;
  }
}

/*---------------------------------------------------------------------
  qt_family_check ()
  
  support function to help supress more than one page if family file
  output not specified by the user  (e.g., with the -fam command-line option).
  Adapted directly from svg.c
  ---------------------------------------------------------------------*/
static int already_warned = 0;
static int qt_family_check(PLStream *pls)
{
  if (pls->family || pls->page == 1) 
  {
    return 0;
  }
  else 
  {
    if(! already_warned)
    {
      already_warned = 1;
      plwarn("All pages after the first skipped because family file output not specified.\n");
    }
    return 1;
  }
}

// Declaration of the driver-specific interface functions
#if defined (PLD_bmpqt) || defined(PLD_jpgqt) || defined (PLD_pngqt) || defined(PLD_ppmqt) || defined(PLD_tiffqt)
void plD_init_rasterqt(PLStream *);
void plD_eop_rasterqt(PLStream *);
#endif

#if defined (PLD_bmpqt)
void plD_dispatch_init_bmpqt(PLDispatchTable *pdt);
void plD_bop_bmpqt(PLStream *);
#endif

#if defined (PLD_jpgqt)
void plD_dispatch_init_jpgqt(PLDispatchTable *pdt);
void plD_bop_jpgqt(PLStream *);
#endif

#if defined (PLD_pngqt)
void plD_dispatch_init_pngqt(PLDispatchTable *pdt);
void plD_bop_pngqt(PLStream *);
#endif

#if defined (PLD_ppmqt)
void plD_dispatch_init_ppmqt(PLDispatchTable *pdt);
void plD_bop_ppmqt(PLStream *);
#endif

#if defined (PLD_tiffqt)
void plD_dispatch_init_tiffqt(PLDispatchTable *pdt);
void plD_bop_tiffqt(PLStream *);
#endif

#if defined(PLD_svgqt) && QT_VERSION >= 0x040300
void plD_dispatch_init_svgqt(PLDispatchTable *pdt);
void plD_init_svgqt(PLStream *);
void plD_bop_svgqt(PLStream *);
void plD_eop_svgqt(PLStream *);
#endif

#if defined(PLD_epsqt) || defined(PLD_pdfqt)
void plD_init_epspdfqt(PLStream *);
void plD_bop_epspdfqt_helper(PLStream *, int ifeps);
void plD_eop_epspdfqt(PLStream *);
#endif
#if defined(PLD_epsqt)
void plD_dispatch_init_epsqt(PLDispatchTable *pdt);
void plD_bop_epsqt(PLStream *);
#endif
#if defined(PLD_pdfqt)
void plD_dispatch_init_pdfqt(PLDispatchTable *pdt);
void plD_bop_pdfqt(PLStream *);
#endif

#if defined(PLD_qtwidget)
void plD_dispatch_init_qtwidget(PLDispatchTable *pdt);
void plD_init_qtwidget(PLStream *);
void plD_eop_qtwidget(PLStream *);
#endif

#if defined(PLD_extqt)
void plD_dispatch_init_extqt(PLDispatchTable *pdt);
void plD_init_extqt(PLStream *);
void plD_eop_extqt(PLStream *);
void plD_line_extqt(PLStream *, short, short, short, short);
void plD_polyline_extqt(PLStream *, short*, short*, PLINT);
void plD_tidy_extqt(PLStream *);
void plD_state_extqt(PLStream *, PLINT);
void plD_esc_extqt(PLStream *, PLINT, void*);
#endif

// Declaration of the generic interface functions

void plD_line_qt(PLStream *, short, short, short, short);
void plD_polyline_qt(PLStream *, short*, short*, PLINT);
void plD_bop_qt(PLStream *){}
void plD_tidy_qt(PLStream *);
void plD_state_qt(PLStream *, PLINT);
void plD_esc_qt(PLStream *, PLINT, void*);


// Generic driver interface

void plD_line_qt(PLStream * pls, short x1a, short y1a, short x2a, short y2a)
{
  QtPLDriver * widget=NULL;
	
  // We have to dynamic_cast to make sure the good virtual functions are called
#if defined (PLD_bmpqt) || defined(PLD_jpgqt) || defined (PLD_pngqt) || defined(PLD_ppmqt) || defined(PLD_tiffqt)
  if(widget==NULL) widget=dynamic_cast<QtRasterDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_svgqt) && QT_VERSION >= 0x040300
  if(widget==NULL) widget=dynamic_cast<QtSVGDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_epsqt) || defined(PLD_pdfqt)
  if(widget==NULL) widget=dynamic_cast<QtEPSDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_qtwidget) || defined(PLD_extqt)
  if(widget==NULL) widget=dynamic_cast<QtPLWidget*>((QWidget *) pls->dev);
#endif
  if(widget==NULL) return;
	
  widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
  widget->drawLine(x1a, y1a, x2a, y2a);
}

void plD_polyline_qt(PLStream *pls, short *xa, short *ya, PLINT npts)
{
  QtPLDriver * widget=NULL;

#if defined (PLD_bmpqt) || defined(PLD_jpgqt) || defined (PLD_pngqt) || defined(PLD_ppmqt) || defined(PLD_tiffqt)
  if(widget==NULL) widget=dynamic_cast<QtRasterDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_svgqt) && QT_VERSION >= 0x040300
  if(widget==NULL) widget=dynamic_cast<QtSVGDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_epsqt) || defined(PLD_pdfqt)
  if(widget==NULL) widget=dynamic_cast<QtEPSDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_qtwidget) || defined(PLD_extqt)
  if(widget==NULL) widget=dynamic_cast<QtPLWidget*>((QWidget *) pls->dev);
#endif
  if(widget==NULL) return;
	
  widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
  widget->drawPolyline(xa, ya, npts);
}

void plD_esc_qt(PLStream * pls, PLINT op, void* ptr)
{
  short *xa, *ya;
  PLINT i, j;
  QtPLDriver * widget=NULL;

#if defined (PLD_bmpqt) || defined(PLD_jpgqt) || defined (PLD_pngqt) || defined(PLD_ppmqt) || defined(PLD_tiffqt)
  if(widget==NULL) widget=dynamic_cast<QtRasterDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_svgqt) && QT_VERSION >= 0x040300
  if(widget==NULL) widget=dynamic_cast<QtSVGDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_epsqt) || defined(PLD_pdfqt)
  if(widget==NULL) widget=dynamic_cast<QtEPSDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_qtwidget) || defined(PLD_extqt)
  if(widget==NULL) widget=dynamic_cast<QtPLWidget*>((QWidget *) pls->dev);
#endif
  if(widget==NULL) return;
		    
  switch(op)
  {
    case PLESC_DASH:
      widget->setDashed(pls->nms, pls->mark, pls->space);
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      widget->drawPolyline(pls->dev_x, pls->dev_y, pls->dev_npts);
      widget->setSolid();
      break;
        
    case PLESC_FILL:
      xa=new short[pls->dev_npts];
      ya=new short[pls->dev_npts];
            
      for (i = 0; i < pls->dev_npts; i++)
      {
        xa[i] = pls->dev_x[i];
        ya[i] = pls->dev_y[i];
      }
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      widget->drawPolygon(xa, ya, pls->dev_npts);
            
      delete[] xa;
      delete[] ya;
      break;
      
    case PLESC_HAS_TEXT:
      /*$$ call the generic ProcessString function
	ProcessString( pls, (EscText *)ptr ); */
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      widget->drawText(pls, (EscText *)ptr);
      break;        
    
    default: break;
  }
}

void plD_state_qt(PLStream * pls, PLINT op)
{
  QtPLDriver * widget=NULL;
#if defined (PLD_bmpqt) || defined(PLD_jpgqt) || defined (PLD_pngqt) || defined(PLD_ppmqt) || defined(PLD_tiffqt)
  if(widget==NULL) widget=dynamic_cast<QtRasterDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_svgqt) && QT_VERSION >= 0x040300
  if(widget==NULL) widget=dynamic_cast<QtSVGDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_epsqt) || defined(PLD_pdfqt)
  if(widget==NULL) widget=dynamic_cast<QtEPSDevice*>((QtPLDriver *) pls->dev);
  if(widget!=NULL && qt_family_check(pls)) {return;} 
#endif
#if defined(PLD_qtwidget) || defined(PLD_extqt)
  if(widget==NULL) widget=dynamic_cast<QtPLWidget*>((QWidget *) pls->dev);
#endif
  if(widget==NULL) return;
    
  switch(op)
  {
    case PLSTATE_WIDTH:
      widget->setWidth(pls->width);
      break;
       
    case PLSTATE_COLOR0:
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      break;
			
    case PLSTATE_COLOR1:
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      break;

            
    default: break;
  }
}

void plD_tidy_qt(PLStream * pls)
{
  QtPLDriver * widget=NULL;
#if defined (PLD_bmpqt) || defined(PLD_jpgqt) || defined (PLD_pngqt) || defined(PLD_ppmqt) || defined(PLD_tiffqt)
  if(widget==NULL) widget=dynamic_cast<QtRasterDevice*>((QtPLDriver *) pls->dev);
#endif
#if defined(PLD_svgqt) && QT_VERSION >= 0x040300
  if(widget==NULL) widget=dynamic_cast<QtSVGDevice*>((QtPLDriver *) pls->dev);
#endif
#if defined(PLD_epsqt) || defined(PLD_pdfqt)
  if(widget==NULL) widget=dynamic_cast<QtEPSDevice*>((QtPLDriver *) pls->dev);
#endif
#if defined(PLD_qtwidget) || defined(PLD_extqt)
  if(widget==NULL) widget=dynamic_cast<QtPLWidget*>((QtPLWidget *) pls->dev);
#endif
	
  if(widget!=NULL)
  {
    handler.DeviceClosed(widget);
    delete widget;
    pls->dev=NULL;
  }
	
  closeQtApp();
}

////////////////// Raster driver-specific definitions: class and interface functions /////////
#if defined (PLD_bmpqt) || defined(PLD_jpgqt) || defined (PLD_pngqt) || defined(PLD_ppmqt) || defined(PLD_tiffqt)
void plD_init_rasterqt(PLStream * pls)
{
  double dpi;
	
  vectorize=0;
  plParseDrvOpts(qt_options);
	
  /* Stream setup */
  pls->color = 1;
  pls->plbuf_write=0;
  pls->dev_fill0 = 1;
  pls->dev_fill1 = 0;
  /* Let the PLplot core handle dashed lines since
   * the driver results for this capability have a number of issues.
   pls->dev_dash=1; */
  pls->dev_dash=0;
  pls->dev_flush=1;
  /* Driver does not have a clear capability so use (good) PLplot core
   * fallback for that instead.  */
  pls->dev_clear=0;
  pls->termin=0;
  pls->page = 0;
  pls->dev_text = 1; // want to draw text
  pls->dev_unicode = 1; // want unicode 
  
  bool isMaster=initQtApp(/*false*/true); // Is it still a problem?
	
  if (pls->xdpi <=0.)
    dpi = DEFAULT_DPI;
  else
    dpi = pls->xdpi;
  // For raster debug
  // std::cout << dpi << std::endl;

  // Shamelessly copied on the Cairo stuff :)
  if (pls->xlength <= 0 || pls->ylength <= 0)
  {
    pls->dev=new QtRasterDevice;
    pls->xlength = (PLINT)(((QtRasterDevice*)(pls->dev))->m_dWidth);
    pls->ylength = (PLINT)(((QtRasterDevice*)(pls->dev))->m_dHeight);
  }
  else
  {
    pls->dev=new QtRasterDevice(pls->xlength, pls->ylength);
  }

  if(isMaster) handler.setMasterDevice((QtRasterDevice*)(pls->dev));

  if (pls->xlength > pls->ylength)
    ((QtRasterDevice*)(pls->dev))->downscale = (PLFLT)pls->xlength/(PLFLT)(PIXELS_X-1);
  else
    ((QtRasterDevice*)(pls->dev))->downscale = (PLFLT)pls->ylength/(PLFLT)PIXELS_Y;
	
  plP_setphy((PLINT) 0, (PLINT) (pls->xlength / ((QtRasterDevice*)(pls->dev))->downscale), (PLINT) 0, (PLINT) (pls->ylength / ((QtRasterDevice*)(pls->dev))->downscale));
	
  plP_setpxl(dpi/25.4/((QtRasterDevice*)(pls->dev))->downscale, dpi/25.4/((QtRasterDevice*)(pls->dev))->downscale);
	
  ((QtRasterDevice*)(pls->dev))->setResolution(dpi);

  /* Initialize family file info */
  plFamInit(pls);
	
  plOpenFile(pls);
}

void plD_eop_rasterqt(PLStream *pls)
{
  if(qt_family_check(pls)) {return;} 
  ((QtRasterDevice *)pls->dev)->savePlot();
  handler.DeviceChangedPage((QtRasterDevice *)pls->dev);
}
#endif

#if defined(PLD_bmpqt)
void plD_dispatch_init_bmpqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt Windows bitmap Driver";
  pdt->pl_DevName  = "bmpqt";
#endif
  pdt->pl_type     = plDevType_FileOriented;
  pdt->pl_seq      = 66;
  pdt->pl_init     = (plD_init_fp)     plD_init_rasterqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_rasterqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_bmpqt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}

void plD_bop_bmpqt(PLStream *pls)
{
  /* Plot familying stuff. Not really understood, just copying gd.c */
  plGetFam(pls);

  pls->famadv = 1;
  pls->page++;
  if(qt_family_check(pls)) {return;} 
  ((QtRasterDevice *)pls->dev)->definePlotName(pls->FileName, "BMP");
}
#endif

#if defined(PLD_jpgqt)
void plD_dispatch_init_jpgqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt jpg Driver";
  pdt->pl_DevName  = "jpgqt";
#endif
  pdt->pl_type     = plDevType_FileOriented;
  pdt->pl_seq      = 67;
  pdt->pl_init     = (plD_init_fp)     plD_init_rasterqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_rasterqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_jpgqt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}

void plD_bop_jpgqt(PLStream *pls)
{
  /* Plot familying stuff. Not really understood, just copying gd.c */
  plGetFam(pls);

  pls->famadv = 1;
  pls->page++;
  if(qt_family_check(pls)) {return;} 
  ((QtRasterDevice *)pls->dev)->definePlotName(pls->FileName, "JPG");
}
#endif

#if defined(PLD_pngqt)
void plD_dispatch_init_pngqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt png Driver";
  pdt->pl_DevName  = "pngqt";
#endif
  pdt->pl_type     = plDevType_FileOriented;
  pdt->pl_seq      = 68;
  pdt->pl_init     = (plD_init_fp)     plD_init_rasterqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_rasterqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_pngqt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}

void plD_bop_pngqt(PLStream *pls)
{
  /* Plot familying stuff. Not really understood, just copying gd.c */
  plGetFam(pls);

  pls->famadv = 1;
  pls->page++;
  if(qt_family_check(pls)) {return;} 
  ((QtRasterDevice *)pls->dev)->definePlotName(pls->FileName, "PNG");
}
#endif

#if defined(PLD_ppmqt)
void plD_dispatch_init_ppmqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt ppm Driver";
  pdt->pl_DevName  = "ppmqt";
#endif
  pdt->pl_type     = plDevType_FileOriented;
  pdt->pl_seq      = 69;
  pdt->pl_init     = (plD_init_fp)     plD_init_rasterqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_rasterqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_ppmqt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}

void plD_bop_ppmqt(PLStream *pls)
{
  /* Plot familying stuff. Not really understood, just copying gd.c */
  plGetFam(pls);

  pls->famadv = 1;
  pls->page++;
  if(qt_family_check(pls)) {return;} 
  ((QtRasterDevice *)pls->dev)->definePlotName(pls->FileName, "PPM");
}
#endif

#if defined(PLD_tiffqt)
void plD_dispatch_init_tiffqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt tiff Driver";
  pdt->pl_DevName  = "tiffqt";
#endif
  pdt->pl_type     = plDevType_FileOriented;
  pdt->pl_seq      = 70;
  pdt->pl_init     = (plD_init_fp)     plD_init_rasterqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_rasterqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_tiffqt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}

void plD_bop_tiffqt(PLStream *pls)
{
  /* Plot familying stuff. Not really understood, just copying gd.c */
  plGetFam(pls);

  pls->famadv = 1;
  pls->page++;
  if(qt_family_check(pls)) {return;} 
  ((QtRasterDevice *)pls->dev)->definePlotName(pls->FileName, "TIFF");
}
#endif

#if defined(PLD_svgqt) && QT_VERSION >= 0x040300
void plD_dispatch_init_svgqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt SVG Driver";
  pdt->pl_DevName  = "svgqt";
#endif
  pdt->pl_type     = plDevType_FileOriented;
  pdt->pl_seq      = 71;
  pdt->pl_init     = (plD_init_fp)     plD_init_svgqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_svgqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_svgqt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}

void plD_init_svgqt(PLStream * pls)
{
  vectorize=1;
  plParseDrvOpts(qt_options);

  /* Stream setup */
  pls->color = 1;
  pls->plbuf_write=0;
  pls->dev_fill0 = 1;
  pls->dev_fill1 = 0;
  /* Let the PLplot core handle dashed lines since
   * the driver results for this capability have a number of issues.
   pls->dev_dash=1; */
  pls->dev_dash=0;
  pls->dev_flush=1;
  /* Driver does not have a clear capability so use (good) PLplot core
   * fallback for that instead.  */
  pls->dev_clear=0;
  pls->termin=0;
  pls->page = 0;
  pls->dev_text = 1; // want to draw text
  pls->dev_unicode = 1; // want unicode 
	
  bool isMaster=initQtApp(/*false*/ true); // Is is still a problem?
	
  if (pls->xlength <= 0 || pls->ylength <= 0)
  {
    pls->dev=new QtSVGDevice;
    pls->xlength = (int)(((QtSVGDevice*)(pls->dev))->m_dWidth);
    pls->ylength = (int)(((QtSVGDevice*)(pls->dev))->m_dHeight);
  }
  else
  {
    pls->dev=new QtSVGDevice(pls->xlength, pls->ylength);
  }

  if(isMaster) handler.setMasterDevice((QtSVGDevice*)(pls->dev));

  if (pls->xlength > pls->ylength)
    ((QtSVGDevice*)(pls->dev))->downscale = (PLFLT)pls->xlength/(PLFLT)(PIXELS_X-1);
  else
    ((QtSVGDevice*)(pls->dev))->downscale = (PLFLT)pls->ylength/(PLFLT)PIXELS_Y;
	
  plP_setphy((PLINT) 0, (PLINT) (pls->xlength / ((QtSVGDevice*)(pls->dev))->downscale), (PLINT) 0, (PLINT) (pls->ylength / ((QtSVGDevice*)(pls->dev))->downscale));
	
  plP_setpxl(POINTS_PER_INCH/25.4/((QtSVGDevice*)(pls->dev))->downscale, POINTS_PER_INCH/25.4/((QtSVGDevice*)(pls->dev))->downscale);

  /* Initialize family file info */
  plFamInit(pls);
  
  plOpenFile(pls);
}

void plD_bop_svgqt(PLStream *pls)
{
  /* Plot familying stuff. Not really understood, just copying gd.c */
  plGetFam(pls);
	
  pls->famadv = 1;
  pls->page++;
  if(qt_family_check(pls)) {return;} 
  ((QtSVGDevice *)pls->dev)->definePlotName(pls->FileName);
}

void plD_eop_svgqt(PLStream *pls)
{
  double downscale;
  QSize s;
	
  ((QtSVGDevice *)pls->dev)->savePlot();
  // Once saved, we have to create a new device with the same properties
  // to be able to plot another page.
  downscale=((QtSVGDevice *)pls->dev)->downscale;
  s=((QtSVGDevice *)pls->dev)->size();
  bool isMaster=(handler.isMasterDevice((QtSVGDevice *)pls->dev));
  delete ((QtSVGDevice *)pls->dev);
	
  pls->dev=new QtSVGDevice(s.width(), s.height());
  ((QtSVGDevice *)pls->dev)->downscale=downscale;
	
  if(isMaster) handler.setMasterDevice((QtSVGDevice *)pls->dev);
  handler.DeviceChangedPage((QtSVGDevice *)pls->dev);
}
#endif

#if defined(PLD_epsqt)
void plD_dispatch_init_epsqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt EPS Driver";
  pdt->pl_DevName  = "epsqt";
#endif
  pdt->pl_type     = plDevType_FileOriented;
  pdt->pl_seq      = 73;
  pdt->pl_init     = (plD_init_fp)     plD_init_epspdfqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_epspdfqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_epsqt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}
#endif

#if defined(PLD_pdfqt)
void plD_dispatch_init_pdfqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt PDF Driver";
  pdt->pl_DevName  = "pdfqt";
#endif
  pdt->pl_type     = plDevType_FileOriented;
  pdt->pl_seq      = 74;
  pdt->pl_init     = (plD_init_fp)     plD_init_epspdfqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_epspdfqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_pdfqt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}
#endif

#if defined(PLD_epsqt) || defined(PLD_pdfqt)
void plD_init_epspdfqt(PLStream * pls)
{
  vectorize=0;
  plParseDrvOpts(qt_options);
	
  /* Stream setup */
  pls->color = 1;
  pls->plbuf_write=0;
  pls->dev_fill0 = 1;
  pls->dev_fill1 = 0;
  /* Let the PLplot core handle dashed lines since
   * the driver results for this capability have a number of issues.
   pls->dev_dash=1; */
  pls->dev_dash=0;
  pls->dev_flush=1;
  /* Driver does not have a clear capability so use (good) PLplot core
   * fallback for that instead.  */
  pls->dev_clear=0;
  pls->termin=0;
  pls->page = 0;
  pls->dev_text = 1; // want to draw text
  pls->dev_unicode = 1; // want unicode 
	
  // QPrinter devices won't create if there is no QApplication declared...
  bool isMaster=initQtApp(/*false*/true); //Is it still a problem?
	
  if (pls->xlength <= 0 || pls->ylength <= 0)
  {
    pls->dev=new QtEPSDevice;
    pls->xlength = (int)(((QtEPSDevice*)(pls->dev))->m_dWidth);
    pls->ylength = (int)(((QtEPSDevice*)(pls->dev))->m_dHeight);
  }
  else
  {
    pls->dev=new QtEPSDevice(pls->xlength, pls->ylength);
  }

  if(isMaster) handler.setMasterDevice((QtEPSDevice*)(pls->dev));

  if (pls->xlength > pls->ylength)
    ((QtEPSDevice*)(pls->dev))->downscale = (PLFLT)pls->xlength/(PLFLT)(PIXELS_X-1);
  else
    ((QtEPSDevice*)(pls->dev))->downscale = (PLFLT)pls->ylength/(PLFLT)PIXELS_Y;
	
  plP_setphy((PLINT) 0, (PLINT) (pls->xlength / ((QtEPSDevice*)(pls->dev))->downscale), (PLINT) 0, (PLINT) (pls->ylength / ((QtEPSDevice*)(pls->dev))->downscale));
	
  plP_setpxl(POINTS_PER_INCH/25.4/((QtEPSDevice*)(pls->dev))->downscale, POINTS_PER_INCH/25.4/((QtEPSDevice*)(pls->dev))->downscale);
	
  /* Initialize family file info */
  plFamInit(pls);
	
  plOpenFile(pls);
}

void plD_bop_epspdfqt_helper(PLStream *pls, int ifeps)
{
  /* Plot familying stuff. Not really understood, just copying gd.c */
  plGetFam(pls);

  pls->famadv = 1;
  pls->page++;
  if(qt_family_check(pls)) {return;}
  ((QtEPSDevice *)pls->dev)->definePlotName(pls->FileName, ifeps);
}

void plD_eop_epspdfqt(PLStream *pls)
{
  double downscale;

  int argc=0;
  char argv[]={'\0'};
  if(qt_family_check(pls)) {return;} 
  ((QtEPSDevice *)pls->dev)->savePlot();
  // Once saved, we have to create a new device with the same properties
  // to be able to plot another page.
  downscale=((QtEPSDevice *)pls->dev)->downscale;
  bool isMaster=handler.isMasterDevice((QtEPSDevice *)pls->dev);
  delete ((QtEPSDevice *)pls->dev);

  pls->dev=new QtEPSDevice;
  ((QtEPSDevice *)pls->dev)->downscale=downscale;

  if(isMaster) handler.setMasterDevice((QtEPSDevice *)pls->dev);
  handler.DeviceChangedPage((QtEPSDevice *)pls->dev);
}
#endif

#if defined(PLD_epsqt)
void plD_bop_epsqt(PLStream *pls)
{
  plD_bop_epspdfqt_helper(pls, 1);
}
#endif

#if defined(PLD_pdfqt)
void plD_bop_pdfqt(PLStream *pls)
{
  plD_bop_epspdfqt_helper(pls, 0);
}
#endif

#if defined (PLD_qtwidget)
void plD_dispatch_init_qtwidget(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Qt Widget";
  pdt->pl_DevName  = "qtwidget";
#endif
  pdt->pl_type     = plDevType_Interactive;
  pdt->pl_seq      = 72;
  pdt->pl_init     = (plD_init_fp)     plD_init_qtwidget;
  pdt->pl_line     = (plD_line_fp)     plD_line_qt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_qt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_qtwidget;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_qt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_qt;
  pdt->pl_state    = (plD_state_fp)    plD_state_qt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_qt;
}

void plD_init_qtwidget(PLStream * pls)
{
  vectorize=0;
  plParseDrvOpts(qt_options);
	
  PLINT w, h;
  bool isMaster=initQtApp(true);
  QtPLWidget* widget;

  if (pls->xlength <= 0 || pls->ylength <= 0)
  {
    widget=new QtPLWidget;
    pls->dev=(void*) widget;
    pls->xlength = (int)widget->m_dWidth;
    pls->ylength = (int)widget->m_dHeight;
  }
  else
  {
    widget=new QtPLWidget(pls->xlength, pls->ylength);
    pls->dev=(void*) widget;
  }

  if(isMaster) handler.setMasterDevice(widget);	

  if (plsc->xlength > plsc->ylength)
    widget->downscale = (PLFLT)plsc->xlength/(PLFLT)(PIXELS_X-1);
  else
    widget->downscale = (PLFLT)plsc->ylength/(PLFLT)PIXELS_Y;
	
  plP_setphy((PLINT) 0, (PLINT) (plsc->xlength / widget->downscale), (PLINT) 0, (PLINT) (plsc->ylength / widget->downscale));
	
  QPicture temp;
  QPainter tempPainter(&temp);
	
  plP_setpxl(temp.logicalDpiX()/25.4/widget->downscale, temp.logicalDpiY()/25.4/widget->downscale);

  pls->color = 1;		/* Is a color device */
  pls->plbuf_write=0;
  pls->dev_fill0 = 1;	/* Handle solid fills */
  pls->dev_fill1 = 0;
  /* Let the PLplot core handle dashed lines since
   * the driver results for this capability have a number of issues.
   pls->dev_dash=1; */
  pls->dev_dash=0;
  pls->dev_flush=1;
  /* Driver does not have a clear capability so use (good) PLplot core
   * fallback for that instead.  */
  pls->dev_clear=0;
  // 	pls->termin=1;
  pls->dev_text = 1; // want to draw text
  pls->dev_unicode = 1; // want unicode 
	
  widget->setVisible(true);
  widget->resize(plsc->xlength, plsc->ylength);

  qApp->connect(&handler, SIGNAL(MasterChangedPage()), widget, SLOT(nextPage()));
  qApp->connect(&handler, SIGNAL(MasterClosed()), widget, SLOT(close()));
	
}

void plD_eop_qtwidget(PLStream *pls)
{
  QtPLWidget* widget=((QtPLWidget*)pls->dev);
  int currentPage=widget->pageNumber;
  while(currentPage==widget->pageNumber && handler.isMasterDevice(widget) && ! pls->nopause)
  {
    qApp->processEvents(QEventLoop::WaitForMoreEvents);
  }
}
#endif

#if defined(PLD_extqt)
void plD_dispatch_init_extqt(PLDispatchTable *pdt)
{
#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "External Qt Widget";
  pdt->pl_DevName  = "extqt";
#endif
  pdt->pl_type     = plDevType_Interactive;
  pdt->pl_seq      = 75;
  pdt->pl_init     = (plD_init_fp)     plD_init_extqt;
  pdt->pl_line     = (plD_line_fp)     plD_line_extqt;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_extqt;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_extqt;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_qt;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_extqt;
  pdt->pl_state    = (plD_state_fp)    plD_state_extqt;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_extqt;
}

void plD_init_extqt(PLStream * pls)
{
  vectorize=0;
  plParseDrvOpts(qt_options);
	
  if(pls->dev==NULL/* || pls->xlength <= 0 || pls->ylength <= 0*/)
  {
    printf("Error: use plsetqtdev to set up the Qt device before calling plinit()\n");
    return;
  }

  QtExtWidget* widget=(QtExtWidget*)(pls->dev);

  if (widget->m_dWidth > widget->m_dHeight)
    widget->downscale = (PLFLT)widget->m_dWidth/(PLFLT)(PIXELS_X-1);
  else
    widget->downscale = (PLFLT)widget->m_dHeight/(PLFLT)PIXELS_Y;
	
  plP_setphy((PLINT) 0, (PLINT) (widget->m_dWidth / widget->downscale), (PLINT) 0, (PLINT) (widget->m_dHeight / widget->downscale));
	
  QPicture temp;
  QPainter tempPainter(&temp);
	
  plP_setpxl(temp.logicalDpiX()/25.4/widget->downscale, temp.logicalDpiY()/25.4/widget->downscale);

  pls->color = 1;		/* Is a color device */
  pls->plbuf_write=0;
  pls->dev_fill0 = 1;	/* Handle solid fills */
  pls->dev_fill1 = 0;
  /* Let the PLplot core handle dashed lines since
   * the driver results for this capability have a number of issues.
   pls->dev_dash=1; */
  pls->dev_dash=0;
  pls->dev_flush=1;
  /* Driver does not have a clear capability so use (good) PLplot core
   * fallback for that instead.  */
  pls->dev_clear=0;
  pls->dev_text = 1; // want to draw text
  pls->dev_unicode = 1; // want unicode 
}

/*
  These functions are separated out (instead of using dynamic_cast)
  for the benefit of the PyQt4 bindings. C++ QtExtWidgets created
  by PyQt4 are not properly type resolved. 
 */

void plD_line_extqt(PLStream * pls, short x1a, short y1a, short x2a, short y2a)
{
  QtExtWidget * widget=NULL;

  widget=(QtExtWidget*)pls->dev;
  widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
  widget->drawLine(x1a, y1a, x2a, y2a);
}

void plD_polyline_extqt(PLStream *pls, short *xa, short *ya, PLINT npts)
{
  QtExtWidget * widget=NULL;

  widget=(QtExtWidget*)pls->dev;
  widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
  widget->drawPolyline(xa, ya, npts);
}

void plD_esc_extqt(PLStream * pls, PLINT op, void* ptr)
{
  short *xa, *ya;
  PLINT i, j;
  QtExtWidget * widget=NULL;

  widget=(QtExtWidget*)pls->dev;
  switch(op)
  {
    case PLESC_DASH:
      widget->setDashed(pls->nms, pls->mark, pls->space);
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      widget->drawPolyline(pls->dev_x, pls->dev_y, pls->dev_npts);
      widget->setSolid();
      break;
        
    case PLESC_FILL:
      xa=new short[pls->dev_npts];
      ya=new short[pls->dev_npts];
            
      for (i = 0; i < pls->dev_npts; i++)
      {
        xa[i] = pls->dev_x[i];
        ya[i] = pls->dev_y[i];
      }
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      widget->drawPolygon(xa, ya, pls->dev_npts);
            
      delete[] xa;
      delete[] ya;
      break;
      
    case PLESC_HAS_TEXT:
      /*$$ call the generic ProcessString function
	ProcessString( pls, (EscText *)ptr ); */
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      widget->drawText(pls, (EscText *)ptr);
      break;        
    
    default: break;
  }
}

void plD_state_extqt(PLStream * pls, PLINT op)
{
  QtExtWidget * widget=NULL;

  widget=(QtExtWidget*)pls->dev;    
  switch(op)
  {
    case PLSTATE_WIDTH:
      widget->setWidth(pls->width);
      break;
       
    case PLSTATE_COLOR0:
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      break;
			
    case PLSTATE_COLOR1:
      widget->setColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b, pls->curcolor.a);
      break;

            
    default: break;
  }
}

void plD_tidy_extqt(PLStream * pls)
{
  QtExtWidget * widget=NULL;

  widget=(QtExtWidget*)pls->dev;
  if(widget!=NULL)
  {
    handler.DeviceClosed(widget);
    delete widget;
    pls->dev=NULL;
  }
	
  closeQtApp();
}

void plD_eop_extqt(PLStream *pls)
{
}
#endif
