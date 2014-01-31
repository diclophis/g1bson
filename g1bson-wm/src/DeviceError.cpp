/* $Id: DeviceError.cpp,v 1.3 2006/06/16 07:36:54 whot Exp $ */

/*--
  --*/

#include "DeviceError.h"

const int DeviceError::NO_ERROR = 0;
const int DeviceError::NO_DEVICES = 1;
const int DeviceError::OPEN_FAILED = 2;
const int DeviceError::ALREADY_ASSIGNED = 4;

const string DeviceError::MSG_NO_ERROR("No error.");
const string DeviceError::MSG_NO_DEVICES("No Input extension devices found");
const string DeviceError::MSG_OPEN_FAILED("Device open failed.");
const string DeviceError::MSG_ALREADY_ASSIGNED("Another device has already been assigned for this task.");
const string DeviceError::MSG_UNKNOWN("Unknown error");

DeviceError::DeviceError(int errnum)
{
    switch(errnum)
    {
        case DeviceError::NO_ERROR:
            message = MSG_NO_ERROR;
            break;
        case DeviceError::NO_DEVICES:
            message = MSG_NO_DEVICES;
            break;
        case DeviceError::OPEN_FAILED:
            message = MSG_OPEN_FAILED;
            break;
        case DeviceError::ALREADY_ASSIGNED:
            message = MSG_ALREADY_ASSIGNED;
            break;
        default:
            message = MSG_UNKNOWN;
            break;
    }
}
