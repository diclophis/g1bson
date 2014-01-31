/* $Id: XError.cpp,v 1.3 2006/06/16 07:36:54 whot Exp $ */

/*--
  --*/

#include "XError.h"

const int XError::NO_ERROR = 0;
const int XError::NO_DISPLAY = 1;
const int XError::UNKNOWN_WINDOW = 2;

const string XError::MSG_NO_DISPLAY("Could not connect to display.");
const string XError::MSG_NO_ERROR("No error");
const string XError::MSG_UNKNOWN("Unknown error.");
const string XError::MSG_UNKNOWN_WINDOW("Unknown window.");

XError::XError(int errnum)
{
    switch(errnum)
    {
        case XError::NO_ERROR:
            message = MSG_NO_ERROR;
            break;
        case XError::NO_DISPLAY:
            message = MSG_NO_DISPLAY;
            break;
        case XError::UNKNOWN_WINDOW:
            message = MSG_UNKNOWN_WINDOW;
            break;
        default:
            message = MSG_UNKNOWN;
            break;
    }
}
