/* $Id: Manager.cpp,v 1.26 2007/01/07 05:00:56 whot Exp $ */

/*--
 --*/

#include "Manager.h"
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

int MPWM_ErrorHandler(Display* dpy, XErrorEvent* ev)
{
    char *buffer = new char[1024];
    
    XGetErrorText(dpy, ev->error_code, buffer, 1024);
    DBG("%s, request: %d, id: %x\n", buffer, ev->request_code, 
            (unsigned int)ev->resourceid);
    delete buffer;

    return 0;
}


/**
 * Start up and initialize internal variables.
 */
Manager::Manager()
{
    x11 = NULL;
    stop = false;
    dock = NULL;
    time = 0;
}

/**
 * Clean up.
 */
Manager::~Manager()
{
    XCloseDisplay(x11->dpy);
}

/**
 * Connect to the X display, decorate all existing windows and set up MPGXlib.
 * @param display String with the name of the display.
 */
void Manager::init(char* display)
{
    initX11(display);

    dock = new Dock(x11);
    initXi();
    queryInitial();
}

/**
 * Connect to the X display and register as WM at the root window.
 * @param display The display to connect to (i.e. "host:0").
 */
void Manager::initX11(char* display)
{
    x11 = new XConn(display);

    Config::init(x11);

    XSetWindowAttributes attr;
    attr.background_pixmap = None;
    attr.background_pixel = x11->white;
    attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
        ColormapChangeMask | PropertyChangeMask | ExposureMask; 
    XChangeWindowAttributes(x11->dpy, x11->root, 
            CWBackPixmap | CWBackPixel | CWEventMask, &attr);

    XSync(x11->dpy, False);

    XGCValues vals;
    root_gc = XCreateGC(x11->dpy, x11->root, 0, &vals);

    if (!root_gc)
        ERR("Could not create canvas on root window\n");
    else
        XFillRectangle(x11->dpy, x11->root, root_gc, 0, 0, x11->width,
                x11->height);

    pxBackground = XCreatePixmap(x11->dpy, x11->root, 
                                 x11->width, x11->height, x11->depth);
    cairo_surface_t* cr_sf = cairo_xlib_surface_create(x11->dpy, pxBackground,
            x11->vis, x11->width, x11->height);
    cairo_t* cr = cairo_create(cr_sf);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_surface_destroy(cr_sf);
    cairo_destroy(cr);

    XCopyArea(x11->dpy, pxBackground, x11->root, root_gc, 0, 0, x11->width,
            x11->height, 0, 0);
    XSetErrorHandler(MPWM_ErrorHandler);
}

/**
 * Set up the X Input Extension.
 */
void Manager::initXi()
{
    XIDeviceInfo* devices;
    int devicecount;

    devices = XIQueryDevice(x11->dpy, XIAllMasterDevices, &devicecount);

    if (devicecount <= 0)
        throw new DeviceError(DeviceError::NO_DEVICES);

    while(devicecount)
    {
        XIDeviceInfo* currDevice;

        currDevice = &devices[--devicecount];
        /* ignore slave devices, only masters are interesting */
        if ((currDevice->use == XIMasterPointer))
        {
            try
            {
                PointerDevice *p = new PointerDevice(currDevice, x11, this);
                pointers.push_back(p);

            } catch (DeviceError* e)
            {
                ERR("%s\n", e->message.c_str());
            }
        } else if ((currDevice->use == XIMasterKeyboard))
        {
            try
            {
                KeyboardDevice *k = new KeyboardDevice(currDevice, x11, this);
                keyboards.push_back(k);
                k->setPaired(idToPointerDevice(currDevice->attachment));
            } catch (DeviceError* e)
            {
                ERR("%s\n", e->message.c_str());
            }
        }
    }

    dock->setPointerEvents(&pointers);
    XIFreeDeviceInfo(devices);

    XIEventMask mask;
    unsigned char bits[4] = {0};

    mask.mask = bits;
    mask.mask_len = sizeof(bits);
    mask.deviceid = XIAllDevices;
    SetBit(bits, XI_HierarchyChanged);

    XISelectEvents(x11->dpy, x11->root, &mask, 1);
}

/**
 * Disconnect from X and bring down any allocated memory.
 */
void Manager::takedown()
{
    XCloseDisplay(x11->dpy);
}

/**
 * Add a new window.
 * 
 */
void Manager::addNewClient()
{

}

/**
 * Query the root window for the initial set of windows and create decorations
 * for all of them.
 */
