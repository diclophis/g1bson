/*--
  --*/

#include "WMWindow.h"
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <math.h>

/**
 * errorwindow is set by a WMWindow on startup. If the call to
 * XGetWindowAttributes fails, a flag in the errorwindow is set and it raises
 * an exception on startup.
 */
WMWindow* errorwindow = NULL;

int X_GetWindowAttributesErrorHandler(Display *dpy, XErrorEvent*  err)
{
    ERR("XGetWindow Attributes failed on window %x\n", (unsigned int)err->resourceid);
    ERR("request number %d, error code %d\n", err->request_code, err->error_code);
    errorwindow->cancel();
    return 0; // is ignored anyway
}

WMWindow::WMWindow(Manager* manager, Window w, XConn* x11)
{
    XWindowAttributes attr;
    this->x11 = x11;
    this->manager = manager;
    this->exception = false;
    this->process = NULL;
    this->state = 0;
    this->minimized = false;
    this->resizing = false;
    this->clientOffset = Config::getInstance()->clientOffset;

    errorwindow = this;


    // set the error handler. If GetWindowAttributes fails (because the window
    // has been unmapped (firefox does that), then throw an exception and stop
    // the constructor.
    XSync(x11->dpy, False);
    XSetErrorHandler(X_GetWindowAttributesErrorHandler);
    XGetWindowAttributes(x11->dpy, w, &attr);
    XSync(x11->dpy, False);
    XSetErrorHandler(MPWM_ErrorHandler);
    errorwindow = NULL;

    if (exception)
        throw XError(XError::UNKNOWN_WINDOW);

    // everything should be nice and shiny here
    client = w;
    x = attr.x;
    y = attr.y;


    width = attr.width;
    height = attr.height;

    // boundary check
    x = (x < 0) ? 0 : x;
    y = (y < 0) ? 0 : y;

    x = (x > (x11->width - width)) ? x11->width - width : x; 
    y = (y > (x11->height - height)) ? x11->height - height : y; 

    state = attr.map_state;
    override_redirect = attr.override_redirect;

    controller = NULL;
    restrictedTo = NULL;

}

WMWindow::~WMWindow()
{
    TRACE("Deleting\n");
    // disable resizers
    vector<PointerDevice*> tmp = resizers;
    vector<PointerDevice*>::const_iterator it = tmp.begin();
    while(it != tmp.end())
    {
        (*it)->resizeOff();
        it++;
    }

    // disable movers

    if (controller)
        controller->dragOff();

    XDestroyWindow(x11->dpy, container);

    if (process != NULL)
        process->removeWindow(this);
}

/**
 * Check if the given window belongs to this particular WMWindow.
 * @param The X window to check.
 * @return true if the window is either decoration, container or the client
 * window.
 */
bool WMWindow::hasWindow(Window window)
{
    return (window == container || window == client || window == windowBar ||
            window == btClose || window == resizeBar ||
            window == btOwner || window == btMinimize
            || isResizeButton(window));
}

bool WMWindow::isWindowBar(Window window)
{
    return window == windowBar;
}
bool WMWindow::isButtonClose(Window window)
{
    return window == btClose;
}
bool WMWindow::isButtonMinimize(Window window)
{
    return window == btMinimize;
}

bool WMWindow::isContainer(Window window)
{
    return window == container;
}
bool WMWindow::isResizeButton(Window window)
{
    return (window == resizeBtNE || window == resizeBtNW || window ==
            resizeBtSE || window == resizeBtSW);
}

bool WMWindow::isResizeBar(Window window)
{
    return window == resizeBar;
}

bool WMWindow::isClientWindow(Window window)
{
    return (window == client);
}


bool WMWindow::onPosition(int x, int y)
{
   return (x >= this->x && x <= this->x + this->width && y >= this->y && 
           y <= this->y + this->height + 
           Config::getInstance()->windowBarHeight +
           Config::getInstance()->resizeBarHeight);
}


