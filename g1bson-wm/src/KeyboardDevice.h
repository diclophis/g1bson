#ifndef __KEYBOARDDEVICE_H__
#define __KEYBOARDDEVICE_H__

#include<X11/Xlib.h>
#include<X11/extensions/XInput2.h>
#include<string>
#include<vector>
#include "DeviceError.h"
#include "logger.h"
#include "XConn.h"

using namespace std;

class Manager;
class PointerDevice;

class KeyboardDevice
{
    public:
        static int counter;

    private:
        int kbdID;
        XConn* x11;
        int id;
        string name;
        XDevice* dev;
        PointerDevice* paired;

    public:
        KeyboardDevice(XIDeviceInfo* device, XConn* x11, Manager* manager);
        int getID() { return id; }
        string getName() { return name; }
        void setPaired(PointerDevice* ptr) { paired = ptr; }
        PointerDevice* getPaired() { return paired; }
};

#endif
