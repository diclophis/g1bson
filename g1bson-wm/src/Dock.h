/* $Id: Dock.h,v 1.5 2007/01/07 05:00:56 whot Exp $ */

#ifndef __DOCK_H__
#define __DOCK_H__

#include <vector>
#include <cairo/cairo.h>
#include "XConn.h"
#include "Config.h"
#include "DockApp.h"
#include "DockProcess.h"
#include "DockKeyboard.h"

using namespace std;

const int DOCK_BUTTON_WIDTH = 32;
const int DOCK_BUTTON_HEIGHT = 32;
/* 
   Size of the dock is always applications * dock button size
   the following two decide how much bigger (in px) the dock should be than
   the space occupied by the buttons. Space will be divided equally to left
   and right of the buttons. Height will be above the buttons.
 */
const int DOCK_WIDTH_EXTENDED = 10;
const int DOCK_HEIGHT_EXTENDED = 5;

class DockProcess;
class DockApp;
class PointerDevice;

class Dock 
{
    private:
        XConn* x11;
        Window dock;
        Pixmap backbuff;
        GC gc_dock;
        vector<DockApp*> apps; /* apps to start app */
        vector<DockProcess*> processes; /* minimized processes */
        vector<DockKeyboard*> keyboards; /* connected keyboards */

        int x;
        int y;
        int width;
        int height;
        
        cairo_t* cr;

    public:
        Dock(XConn* x11);
        ~Dock();
        bool hasWindow(Window win);
        void handleExpose(XExposeEvent* ev);
        void appendProcess(WMWindow* win);
        void appendKeyboard(KeyboardDevice* kbd);
        void removeProcess(DockProcess* dp);
        void setPointerEvents(vector<PointerDevice*>* pointers);
        DockItem* getDockItem(Window win);

    private:
        void repaint();
};

#endif
