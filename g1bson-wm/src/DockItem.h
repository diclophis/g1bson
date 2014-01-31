/* $Id: DockItem.h,v 1.3 2007/01/07 05:00:56 whot Exp $ */

#ifndef __DOCKITEM_H__
#define __DOCKITEM_H__

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include "XConn.h"

class PointerDevice;
class DockItem 
{
    protected:
        XConn* x11;
        Window button;
        GC button_gc;
        Pixmap backbuff;
        int width, height;

    public:
        void initGUI(Window parent, int x, int y, int width, int height);
        bool hasWindow(Window win);
        void handleExpose(XExposeEvent *ev);
        virtual void handleButtonEvent(PointerDevice* ptr, XIDeviceEvent* ev) = 0;
        virtual void setup() = 0;
        virtual ~DockItem();
        Window getButton() { return button; }

        void move(int x, int y);

};

#endif

