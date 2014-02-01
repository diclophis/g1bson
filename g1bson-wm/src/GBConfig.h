/* $Id: Config.h,v 1.13 2006/10/24 07:49:50 whot Exp $ */

/*--
  --*/

#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <X11/Xlib.h>
#include <vector>
#include "ConfigurationError.h"
#include "logger.h"
#include "XConn.h"
#include "DockApp.h"

using namespace std;

#ifndef IMAGEPATH
#define IMAGEPATH "../images/"
#endif

class DockApp;

class Config
{
    private: 
        XConn* x11;

    public:
        char* imgBackground;
        char* kbdImage;
	char* crsImage;

	int idXOffset;
	int idYOffset;
	int idFontSize;

        int clientOffset;
        int windowBarHeight;
        int resizeBarHeight;

        int resizeButtonWidth;
        int resizeButtonHeight;

        int buttonWidth;
        int buttonHeight;

        int buttonCloseX;
        int buttonCloseY;

        int buttonFloorX;
        int buttonFloorY;

        int buttonOverlayX;
        int buttonOverlayY;

        int buttonOwnerX;
        int buttonOwnerY;

        int buttonMinimizeX;
        int buttonMinimizeY;

        int drawLineWidth;
        int eraseLineWidth;

        vector<DockApp*> apps;

    protected:
        static Config* instance;

        
    public: 
        static Config* getInstance();
        static void init(XConn* x11);

        long cursorColor(int cursor);

    protected:
        Config(XConn* x11);


};

#endif