void Manager::queryInitial()
{
   windows = queryWindows(); 
   vector<WMWindow*>::const_iterator it = windows.begin();

   while(it != windows.end())
   {
       (*it)->decorate();
       (*it)->manage();
       it++;
   }

   XSync(x11->dpy, False);
}
   
/**
 * Query the root window's top-level childs.
 */
vector<WMWindow*> Manager::queryWindows()
{
    Window rootWindow;
    Window parent;
    Window *children;
    Window *clientChildren;
    unsigned int childcount, clientchildcount;
    vector<WMWindow*> windowList;

    XQueryTree(x11->dpy, x11->root, &rootWindow, &parent, &children,
            &childcount);

    // for each window found we query whether the window is a WM window, just
    // to be sure. If the window isn't, it is a new client window.
    for (unsigned int i = 0; i < childcount; i++)
    {
        XQueryTree(x11->dpy, children[i], &rootWindow, &parent,
                &clientChildren, &clientchildcount);
        XFree(clientChildren);

        if (parent != x11->root || isWMWindow(children[i]))
        {
            continue;
        }

        WMWindow* win = new WMWindow(this, children[i], x11);
        windowList.push_back(win);
    }
    XFree(children);

    return windowList;
}

/**
 * @return true if the given window belongs to a WM window (that is,
 * container, decoration or other).
 */
bool Manager::isWMWindow(Window win)
{
    WMWindow* window = windowToWMWindow(win);
    if (window)
        return true;

    
    if (dock->hasWindow(win))
        return true;

    return false;
}

bool Manager::isWMDecoration(Window win)
{
    WMWindow* window = windowToWMWindow(win);
    if (window && !window->isClientWindow(win))
        return true;

    return false;
}

/**
 * @returns A pointer to the WMWindow this window belongs to.
 */
WMWindow* Manager::windowToWMWindow(Window w)
{
    // iterate through windows
    vector<WMWindow*>::reverse_iterator it = windows.rbegin();

    while(it != windows.rend())
    {
        if ((*it)->hasWindow(w))
            return *it;
        it++;
    }

    return NULL;
}


/**
 * Main loop. Waits for events and processes them accordingly.
 */
void Manager::loop()
{
    XEvent ev;

    while(!stop)
    {
        XNextEvent(x11->dpy, &ev);
        switch(ev.type)
        {
            /* WM events */
            case CreateNotify:
                handleCreateNotify(&ev.xcreatewindow);
                break;
            case MapRequest:
                handleMapRequest(&ev.xmap);
                break;
            case MapNotify:
                handleMapNotify(&ev.xmap);
                break;
            case ConfigureRequest:
                handleConfigureRequest(&ev.xconfigurerequest);
                break;
            case ReparentNotify:
                handleReparentNotify(&ev.xreparent);
                break;
            case DestroyNotify:
                handleDestroyNotify(&ev.xdestroywindow);
                break;
            case UnmapNotify:
                handleUnmapNotify(&ev.xunmap);
                break;
            case ConfigureNotify:
                break;
            case Expose:
                if (ev.xexpose.window == x11->root)
                    paintBackground(&(ev.xexpose));
                else
                {
                    WMWindow* client = windowToWMWindow(ev.xexpose.window);
                    if (client)
                        client->expose(&ev.xexpose);
                    else
                        dock->handleExpose(&ev.xexpose);
                }
                break;
            case PropertyNotify:
                handlePropertyNotify(&(ev.xproperty));
                break;
                /* XInput device events */
            default:
                handleOtherEvents((XIEvent*)&ev);
                break;
        }
    }
}

/**
 * Handle create notify. A create notify causes the WM to allocate the
 * necessary structures for the window borders but does not reparent the
 * window yet. If we reparent, we would not get the MapRequest.
 */
void Manager::handleCreateNotify(XCreateWindowEvent* ev)
{
    TRACE("Create notify for window %x\n", (unsigned int)ev->window);

    if (isWMWindow(ev->window))
    {
        TRACE(" -- No action. WM window\n");
        return;
    }

    if (ev->parent != x11->root)
    {
        TRACE(" -- No action, parent is not root\n");
        return;
    }

    try {
        WMWindow* w = new WMWindow(this, ev->window, x11);
        w->decorate();
        windows.push_back(w);
    } catch (XError err)
    {
        ERR("%s\n", err.message.c_str());
    }
}

