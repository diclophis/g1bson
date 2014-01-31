/* $Id: Dock.cpp,v 1.5 2007/01/07 05:00:56 whot Exp $ */

#include "Dock.h"
#include <cairo/cairo-xlib.h>
#include <math.h>

Dock::Dock(XConn* x11)
{
    this->x11 = x11;

    apps = Config::getInstance()->apps;
    processes.clear();


    /* place dock bottom center */
    x = (int)((x11->width/2) - (apps.size() / 2.0 * DOCK_BUTTON_WIDTH)) -
        DOCK_WIDTH_EXTENDED/2;
    y = x11->height - DOCK_BUTTON_HEIGHT - DOCK_HEIGHT_EXTENDED;
    width = apps.size() * DOCK_BUTTON_WIDTH + DOCK_WIDTH_EXTENDED;
    height = DOCK_BUTTON_HEIGHT + DOCK_WIDTH_EXTENDED;

    dock = XCreateSimpleWindow(x11->dpy, x11->root, 
                               x, y, 
                               width, height, 
                               0, 0, x11->white);
    XGCValues vals;
    gc_dock = XCreateGC(x11->dpy, dock, 0, &vals);

    backbuff = XCreatePixmap(x11->dpy, dock, width, height, x11->depth);

    XSelectInput(x11->dpy, dock, ExposureMask);

    repaint();

    /* add apps to dock */
    vector<DockApp*>::const_iterator it = apps.begin();

    int app_x = DOCK_WIDTH_EXTENDED/2;
    while(it != apps.end())
    {
        DockApp* app = *it;

        app->initGUI(dock, app_x, DOCK_HEIGHT_EXTENDED, DOCK_BUTTON_WIDTH,
                DOCK_BUTTON_HEIGHT); 
        app->setup();
        app_x += DOCK_BUTTON_WIDTH;
        it++;
    }

    XMapRaised(x11->dpy, dock);
}

Dock::~Dock()
{
    XFreePixmap(x11->dpy, backbuff);
}

bool Dock::hasWindow(Window win)
{
    if (win == dock)
        return true;

    vector<DockApp*>::const_iterator it = apps.begin();

    while(it != apps.end())
    {
        if ((*it)->hasWindow(win))
            return true;
        it++;
    }

    /* check minimized apps */
    vector<DockProcess*>::const_iterator it2 = processes.begin();
    while(it2 != processes.end())
    {
        if ((*it2)->hasWindow(win))
            return true;
        it2++;
    }

    /* check keyboards */
    vector<DockKeyboard*>::const_iterator it3 = keyboards.begin();
    while(it3 != keyboards.end())
    {
        if ((*it3)->hasWindow(win))
            return true;
        it3++;
    }

    return false;
}

void Dock::handleExpose(XExposeEvent* ev)
{
    XFlush(x11->dpy);

    XCopyArea(x11->dpy, backbuff, dock, gc_dock, 0, 0, width, height, 0, 0);

    vector<DockApp*>::const_iterator it = apps.begin();

    while(it != apps.end())
    {
        if ((*it)->hasWindow(ev->window))
        {
            (*it)->handleExpose(ev);
            break;
        }
        it++;
    }

    vector<DockProcess*>::const_iterator it2 = processes.begin();

    while(it2 != processes.end())
    {
        if ((*it2)->hasWindow(ev->window))
        {
            (*it2)->handleExpose(ev);
            break;
        }
        it2++;
    }


    vector<DockKeyboard*>::const_iterator it3 = keyboards.begin();
    while(it3 != keyboards.end())
    {
        if ((*it3)->hasWindow(ev->window))
        {
            (*it3)->handleExpose(ev);
            break;
        }
        it3++;
    }
    return;
}

/*
 * A process has been minimized and needs to be appended to the GUI.
 */
void Dock::appendProcess(WMWindow* win)
{
    x -= DOCK_BUTTON_WIDTH/2;
    width += DOCK_BUTTON_WIDTH;
    XMoveResizeWindow(x11->dpy, dock, x, y, width, height);

    int dp_x = width - DOCK_BUTTON_WIDTH - DOCK_WIDTH_EXTENDED/2;
    int dp_y = DOCK_HEIGHT_EXTENDED;
    DockProcess* dp = new DockProcess(x11, win, this);
    dp->initGUI(dock, dp_x, dp_y, DOCK_BUTTON_WIDTH, DOCK_BUTTON_HEIGHT);
    dp->setup();
    processes.push_back(dp);


    XFreePixmap(x11->dpy, backbuff);
    backbuff = XCreatePixmap(x11->dpy, dock, width, height, x11->depth);
    repaint();

    TRACE("append dock with %d, dpx %d\n", width, dp_x);
}