/**
 * Creates the window manager windows for this client. This is not done
 * automatically, instead on the client's first map request.
 */
void WMWindow::decorate()
{

    int btwidth, btheight;
    XGCValues vals;
    XSetWindowAttributes attr;

    XSelectInput(x11->dpy, client, ColormapChangeMask | EnterWindowMask |
            PropertyChangeMask | FocusChangeMask);
    XFlush(x11->dpy);

    TRACE("decorating window %x\n", (unsigned int)client);
    int totalHeight = height + Config::getInstance()->windowBarHeight +
        Config::getInstance()->resizeBarHeight;
    int totalWidth = width + clientOffset * 2;

    container = XCreateSimpleWindow(x11->dpy, x11->root, x, y, totalWidth, 
            totalHeight, 0, 0, x11->white);
    containerGC = XCreateGC(x11->dpy, container, 0, &vals);
    pxContainer = 0;

    TRACE("  - container %x\n", (unsigned int)container);

    windowBar = XCreateWindow(x11->dpy, container, 0, 0, totalWidth,
            Config::getInstance()->windowBarHeight, 0, 0, InputOnly,
            x11->vis, 0, &attr);

    TRACE("  - windowBar %x\n", (unsigned int)windowBar);

    btwidth = Config::getInstance()->buttonWidth;
    btheight = Config::getInstance()->buttonHeight;
    btClose = XCreateWindow(x11->dpy, windowBar, totalWidth -
            Config::getInstance()->buttonCloseX,
            Config::getInstance()->buttonCloseY, 
            btwidth, btheight, 0, 0, InputOnly, x11->vis, 0, &attr);
            
    TRACE("  - btClose %x\n", (unsigned int)btClose);

    btMinimize = XCreateWindow(x11->dpy, windowBar, totalWidth - 
            Config::getInstance()->buttonMinimizeX,
            Config::getInstance()->buttonMinimizeY, 
            btwidth, btheight, 0, 0, InputOnly, x11->vis, 0, &attr);

    TRACE("  - btMinimize %x\n", (unsigned int)btMinimize);

    btOwner = XCreateWindow(x11->dpy, windowBar, totalWidth -
            Config::getInstance()->buttonOwnerX,
            Config::getInstance()->buttonOwnerY, 
            btwidth, btheight, 0, 0, InputOnly, x11->vis, 0, &attr);

    TRACE("  - btOwner %x\n", (unsigned int)btOwner);


    resizeBar = XCreateWindow(x11->dpy, container, 0, 
            height + Config::getInstance()->windowBarHeight + 1,
            totalWidth, Config::getInstance()->resizeBarHeight,
            0, 0, InputOnly, x11->vis, 0, &attr);

    TRACE(" - resizeBar %x\n", (unsigned int)resizeBar);

    resizeBtNE = XCreateWindow(x11->dpy, windowBar, totalWidth -
            Config::getInstance()->resizeButtonWidth,
            0, Config::getInstance()->resizeButtonWidth,
            Config::getInstance()->resizeButtonHeight,
            0, 0, InputOnly, x11->vis, 0, &attr);

    resizeBtNW = XCreateWindow(x11->dpy, windowBar, 
            0, 0, Config::getInstance()->resizeButtonWidth,
            Config::getInstance()->resizeButtonHeight,
            0, 0, InputOnly, x11->vis, 0, &attr);

    resizeBtSE = XCreateWindow(x11->dpy, resizeBar, totalWidth -
            Config::getInstance()->resizeButtonWidth,
            0, Config::getInstance()->resizeButtonWidth,
            Config::getInstance()->resizeButtonHeight,
            0, 0, InputOnly, x11->vis, 0, &attr);

    resizeBtSW = XCreateWindow(x11->dpy, resizeBar, 
            0, 0, Config::getInstance()->resizeButtonWidth,
            Config::getInstance()->resizeButtonHeight,
            0, 0, InputOnly, x11->vis, 0, &attr);


    attr.override_redirect = False;
    XChangeWindowAttributes(x11->dpy, container, CWOverrideRedirect, &attr);
            
    XMapWindow(x11->dpy, btClose);
    XMapWindow(x11->dpy, btMinimize);
    XMapWindow(x11->dpy, btOwner);
    XMapWindow(x11->dpy, windowBar);
    XMapWindow(x11->dpy, resizeBar);
    XMapWindow(x11->dpy, resizeBtSE);
    XMapWindow(x11->dpy, resizeBtNE);
    XMapWindow(x11->dpy, resizeBtSW);
    XMapWindow(x11->dpy, resizeBtNW);
    paintWindowBar();
    XFlush(x11->dpy);
}

