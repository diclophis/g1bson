
#include "DockKeyboard.h"
#include "logger.h"
#include "Config.h"
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

DockKeyboard::DockKeyboard(XConn* x11, KeyboardDevice* kbd)
{
    this->kbd = kbd;
    this->x11 = x11;
}

void DockKeyboard::setup()
{
    cairo_surface_t* png = 
        cairo_image_surface_create_from_png(Config::getInstance()->kbdImage);
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

void DockKeyboard::handleButtonEvent(PointerDevice* ptr, 
                                     XIDeviceEvent* ev)
{
    TRACE("Pairing %s with %s\n", ptr->getName().c_str(), kbd->getName().c_str());
    //XChangePointerKeyboardPairing(x11->dpy, ptr->getDevice(), kbd->getDevice());
    //kbd->setPaired(ptr);
}
