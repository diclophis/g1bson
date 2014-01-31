/* $Id: WMWindow.h,v 1.19 2007/01/07 05:00:56 whot Exp $ */

/*--
  --*/

#ifndef __WMWINDOW_H__
#define __WMWINDOW_H__

#include<X11/Xlib.h>
#include<X11/Xatom.h>
#include<stdio.h>
#include<vector>
#include<utility>
#include "logger.h"
#include "Config.h"
#include "Manager.h"
#include "Process.h"
#include "XConn.h"
#include <cairo/cairo.h>

using namespace std;

class Manager;
class PointerDevice;
class Process;

/**
 * Describes a top-level window. A window consists of a container (with the
 * root window as parent) and several child windows (the actual client's
 * window and whatever the window manager added).  
 */
class WMWindow
{
    private:
        Manager* manager;
        XConn* x11;

        Process* process;

        /* X11 windows used for the client's window. */
        Window container; // container window, parent of all others
        GC containerGC;
        Window client; // the actual client's window
        Window windowBar; // window manager top bar
        GC windowBarGC;
        Window resizeBar; // the bottom bar. Holds resizeSE and resizeSW.

        Window resizeBtNE; // resize button north-east 
        Window resizeBtSE; // resize button south-east 
        Window resizeBtNW; // resize button north-west 
        Window resizeBtSW; // resize button south-west
        Window btClose; // close window button
        Window btOwner; // ownership button
        Window btMinimize; // minimize button

        /* Pixmaps for back buffering */
        Pixmap pxContainer; 

        Window resizeWin; // overlay during resizing

        int clientOffset; // x offset of client
        int width; // width of container window
        int height; // height of container window
        int x; // x coordinate of container window relative to root window
        int y; // y coordinate of container window relative to root window

        int state;
        int override_redirect;

        bool minimized;
        bool resizing;

        PointerDevice* controller; // the device in control of the window
        PointerDevice* restrictedTo; // the device allowed to access the window
        vector<PointerDevice*> resizers; // devices that resizes the window ATM

        bool exception; // used to indicate a failed startup.

    private:
        void paintWindowBar();
        void paintButton(cairo_t* cr);

    public:
        WMWindow(Manager* manager, Window client_window, XConn* x11);
        ~WMWindow();
        bool hasWindow(Window window); 
        bool isWindowBar(Window window);
        bool isResizeButton(Window window);
        bool isResizeBar(Window window);
        bool isButtonClose(Window window);
        bool isButtonMinimize(Window window);
        bool isContainer(Window window);
        bool isClientWindow(Window window);
        bool onPosition(int x, int y);

        int getX() { return x; }
        int getY() { return y; }
        int getWidth() { return width; }
        int getHeight() { return height; }

        void decorate();
        void reconfigure(XConfigureRequestEvent* ev);
        void manage();
        void move(int x, int y);
        void setResizeInProgress(bool on);
        void resize(int width, int height);
        void resizeDirected(Window button, int width, int height);
        void resizeAbsolute(int x, int y, int width, int height);
        void mapAll();
        void raise();

        int getState() { return state; }
        void setState(int s) { state = s; }

        bool setController(PointerDevice* dev);
        void releaseController() { controller = NULL; }
        bool addResizer(PointerDevice* dev);
        void releaseResizer(PointerDevice* dev);

        Window getWindowBar() { return windowBar; }
        Window getResizeBar() { return resizeBar; }
        Window getButtonClose() { return btClose; }
        Window getButtonMinimize() { return btMinimize; }
        Window getClientWindow() { return client; }
        Window getContainer() { return container; }

        Window getResizeBtNE() { return resizeBtNE; }
        Window getResizeBtSE() { return resizeBtSE; }
        Window getResizeBtNW() { return resizeBtNW; }
        Window getResizeBtSW() { return resizeBtSW; }

        bool isResizing() { return resizing; }

        void suggestDestruction();
        void destroy();
        void expose(XExposeEvent* event);
        void setMinimize(bool minimize);

        bool restrictTo(PointerDevice* dev);
        bool isLocked() { return (restrictedTo != NULL); }
        bool release(PointerDevice* dev);

        void cancel() { exception = true; } // to cancel constructor

        void extractPID();

        void changeOwnership(PointerDevice* dev);
        void recolor();

        void handleEnterLeaveNotify(XCrossingEvent* ev);

        friend class Manager;
};

#endif