/**
 * Reconfigure the window to whatever the given event requires.
 */
void WMWindow::reconfigure(XConfigureRequestEvent* ev)
{
    // don't do anything while we're resizing
    if (resizing)
        return;

    if (ev->value_mask & CWWidth) 
        width = ev->width;
    if (ev->value_mask & CWHeight) 
        height = ev->height;
    if (ev->value_mask & CWX) 
        x = ev->x; 
    if (ev->value_mask & CWY) 
        y = ev->y;

    resize(width, height);

    if (ev->value_mask & (CWX | CWY))
        move(x, y);

    TRACE("  -  Configured client %x\n", (unsigned int)client);
    XFlush(x11->dpy);
}


/**
 * Reparents the window. Once this method is called, our WM has control over
 * it.
 */
void WMWindow::manage()
{
    XAddToSaveSet(x11->dpy, client);
    XSync(x11->dpy, False);
    XReparentWindow(x11->dpy, client, container, clientOffset, Config::getInstance()->windowBarHeight);

    XSelectInput(x11->dpy, container, SubstructureRedirectMask |
            SubstructureNotifyMask | ExposureMask);

    vector<PointerDevice*>* pointers = manager->getPointers();
    vector<PointerDevice*>::const_iterator it = pointers->begin();

    while(it != pointers->end())
    {
        (*it)->setWMEvents(this);
        it++;
    }

    XLowerWindow(x11->dpy, client);

    TRACE("Managing client %x\n", (unsigned int)client);
}

/**
 * Move the client window (including decoration) to the given location.
 */
void WMWindow::move(int x, int y)
{
    XMoveWindow(x11->dpy, container, x, y);
    this->x = x;
    this->y = y;

}

void WMWindow::mapAll()
{
    XMapWindow(x11->dpy, client);
    XMapWindow(x11->dpy, windowBar);
    XMapWindow(x11->dpy, resizeBar);
    XMapWindow(x11->dpy, btClose);
    XMapWindow(x11->dpy, btMinimize);
    XMapWindow(x11->dpy, btOwner);
    XMapWindow(x11->dpy, container);
    XMapWindow(x11->dpy, resizeBtSE);
    XMapWindow(x11->dpy, resizeBtNE);
    XMapWindow(x11->dpy, resizeBtSW);
    XMapWindow(x11->dpy, resizeBtNW);
    setState(IsViewable);
}

/**
 * Sets the controlling pointer for this window. Only one pointer can be
 * controlling pointer at one time.
 * @param dev The controlling device.
 */
bool WMWindow::setController(PointerDevice* dev)
{
    if (restrictedTo != NULL && dev != restrictedTo)
        return false;

    // only one device can be controlling pointer
    if (controller != NULL && controller != dev)
        return false;

    controller = dev;
    return true;
}

bool WMWindow::addResizer(PointerDevice* dev)
{
    if (restrictedTo != NULL && dev != restrictedTo)
        return false;


    vector<PointerDevice*>::const_iterator it = resizers.begin();

    while(it != resizers.end())
    {
        if ((*it) == dev)
            return true;

        it++;
    }
   
    resizers.push_back(dev);
    setResizeInProgress(true);
    return true;
}