/**
 * A map request is issued by the client when it wants to map something to the
 * root window. We are a WM, so we get the request and instead of the client
 * mapping the window, we have to map it.
 */
void Manager::handleMapRequest(XMapEvent* ev)
{
    WMWindow* w = windowToWMWindow(ev->window);
    if (w == NULL)
    {
        DBG("w is null\n");
        return;
    }

    if (w->getState() == WithdrawnState)
    {
        w->manage();
        w->setState(NormalState);
        TRACE("Managing window %x\n", (unsigned int)ev->window);
    }

    w->mapAll();

    TRACE("Client %x mapped\n", (unsigned int)ev->window);
}

void Manager::handleMapNotify(XMapEvent* ev)
{
    TRACE("Map notify for %x\n", (unsigned int)ev->window);
    if (isWMWindow(ev->window))
    {
        TRACE(" - is WM window\n");
        return;
    }

    WMWindow* w = windowToWMWindow((unsigned int)ev->window);
    if (w == NULL)
    {
        TRACE(" - non client window\n");
        return;
    }

    TRACE(" - mapping all\n");
    w->mapAll(); 
}

void Manager::handleConfigureRequest(XConfigureRequestEvent* ev)
{
    TRACE("Configure for %x\n", (unsigned int)ev->window);

    WMWindow* w = windowToWMWindow(ev->window);

    if (w != NULL)
        w->reconfigure(ev);

}

void Manager::handleReparentNotify(XReparentEvent* ev)
{
    TRACE("ReparentNotify for client %x. Now on parent %x.\n", (unsigned int)ev->window, (unsigned int)ev->parent);

    XMapWindow(x11->dpy, ev->window);
    

    WMWindow* w = windowToWMWindow(ev->window);
    if (w == NULL)
    {
        TRACE(" - no client for window.");
        return;
    }

    if (w->getState() == IsViewable) 
        w->mapAll();
    XFlush(x11->dpy);
}

void Manager::handleDestroyNotify(XDestroyWindowEvent* ev)
{
    WMWindow* w = windowToWMWindow(ev->window);


    TRACE("Destroy notify for %x\n", (unsigned int)ev->window);

    if (w == NULL)
    {
        TRACE(" - not a client window\n");
        return;
    }

    if (w->isClientWindow(ev->window))
    {
        vector<WMWindow*>::iterator it = windows.begin();
        while(it != windows.end())
        {
            if ((*it) == w)
            {
                windows.erase(it);
                break;
            }
            it++;
        }
        delete w;
    }
}


/**
 * Unmaps the client window's container window if the client's window has
 * unmapped.
 * @param ev The Unmap event caused by the X server.
 */
void Manager::handleUnmapNotify(XUnmapEvent* ev)
{
    TRACE("Unmap notify for %x\n", (unsigned int)ev->window);
    if (isWMDecoration(ev->window))
    {
        TRACE("-- is WM decoration\n");
        return;
    }

    WMWindow* w = windowToWMWindow(ev->window);

    if (w == NULL)
    {
        TRACE("-- is unknown window\n");
        return;
    }

    if (!w->isResizing())
        XUnmapWindow(x11->dpy, w->container);
}


/**
 * Handles extension events and all other events.
 * Input extension events are not core events. Thus each input extension
 * event will pass through this function. However, other core events that are
 * not handled in the loop may also pass through here.
 * @param ev The event gathered in the X event loop.
 */

void Manager::handleOtherEvents(XIEvent *ev)
{
    if (ev->type != GenericEvent)
        return;

    if (ev->evtype == XI_Motion)
    {
        handleMotionEvent((XIDeviceEvent*)ev);
    } else if (ev->evtype == XI_ButtonPress)
    {
        handleButtonPress((XIDeviceEvent*)ev);
    } else if (ev->evtype == XI_ButtonRelease)
    {
        handleButtonRelease((XIDeviceEvent*)ev);
    } else if (ev->evtype == XI_Enter || ev->evtype == XI_Leave)
    {
        handleEnterLeaveNotify((XIEnterEvent*)ev);
    } else if (ev->evtype == XI_HierarchyChanged)
    {
        handleHierarchyEvent((XIHierarchyEvent*)ev);
    }
}

/**
 * Handles all Input Extension motion events. 
 */
