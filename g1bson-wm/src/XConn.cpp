/* $Id: XConn.cpp,v 1.4 2007/01/07 05:00:56 whot Exp $ */

#include "XConn.h"
#include "XError.h"

extern "C" {
    extern int _XiGetDevicePresenceNotifyEvent(Display*);
}

/*
 * Connect to the given X Server and get all default settings.
 */
XConn::XConn(char* host) 
{
    dpy = XOpenDisplay(host);
    if (!dpy)
        throw XError(XError::NO_DISPLAY);
    getDefaults();
}


/**
 * Get most used default settings and fill member variables.
 */
void XConn::getDefaults()
{
    screen = DefaultScreen(dpy);
    vis = DefaultVisual(dpy, screen);
    black = BlackPixel(dpy, screen);
    white = WhitePixel(dpy, screen);
    root = RootWindow(dpy, screen);
    depth = DefaultDepth(dpy, screen);
    cmap = DefaultColormap(dpy, screen);
    width = DisplayWidth(dpy, screen);
    height = DisplayHeight(dpy, screen);

    wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", false);
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", false);

    DevicePresence(dpy, XI_presence, presence_class);
    XI_presence = _XiGetDevicePresenceNotifyEvent(dpy);
}


XConn::~XConn()
{
    XCloseDisplay(dpy);
}
