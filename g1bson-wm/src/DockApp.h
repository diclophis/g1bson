/* $Id: DockApp.h,v 1.4 2007/01/07 05:00:56 whot Exp $ */

#ifndef __DOCKAPP_H__
#define __DOCKAPP_H__

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include "XConn.h"
#include "DockItem.h"
#include "PointerDevice.h"

class DockApp : public DockItem
{
    private:
        const char* app;
        char* imgfile;

    public:
        DockApp(XConn* x11, const char* app, char* imgfile);
        void handleButtonEvent(PointerDevice* ptr, XIDeviceEvent* ev);
        void setup();

};

#endif