void Manager::handleMotionEvent(XIDeviceEvent* mev)
{
    time = mev->time;

    PointerDevice* dev = idToPointerDevice(mev->deviceid);

    if (dev == NULL)
    {
        ERR("Device %d is null\n", (int)mev->deviceid);
        return;
    }

    // There may be a drag event.
    if (dev->isDragging())
        dev->dragTo(mev->root_x, mev->root_y);

    // Resizing is slow. We check if there are other events in the queue cos
    // if there are, we don't resize.
    if (dev->isResizing())
    {
#if 0
        XDeviceMotionEvent ev;
        if (!XCheckTypedEvent(x11->dpy, 
                              PointerDevice::XI_MotionNotify,
                              (XEvent*) &ev))
        {
            if (mev->deviceid == ev.deviceid && mev->state == ev.state)
            {
                // skip
            } else 
            {
                XPutBackEvent(x11->dpy, (XEvent*)&ev);
                dev->resizeTo(mev->x_root, mev->y_root);
            }
        } else
#endif
            dev->resizeTo(mev->root_x, mev->root_y);
    }

}

/**
 * Handles all InputExtension button presses.
 */
void Manager::handleButtonPress(XIDeviceEvent* bev)
{
    time = bev->time;

    PointerDevice* dev = idToPointerDevice(bev->deviceid);

    if (dev == NULL)
    {
        ERR("Device %d is null\n", (int)bev->deviceid);
        return;
    }

    TRACE("DeviceButtonPress for device %s on %f/%f\n",
            dev->getName().c_str(), bev->root_x, bev->root_y);

    DockItem* di = dock->getDockItem(bev->event);
    if (di)
    {
        di->handleButtonEvent(dev, bev);
        return;
    }

    WMWindow* wmwindow = windowToWMWindow(bev->event);

    if (!wmwindow)
    {
        ERR(" - WMWindow not found for %x\n", (int)bev->event);
        return;
    }

    if (wmwindow->isResizeBar(bev->event) ||
            wmwindow->isWindowBar(bev->event) ||
            wmwindow->isClientWindow(bev->event))
    {
        XISetClientPointer(x11->dpy, wmwindow->getClientWindow(), dev->getID());
        vector<KeyboardDevice*>::const_iterator it = keyboards.begin();
        while(it != keyboards.end())
        {
            if ((*it)->getPaired() == dev)
                XISetFocus(x11->dpy, (*it)->getID(),
                        wmwindow->getClientWindow(), PointerRoot);
            it++;
        }

        if (wmwindow->isClientWindow(bev->event))
        {
            XISetClientPointer(x11->dpy, wmwindow->getClientWindow(),
                    dev->getID());
        }
        raiseWindow(wmwindow);
    }

    if (wmwindow->isWindowBar(bev->event))
    {
        if (dev->dragOn(wmwindow, bev->event_x, bev->event_y))
            TRACE(" - Drag on for %x\n", (int)bev->event);
        wmwindow->changeOwnership(dev);
    }


    if (wmwindow->isResizeButton(bev->event))
    {
        if (dev->resizeOn(wmwindow, bev->event, bev->root_x, bev->root_y))
            TRACE(" - Resize on for %x\n", (int)bev->event);
    }

    if (wmwindow->isButtonMinimize(bev->event))
    {
        wmwindow->setMinimize(true);
        dock->appendProcess(wmwindow);
        dock->setPointerEvents(&pointers);
    }
    XIAllowEvents(x11->dpy, dev->getID(), XIReplayDevice, CurrentTime);
/* FIXME     XIAllowEvents(x11->dpy, dev->getID(), XIAsyncOtherDevices, CurrentTime); */
}

void Manager::handleButtonRelease(XIDeviceEvent* bev)
{
    time = bev->time;

    PointerDevice* dev = idToPointerDevice(bev->deviceid);

    if (dev == NULL)
    {
        ERR("Device %d is null\n", (int)bev->deviceid);
        return;
    }

    TRACE("DeviceButtonRelease for device %s\n", dev->getName().c_str());
    TRACE("--- release has window %x\n", (int)bev->event);
    TRACE("--- release has subwindow %x\n", (int)bev->child);


    if (dev->isResizing())
    {
        dev->resizeOff();
        TRACE(" - resize stopped\n");
        return;
    }

    // check for button clicks
    WMWindow* win = windowToWMWindow(bev->event);

    if (win == NULL)
    {
        ERR(" - WMWindow for %x not found\n", (int)bev->event);
        return;
    }

    if (win->isButtonClose(bev->event))
    {
        win->destroy();
        return;
    }

    if (dev->isDragging())
    {
        dev->dragOff();
        TRACE(" - dragging stopped\n");
        return;
    }
}