void WMWindow::releaseResizer(PointerDevice* dev)
{
    vector<PointerDevice*>::iterator it = resizers.begin();

    while(it != resizers.end())
    {
        if ((*it)== dev)
        {
            resizers.erase(it);
            break;
        }
        it++;
    }

    if (resizers.empty())
        setResizeInProgress(false);
}

/**
 * Resize the window in a given direction. 
 */
void WMWindow::resizeDirected(Window button, int x, int y)
{

    if (button == resizeBtSE)
    {
        this->width += x;
        this->height += y;

    } else if (button == resizeBtSW)
    {
        this->x += x;
        this->height += y;
        this->width -= x;

    } else if (button == resizeBtNE)
    {
        this->y += y;
        this->width += x;
        this->height -= y;

    } else if (button == resizeBtNW)
    {
        this->x += x;
        this->y += y;
        this->width -= x;
        this->height -= y;
    }

    resizeAbsolute(this->x, this->y, this->width, this->height);
}

/**
 * Resizes the window according to all pointer's coordinates.  
 * @param x The delta x resize in pixels (narrower < 0 < wider).
 * @param y The delta y resize in pixels (shorter < 0 < taller).
 */
void WMWindow::resize(int width, int height)
{
    resizeAbsolute(-1, -1, width, height);
    paintWindowBar();
}

/**
 * Resizes and moves a window to the given size and position.
 */
void WMWindow::resizeAbsolute(int posx, int posy, int w, int h)
{
    XWindowChanges wc;
    long mask;

    width = (w > 0) ? w : 1;
    height = (h > 0) ? h : 1;

    mask = CWWidth | CWHeight;
    wc.width = width;
    wc.height = height;

    if (!resizing)
        XConfigureWindow(x11->dpy, client, mask, &wc);

    wc.width = width + clientOffset * 2;
    wc.height = height + Config::getInstance()->windowBarHeight +
        Config::getInstance()->resizeBarHeight;

    if (posx >= 0)
    {
        mask |= CWX;
        wc.x = posx;
        x = posx;
    }
    if (posy >= 0)
    {
        mask |= CWY;
        wc.y = posy;
        y = posy;
    }

    XConfigureWindow(x11->dpy, container, mask, &wc);

    mask = CWWidth | CWHeight;
    wc.height = Config::getInstance()->windowBarHeight;
    XConfigureWindow(x11->dpy, windowBar, mask, &wc);

    // shift resize bar to bottom
    wc.y = Config::getInstance()->windowBarHeight + height;
    wc.width = width;
    mask = CWY | CWWidth;
    XConfigureWindow(x11->dpy, resizeBar, mask, &wc);

    // shift the buttons to the right place
    wc.x = width - Config::getInstance()->buttonCloseX;
    wc.y = Config::getInstance()->buttonCloseY;
    mask = CWX | CWY;
    XConfigureWindow(x11->dpy, btClose, mask, &wc);

    wc.x = width - Config::getInstance()->buttonMinimizeX;
    wc.y = Config::getInstance()->buttonMinimizeY;
    mask = CWX | CWY;
    XConfigureWindow(x11->dpy, btMinimize, mask, &wc);

    wc.x = width - Config::getInstance()->buttonOwnerX;
    wc.y = Config::getInstance()->buttonOwnerY;
    mask = CWX | CWY;
    XConfigureWindow(x11->dpy, btOwner, mask, &wc);
    
    // shift resize buttons to right place
    wc.x = width - Config::getInstance()->resizeButtonWidth;
    mask = CWX;
    XConfigureWindow(x11->dpy, resizeBtSE, mask, &wc);

    wc.x = width - Config::getInstance()->resizeButtonWidth;
    mask = CWX;
    XConfigureWindow(x11->dpy, resizeBtNE, mask, &wc);


    XFlush(x11->dpy);

    TRACE("  -  resized client %x\n", (unsigned int)client);
}

