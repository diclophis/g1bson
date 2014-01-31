#ifndef __DOCKKEYBOARD_H__
#define __DOCKKEYBOARD_H__

#include <X11/Xlib.h>
#include "XConn.h"
#include "DockItem.h"
#include "KeyboardDevice.h"

class PointerDevice;

class DockKeyboard : public DockItem
{
    private:
        KeyboardDevice* kbd;

    public:
        DockKeyboard(XConn* x11, KeyboardDevice* kbd);
        void handleButtonEvent(PointerDevice* ptr, XIDeviceEvent* ev);
        void setup();
};


#endif
