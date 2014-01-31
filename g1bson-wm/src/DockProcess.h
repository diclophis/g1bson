/* $Id: DockProcess.h,v 1.3 2007/01/07 05:00:56 whot Exp $ */

#ifndef __DOCKPROCESS_H__
#define __DOCKPROCESS_H__

#include <X11/Xlib.h>
#include "XConn.h"
#include "Dock.h"
#include "DockItem.h"
#include "WMWindow.h"

class WMWindow;
class PointerDevice;
class Dock;

/* An app that was minimized into the dock */
class DockProcess : public DockItem
{
    private:
        WMWindow* client;
        Dock* dock;

    public:
        DockProcess(XConn* x11, WMWindow* client, Dock* dock);
        void handleButtonEvent(PointerDevice* ptr, XIDeviceEvent* ev);
        void setup();

};

#endif