void WMWindow::destroy()
{
    // Unmap using 
    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type = x11->wm_protocols;
    ce.xclient.display = x11->dpy;
    ce.xclient.window = client;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = x11->wm_delete_window;
    ce.xclient.data.l[1] = manager->time;
    ce.xclient.data.l[2] = ce.xclient.data.l[3] = ce.xclient.data.l[4] = 0l;
    XSendEvent(x11->dpy, client, false, NoEventMask, &ce);

    TRACE("suggesting client window deletion\n");
}

void WMWindow::raise()
{
    XRaiseWindow(x11->dpy, container);
}


void WMWindow::extractPID()
{
    if (process)
        return;

    Atom atom_pid = XInternAtom(x11->dpy, "_CLIENT_PID", True);
    if (!atom_pid)
    {
        ERR("Don't want to create _CLIENT_PID.\n");
        return;
    }

    Atom type_return;
    int format_return;
    unsigned long nitems_return, bytes_after;

    unsigned char* prop_return;

    XGetWindowProperty(x11->dpy, client, atom_pid, 0, 4, False, XA_INTEGER, &type_return, &format_return, &nitems_return, &bytes_after, &prop_return);
    if (nitems_return != 4)
    {
        ERR("Cannot get _CLIENT_PID. Use modified Xlib.\n");
        return;
    }
    
    int pid = (0xFF & prop_return[0]) << 24;
    pid |= (0xFF & prop_return[1]) << 16;
    pid |= (0xFF & prop_return[2]) << 8;
    pid |= (0xFF & prop_return[3]);

    DBG("_CLIENT_PID for %x is %d\n", (int)client, pid);

    Process* p = Process::getProcess(pid);
    p->addWindow(this);
    this->process = p;
}

void WMWindow::changeOwnership(PointerDevice* dev)
{
    if (!this->process)
        return;

    this->process->setOwner(dev);

    TRACE("Process %d owned by %d (%s)\n", process->getPID(), dev->getID(),
            dev->getName().c_str());
}


void WMWindow::recolor()
{
    TRACE("Process is %lx, this is %lx\n", (long)process, (long)this);
    if (process == NULL)
        return;

    PointerDevice* dev = process->getOwner();
    if (dev == NULL)
        return;

    XSetWindowBackground(x11->dpy, btOwner,
            Config::getInstance()->cursorColor(dev->getID()));
    XClearWindow(x11->dpy, btOwner);
    XFlush(x11->dpy);
}

void WMWindow::expose(XExposeEvent* ev)
{
    if (!resizing)
    XCopyArea(x11->dpy, pxContainer, container, containerGC, 
            ev->x, ev->y, ev->width, ev->height, ev->x, ev->y);
}

/**
 * Minimizes the window if minimize is set to true, unminimizes the window if
 * false is passed in.
 */
void WMWindow::setMinimize(bool minimize)
{
    this->minimized = minimize;

    if (minimize)
    {
        XUnmapWindow(x11->dpy, container);
    } else
    {
        XMapWindow(x11->dpy, container);
    }
}


void WMWindow::setResizeInProgress(bool on)
{
    if (on && !resizing)
    {
        XUnmapWindow(x11->dpy, client);
        XSetWindowAttributes attr;
        attr.background_pixel = x11->black;
        XChangeWindowAttributes(x11->dpy, container, CWBackPixel, &attr);
        resizing = true;
    } else if (!on && resizing)
    {
        XResizeWindow(x11->dpy, client, width, height);
        XMapWindow(x11->dpy, client);
        resizing = false;
        paintWindowBar();
        XCopyArea(x11->dpy, pxContainer, container, containerGC, 
                0, 0, width + clientOffset * 2, height +
                Config::getInstance()->windowBarHeight +
                Config::getInstance()->resizeBarHeight, 0, 0); 
    }
}