void Dock::appendKeyboard(KeyboardDevice* k)
{
    x -= DOCK_BUTTON_WIDTH/2;
    width += DOCK_BUTTON_WIDTH;
    XMoveResizeWindow(x11->dpy, dock, x, y, width, height);
    int dk_x = width - DOCK_BUTTON_WIDTH - DOCK_WIDTH_EXTENDED/2;
    int dk_y = DOCK_HEIGHT_EXTENDED;
    DockKeyboard* dk = new DockKeyboard(x11, k);
    dk->initGUI(dock, dk_x, dk_y, DOCK_BUTTON_WIDTH, DOCK_BUTTON_HEIGHT);
    dk->setup();
    keyboards.push_back(dk);

    XFreePixmap(x11->dpy, backbuff);
    backbuff = XCreatePixmap(x11->dpy, dock, width, height, x11->depth);
    repaint();

    TRACE("append keyboard with %d, dkx %d\n", width, dk_x);
}


/* 
 * Removes a process from the dock. Usually after maximizing it again.
 */
void Dock::removeProcess(DockProcess* dp)
{
    x += DOCK_BUTTON_WIDTH/2;
    width -= DOCK_BUTTON_WIDTH;
    XMoveResizeWindow(x11->dpy, dock, x, y, width, height);

    int dp_x, dp_y;
    dp_x = apps.size() * DOCK_BUTTON_WIDTH + DOCK_WIDTH_EXTENDED/2;
    dp_y = DOCK_HEIGHT_EXTENDED;

    vector<DockProcess*>::iterator it = processes.begin();
    while(it != processes.end())
    {
        if( dp == (*it) ){
            it = processes.erase(it);
            delete dp;
        }else{
            (*it)->move(dp_x, dp_y);
            dp_x += DOCK_BUTTON_WIDTH;
            it++;
        }
    }

    XFreePixmap(x11->dpy, backbuff);
    backbuff = XCreatePixmap(x11->dpy, dock, width, height, x11->depth);
    repaint();

    TRACE("dock with %d, dpx %d\n", width, dp_x);
}

void Dock::repaint()
{
    cairo_surface_t* surface = 
        cairo_xlib_surface_create(x11->dpy, backbuff, x11->vis, width, height);

    cr = cairo_create(surface);
    cairo_surface_destroy(surface);
    cairo_set_line_width(cr, 5);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    int rpx = 5; // pixels for rounded corners
    cairo_move_to(cr, rpx, 0);
    cairo_line_to(cr, width - rpx, 0);
    cairo_arc(cr, width - rpx, rpx, rpx, 3.0 * M_PI/2.0, 2 * M_PI);
    cairo_line_to(cr, width, height);
    cairo_line_to(cr, 0, height);
    cairo_line_to(cr, 0, rpx);
    cairo_arc(cr, rpx, rpx, rpx, M_PI, 3.0 * M_PI/2.0);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);

    cairo_set_source_rgb(cr, 0.88, 0.88, 1);
    cairo_fill(cr);
    cairo_destroy(cr);
}

void Dock::setPointerEvents(vector<PointerDevice*>* pointers)
{
    vector<DockKeyboard*>::const_iterator kit = keyboards.begin();
    vector<PointerDevice*>::const_iterator it; 
    while(kit != keyboards.end())
    {
        it = pointers->begin();
        while(it != pointers->end())
        {
            (*it)->setDockEvents((*kit)->getButton());
            it++;
        }
        kit++;
    }

    vector<DockApp*>::const_iterator appit = apps.begin();
    while(appit != apps.end())
    {
        it = pointers->begin();
        while(it != pointers->end())
        {
            (*it)->setDockEvents((*appit)->getButton());
            it++;
        }
        appit++;
    }

    vector<DockProcess*>::const_iterator prit = processes.begin();
    while(prit != processes.end())
    {
        it = pointers->begin();
        while(it != pointers->end())
        {
            (*it)->setDockEvents((*prit)->getButton());
            it++;
        }
        prit++;
    }
}

DockItem* Dock::getDockItem(Window win)
{
    vector<DockApp*>::const_iterator appit = apps.begin();
    while(appit != apps.end())
    {
        if ((*appit)->getButton() == win)
            return (*appit);
        appit++;
    }

    vector<DockProcess*>::const_iterator prit = processes.begin();
    while(prit != processes.end())
    {
        if ((*prit)->getButton() == win)
            return (*prit);
        prit++;
    }

    vector<DockKeyboard*>::const_iterator kit = keyboards.begin();
    while(kit != keyboards.end())
    {
        if ((*kit)->getButton() == win)
            return (*kit);
        kit++;
    }

    return NULL;
}
