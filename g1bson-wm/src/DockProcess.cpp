/* $Id: DockProcess.cpp,v 1.3 2007/01/07 05:00:56 whot Exp $ */

#include "DockProcess.h"
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

DockProcess::DockProcess(XConn* x11, WMWindow* client, Dock* dock)
{
    this->x11 = x11;
    this->client = client;
    this->dock = dock;
}

void DockProcess::setup()
{
    cairo_surface_t* surface = 
        cairo_xlib_surface_create(x11->dpy, backbuff, x11->vis, width, height);
    cairo_t* cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_scale(cr, (double)width/x11->width, (double)height/x11->height);
    cairo_move_to(cr, client->getX(), client->getY());
    cairo_rel_line_to(cr, client->getWidth(), 0);
    cairo_rel_line_to(cr, 0, client->getHeight());
    cairo_rel_line_to(cr, -client->getWidth(), 0);
    cairo_rel_line_to(cr, 0, -client->getHeight());
    cairo_set_line_width(cr, 20);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);
    XCopyArea(x11->dpy, backbuff, button, button_gc, 0, 0, width, height, 0, 0);
    XFlush(x11->dpy);
}

void DockProcess::handleButtonEvent(PointerDevice* ptr, XIDeviceEvent* ev)
{
    client->setMinimize(false);
    dock->removeProcess(this);
}