void WMWindow::paintWindowBar()
{
    cairo_t* cr;
    cairo_surface_t* cr_sf;
    int totalHeight = height + Config::getInstance()->windowBarHeight +
        Config::getInstance()->resizeBarHeight;
    int totalWidth = width + clientOffset * 2;
    TRACE(" - painting size %d/%d\n", totalWidth, totalHeight);

    if (pxContainer)
        XFreePixmap(x11->dpy, pxContainer);
    pxContainer = 
      XCreatePixmap(x11->dpy, container, totalWidth, totalHeight, x11->depth);

    cr_sf = cairo_xlib_surface_create(x11->dpy, pxContainer, x11->vis, 
            totalWidth, totalHeight);
    cr = cairo_create(cr_sf);
    cairo_surface_destroy(cr_sf);
    cairo_set_source_rgb(cr, 0.88, 0.88, 1);
    cairo_paint(cr);
    int rounded = 20;
    cairo_move_to(cr, rounded, 0);
    cairo_rel_line_to(cr, totalWidth - rounded * 2, 0);
    cairo_rel_curve_to(cr, rounded, 0, rounded, 0, rounded, rounded);
    cairo_rel_line_to(cr, 0, totalHeight - rounded * 2);
    cairo_rel_curve_to(cr, 0, rounded, 0, rounded, -rounded, rounded);
    cairo_rel_line_to(cr, -totalWidth + rounded * 2, 0);
    cairo_rel_curve_to(cr, -rounded, 0, -rounded, 0, -rounded, -rounded);
    cairo_rel_line_to(cr, 0, -totalHeight + rounded * 2);
    cairo_rel_curve_to(cr, 0, -rounded, 0, -rounded, rounded, -rounded);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 3);
    cairo_stroke(cr);

    int btwidth = Config::getInstance()->buttonWidth;
    int btheight = Config::getInstance()->buttonHeight;
    int x, y;

    // close button
    x = width - Config::getInstance()->buttonCloseX;
    y = Config::getInstance()->buttonCloseY;

    cairo_translate(cr, x, y);
    paintButton(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 3);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, btwidth/3.0, btheight/3.0);
    cairo_line_to(cr, 2 * btwidth/3.0, 2 * btheight/3.0);
    cairo_move_to(cr, 2 * btwidth/3.0, btheight/3.0);
    cairo_line_to(cr, btwidth/3.0, 2 * btheight/3.0);
    cairo_stroke(cr);

    // minimize button
    x = totalWidth - Config::getInstance()->buttonMinimizeX;
    y = Config::getInstance()->buttonMinimizeY;
    cairo_identity_matrix(cr);
    cairo_translate(cr, x, y);
    paintButton(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, 3);
    cairo_move_to(cr, btwidth/5.0, btheight - 2);
    cairo_line_to(cr, 4.0 * btwidth/5.0, btheight - 2);
    cairo_stroke(cr);

    cairo_destroy(cr);
}

void WMWindow::paintButton(cairo_t* cr)
{
    int rounded = 4;
    int btwidth = Config::getInstance()->buttonWidth;
    int btheight = Config::getInstance()->buttonHeight;
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, rounded, 0);
    cairo_rel_line_to(cr, btwidth - rounded * 2, 0);
    cairo_rel_curve_to(cr, rounded, 0, rounded, 0, rounded, rounded);
    cairo_rel_line_to(cr, 0, btheight - rounded * 2);
    cairo_rel_curve_to(cr, 0, rounded, 0, rounded, -rounded, rounded);
    cairo_rel_line_to(cr, -btwidth + rounded * 2, 0);
    cairo_rel_curve_to(cr, -rounded, 0, -rounded, 0, -rounded, -rounded);
    cairo_rel_line_to(cr, 0, -btheight + rounded * 2);
    cairo_rel_curve_to(cr, 0, -rounded, 0, -rounded, rounded, -rounded);
    cairo_fill(cr);
}