void Manager::handleEnterLeaveNotify(XIEnterEvent* ev)
{
}

void Manager::handleHierarchyEvent(XIHierarchyEvent* ev)
{
    TRACE("Device hierarchy event\n");

    if (!(ev->flags & XIMasterAdded))
        return;

    XIHierarchyInfo *info = ev->info;

    while(ev->num_info--)
    {
        if (info->flags & XIMasterAdded)
        {
            int count = 0;
            XIDeviceInfo *dev = XIQueryDevice(x11->dpy, info->deviceid, &count);
            if (count != 1)
            {
                ERR("Hierarchy event: count should be 1\n");
                return;
            }

            if ((dev->use == XIMasterPointer))
            {
                try
                {
                    PointerDevice *p = new PointerDevice(dev, x11, this);
                    pointers.push_back(p);

                } catch (DeviceError* e)
                {
                    ERR("%s\n", e->message.c_str());
                }
            } else if ((dev->use == XIMasterKeyboard))
            {
                try
                {
                    KeyboardDevice *k = new KeyboardDevice(dev, x11, this);
                    keyboards.push_back(k);
                    k->setPaired(idToPointerDevice(dev->attachment));
                } catch (DeviceError* e)
                {
                    ERR("%s\n", e->message.c_str());
                }
            }
            XIFreeDeviceInfo(dev);
        }

        info++;
    }

    XFreeEventData(x11->dpy, (XGenericEventCookie*)ev);
}


/**
 * @return the PointerDevice that has the given ID.
 */
PointerDevice* Manager::idToPointerDevice(int id)
{
    vector<PointerDevice*>::const_iterator it = pointers.begin();

    while(it != pointers.end())
    {
        if ((*it)->getID() == id)
            return *it;
        it++;
    }
    return NULL;
}

KeyboardDevice* Manager::idToKeyboardDevice(int id)
{
    vector<KeyboardDevice*>::const_iterator it = keyboards.begin();
    while(it != keyboards.end())
    {
        if ((*it)->getID() == id)
            return *it;
        it++;
    }
    return NULL;
}


/**
 * @return The list of pointers registered at the manager.
 */
vector<PointerDevice*>* Manager::getPointers()
{
    return &pointers;
}


bool Manager::hasShapeExtension()
{
    static bool queried = false;
    static bool hasShape = false;

    if (!queried)
    {
        int eventop, errorop;
        hasShape = XShapeQueryExtension(x11->dpy, &eventop, &errorop);
        if (!hasShape)
            ERR("No shape extension. Will not use pretty cursors.\n");

        queried = true;
    }

    return hasShape;
}

/**
 * Changes the root window color to white.
 */
void Manager::paintBackground(XExposeEvent* ev)
{
    XCopyArea(x11->dpy, pxBackground, x11->root, root_gc, ev->x, ev->y, 
            ev->width, ev->height, ev->x, ev->y);
}

void Manager::handlePropertyNotify(XPropertyEvent* ev)
{
    WMWindow* win = windowToWMWindow(ev->window);
    if (win == NULL)
        return;

}

/**
 * Returns the wm-window on the given coordinates or NULL if there is none.
 * FIXME: this will not work well when there's more than two windows on the
 * same spot. We need to integrate stacking order here.
 */
WMWindow* Manager::xyToWMWindow(int x, int y)
{
    vector<WMWindow*>::reverse_iterator it = windows.rbegin();
    while(it != windows.rend())
    {
        WMWindow* current = *it;
        if (current->onPosition(x, y))
            return current;
        it++;
    }
    return NULL;
}

/**
 * Sets the ButtonPress mask for all devices.
 */
void Manager::setButtonPressMask(Window win)
{
    XIEventMask mask;
    unsigned char bits[4] = {0};

    mask.mask = bits;
    mask.mask_len = sizeof(bits);
    mask.deviceid = XIAllMasterDevices;
    SetBit(bits, XI_ButtonPress);

    XISelectEvents(x11->dpy, win, &mask, 1);
}

void Manager::raiseWindow(WMWindow* wmwindow)
{
    vector<WMWindow*>::reverse_iterator it = windows.rbegin();

    while(it != windows.rend())
    {
        if (*it == wmwindow)
        {
            // why ++it see http://www.ddj.com/dept/cpp/184401406
            windows.erase((++it).base());
            windows.push_back(wmwindow);
            break;
        }
        it++;
    }

    wmwindow->raise();
}
