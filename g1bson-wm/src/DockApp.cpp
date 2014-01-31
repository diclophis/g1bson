/* $Id: DockApp.cpp,v 1.5 2007/01/07 05:00:56 whot Exp $ */

#include "DockApp.h"
#include "logger.h"
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <stdlib.h>

DockApp::DockApp(XConn* x11, const char* app, char* imgfile)
{
    this->x11 = x11;
    this->app = app;
    this->imgfile = imgfile;

}

void DockApp::setup()
{
    cairo_surface_t* png = 
        cairo_image_surface_create_from_png(imgfile);
    cairo_surface_t* button = 
       cairo_xlib_surface_create(x11->dpy, backbuff, x11->vis, width, height);

    cairo_t* cr = cairo_create(button);
    cairo_set_source_surface(cr, png, 0, 0);
    cairo_paint(cr);
    if (cairo_status(cr))
    {
        ERR("cairo: %s\n", cairo_status_to_string(cairo_status(cr)));
    }
    cairo_surface_destroy(button);
    cairo_surface_destroy(png);
    cairo_destroy(cr);
    XFlush(x11->dpy);
}

void DockApp::handleButtonEvent(PointerDevice* ptr, XIDeviceEvent* ev)
{
    system(app);
}
