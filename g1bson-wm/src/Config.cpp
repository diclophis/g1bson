/* $Id: Config.cpp,v 1.15 2006/10/25 08:04:52 whot Exp $ */

/*--
  --*/

#include "Config.h"

Config* Config::instance = NULL;

/**
 * @return The instance of the configuration.
 * @throws ConfigurationError if the config was not initialized before.
 * @see Config::init
 */
Config* Config::getInstance()
{
    if (instance == NULL)
        throw new ConfigurationError();

    return instance;
}


/**
 * Call this method to initialize the WM config. Otherwise getInstance() will
 * throw an error.
 * @param dpy The X11 display.
 * @param root X11 root window.
 * @param screen The used screen.
 */
void Config::init(XConn* x11)
{
    Config::instance = new Config(x11);
}


/**
 * This is a protected constructor. Do not call it. There can only be one
 * configuration per window manager.
 * Eventually this will load data from a file.
 * @param dpy The X11 display.
 * @param root X11 root window.
 * @param screen The used screen.
 */
Config::Config(XConn* x11)
{

    this->x11 = x11;
    clientOffset = 3;
    imgBackground = IMAGEPATH "background.png";
    kbdImage = IMAGEPATH "keyboard.png";
    crsImage = IMAGEPATH "bare_cursor.png";
    windowBarHeight = 30;

    idXOffset = 11;
    idYOffset = 25;
    idFontSize = 18;

    buttonWidth = 20;
    buttonHeight = 10;

    resizeBarHeight = 20;

    resizeButtonHeight = resizeBarHeight;
    resizeButtonWidth = 20;

    buttonCloseX = 50;
    buttonCloseY= 10;

    buttonMinimizeX = buttonCloseX + 30;
    buttonMinimizeY = buttonCloseY;

    buttonFloorX = buttonMinimizeX + 30;
    buttonFloorY = buttonCloseY;

    buttonOverlayX = buttonFloorX + 30;
    buttonOverlayY = buttonFloorY;

    buttonOwnerX = buttonOverlayX + 30;
    buttonOwnerY = buttonFloorY;

    // pencil sizes for overlay
    drawLineWidth = 4;
    eraseLineWidth = 25;

    /*
       The applications for the dock.
     */
    apps.push_back(new DockApp(x11, "firefox &", IMAGEPATH "firefox.png"));
    apps.push_back(new DockApp(x11, "gnome-terminal &", IMAGEPATH "terminal.png"));
    apps.push_back(new DockApp(x11, "gnome-calculator &", IMAGEPATH "calculator.png"));
    apps.push_back(new DockApp(x11, "abiword &", IMAGEPATH "abiword.png"));
    apps.push_back(new DockApp(x11, "gimp --no-splash &", IMAGEPATH "gimp.png"));
}


static void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v );
long Config::cursorColor(int id)
{
    float r, g, b;


    HSVtoRGB(&r, &g, &b, (85 * id) % 360, 1, 1);

    XColor color;
    color.red = (int)(65535 * r);
    color.blue = (int)(65535 * g);
    color.green = (int)(65535 * b);


    TRACE("allocating %d/%d/%d for pointer %d\n", color.red, color.green, color.blue, id);
    
    long pixel = XAllocColor(x11->dpy, x11->cmap, &color);

    TRACE(" - result %d/%d/%d (pixel %ld)\n", color.red, color.green, color.blue, color.pixel);

    if (!pixel)
    {
        ERR(" -cannot allocate color\n");
        pixel = x11->white;
    }

    return color.pixel;

}


/* Taken from http://www.cs.rit.edu/~ncs/color/t_convert.html */
static void HSVtoRGB( float *r, float *g, float *b, 
        float h, float s, float v )
{
    int i;
    float f, p, q, t;

    if( s == 0 ) {
        // achromatic (grey)
        *r = *g = *b = v;
        return;
    }

    h /= 60;// sector 0 to 5
    i = (h >= 0) ? (int)h : (int)(h - 1);
    f = h - i;// factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );

    switch( i ) {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        default:// case 5:
            *r = v;
            *g = p;
            *b = q;
            break;
    }

}


