/* $Id: DeviceError.h,v 1.3 2006/06/16 07:36:54 whot Exp $ */

/*--
 --*/

#ifndef __DEVICEERROR_H__
#define __DEVICEERROR_H__

#include<string>

using namespace std;

class DeviceError
{
    public:
    static const int NO_ERROR;
    static const int NO_DEVICES;
    static const int OPEN_FAILED;
    static const int ALREADY_ASSIGNED;

    protected: 
    static const string MSG_NO_ERROR;
    static const string MSG_NO_DEVICES;
    static const string MSG_OPEN_FAILED;
    static const string MSG_UNKNOWN;
    static const string MSG_ALREADY_ASSIGNED;

    public:
        int error;
        string message;

    public:
        DeviceError(int num);
};

#endif
