
#include "KeyboardDevice.h"

int KeyboardDevice::counter = 0;

KeyboardDevice::KeyboardDevice(XIDeviceInfo* dev, XConn* x11, Manager* manager)
{
    this->x11 = x11;
    this->kbdID = KeyboardDevice::counter++;
    this->name = string(dev->name);
    this->id = dev->deviceid;
    this->paired = NULL;
    TRACE("Keyboard %d (%s) initialised\n", kbdID, dev->name);
}
