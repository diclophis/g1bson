/* $Id: Manager.h,v 1.18 2007/01/07 05:00:56 whot Exp $ */

/*--
  --*/

#ifndef __MANAGER_H__
#define __MANAGER_H__
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/extensions/XInput2.h>
#include<X11/extensions/shape.h>
#include<vector>
#include<map>
#include<stdio.h>
#include "logger.h"
#include "XConn.h"
#include "Config.h"
#include "WMWindow.h"
#include "PointerDevice.h"
#include "KeyboardDevice.h"
#include "DeviceError.h"
#include "XError.h"
#include "Dock.h"

#define BitIsOn(ptr, bit) (((unsigned char *) (ptr))[(bit)>>3] & (1 << ((bit) & 7)))
#define SetBit(ptr, bit)  (((unsigned char *) (ptr))[(bit)>>3] |= (1 << ((bit) & 7)))

using namespace std;
class WMWindow;
class PointerDevice;
class Dock;
class Config;

int MPWM_ErrorHandler(Display* dpy, XErrorEvent* ev);

class Manager
{
    public:
        long time;

    private:
        Config* config;
        XConn* x11;
        GC root_gc;
        vector<WMWindow*> windows;
        vector<PointerDevice*> pointers;
        vector<KeyboardDevice*> keyboards;

        bool stop;
        Pixmap pxBackground;
        Dock* dock;

        Cursor cursor;


    public:
        Manager(); 
        ~Manager();
        void init(char* display);
        void loop();
        void takedown();
        vector<PointerDevice*>* getPointers();

        bool hasShapeExtension();
        PointerDevice* idToPointerDevice(int id);
        KeyboardDevice* idToKeyboardDevice(int id);
        void setButtonPressMask(Window win);

    private:
        void initX11(char* display);
        void initXi();
        void initMPGXlib();
        void addNewClient();
        void queryInitial();
        vector<WMWindow*> queryWindows();
        bool isWMWindow(Window win);
        bool isWMDecoration(Window win);
        void hideCursor();
        void paintBackground(XExposeEvent* ev);
        WMWindow* windowToWMWindow(Window w);
        WMWindow* xyToWMWindow(int x, int y);
        void raiseWindow(WMWindow* wmwindow);

        /* event processing routines */
        void handleCreateNotify(XCreateWindowEvent* ev);
        void handleMapRequest(XMapEvent* ev);
        void handleMapNotify(XMapEvent* ev);
        void handleConfigureRequest(XConfigureRequestEvent* ev);
        void handleReparentNotify(XReparentEvent* ev);
        void handleDestroyNotify(XDestroyWindowEvent* ev);
        void handleUnmapNotify(XUnmapEvent* ev);
        void handleOtherEvents(XIEvent *ev);
        void handleMotionEvent(XIDeviceEvent* mev);
        void handleButtonPress(XIDeviceEvent* bev);
        void handleButtonRelease(XIDeviceEvent* bev);
        void handlePropertyNotify(XPropertyEvent* pev);
        void handleEnterLeaveNotify(XIEnterEvent* ev);
        void handleHierarchyEvent(XIHierarchyEvent* ev);

};

#endif
