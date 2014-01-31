/* $Id: DockItem.cpp,v 1.4 2007/01/07 05:00:56 whot Exp $ */

#include "DockItem.h"
#include "logger.h"
#include <X11/Xutil.h>
#include <cairo/cairo-xlib.h>
#include <cairo/cairo.h>

void DockItem::handleExpose(XExposeEvent* ev)
{
    XCopyArea(x11->dpy, backbuff, button, button_gc, 0, 0, width, height, 0, 0);
}

void DockItem::initGUI(Window parent, int x, int y, int width, int height)
{
    this->width = width;
    this->height = height;
    button = XCreateSimpleWindow(x11->dpy, parent, x, y, width, height, 0, 0,
            x11->white); 

    XGCValues gcvals;
    button_gc = XCreateGC(x11->dpy, button, 0, &gcvals);

    backbuff = XCreatePixmap(x11->dpy, button, width, height, x11->depth);
    XFlush(x11->dpy);


    cairo_surface_t* surface = 
       cairo_xlib_surface_create(x11->dpy, backbuff, x11->vis, width, height);
    cairo_t* cr = cairo_create(surface);
    cairo_surface_destroy(surface);

    cairo_set_source_rgb(cr, 0.88, 0.88, 1);
    cairo_paint(cr);
    cairo_set_line_width(cr, 10);
    cairo_destroy(cr);

    XCopyArea(x11->dpy, backbuff, button, button_gc, 0, 0, width, height, 0, 0);
    
    XSelectInput(x11->dpy, button, ButtonPressMask | ExposureMask);
    XMapWindow(x11->dpy, button);
    TRACE("DockItem mapped on window %x\n", (unsigned int)button);
    XFlush(x11->dpy);
}


bool DockItem::hasWindow(Window win)
{
    return (button == win);
}

DockItem::~DockItem() {
    XFreeGC(x11->dpy, button_gc);
    XDestroyWindow(x11->dpy, button);
}


void DockItem::move(int x, int y)
{
    XMoveWindow(x11->dpy, button, x, y);
}
