/* $Id: XError.h,v 1.3 2006/06/16 07:36:54 whot Exp $ */

/*--
  --*/

#ifndef __XERROR_H__
#define __XERROR_H__
#include <string>

using namespace std;

class XError
{
    public:
    static const int NO_ERROR;
    static const int NO_DISPLAY;
    static const int UNKNOWN_WINDOW;

    protected:
    static const string MSG_NO_ERROR;
    static const string MSG_NO_DISPLAY;
    static const string MSG_UNKNOWN;
    static const string MSG_UNKNOWN_WINDOW;

    public:
        int error;
        string message;

   public:
        XError(int error);

};

#endif
