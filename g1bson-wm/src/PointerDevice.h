/* $Id: PointerDevice.h,v 1.11 2007/01/07 05:00:56 whot Exp $ */

/*--
  --*/

#ifndef __POINTERDEVICE_H__
#define __POINTERDEVICE_H__

#include<X11/Xlib.h>
#include<X11/extensions/XInput2.h>
#include<map>
#include<string>
#include<sstream> 
#include "DeviceError.h"
#include "Config.h"
#include "logger.h"
#include "WMWindow.h"
#include "XConn.h"

using namespace std;

class WMWindow;
class Manager;

class PointerDevice
{
    public:
        static int counter;
    private:
        int pointerID;
        XConn* x11;

        int id;
        string name;

        WMWindow* dragWindow;
        int dragOffset[2];

        WMWindow* resizeWindow;
        int resizeOffset[2];
        Window resizeButton;

        long color;

    public:
        PointerDevice(XIDeviceInfo* device, XConn* x11, Manager* manager);
        void raise();
        int getID() { return id; }
        string getName() { return name; }

        bool isDragging();
        bool dragOn(WMWindow* win, int x, int y);
        void dragOff();
        void dragTo(int x, int y);

        bool isResizing();
        bool resizeOn(WMWindow* win, Window button, int x, int y);
        void resizeOff();
        void resizeTo(int x, int y);

        void setWMEvents(WMWindow* client);
        void setDockEvents(Window win);
        long getColor() { return color; }

    private:
        void generatePointerImage(int number);
};

#endif
