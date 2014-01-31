/* $Id: PointerDevice.cpp,v 1.17 2007/01/07 05:00:56 whot Exp $ */

/*--
 --*/

#include "PointerDevice.h"
#include <X11/Xcursor/Xcursor.h>
#include <stdlib.h>

int PointerDevice::counter = 0;

PointerDevice::PointerDevice(XIDeviceInfo* dev, XConn* x11, Manager* manager)
{
    this->x11 = x11;
    this->pointerID = PointerDevice::counter++;
    TRACE("Creating pointer %d\n", pointerID);
    this->name = string(dev->name);
    this->id = dev->deviceid;

    this->dragWindow = NULL;
    this->resizeWindow = NULL;

    color = Config::getInstance()->cursorColor(pointerID);

    generatePointerImage(dev->deviceid);

    Cursor cursor = XcursorFilenameLoadCursor(x11->dpy, "/tmp/.mpwm_pointer.cur");
    XIDefineCursor(x11->dpy, dev->deviceid, x11->root, cursor);

    TRACE("Device %d (%s) initialised\n",
            (unsigned int)dev->deviceid, dev->name);
}

/**
 * Move the client's window to the given position.
 */
void PointerDevice::dragTo(int x, int y)
{
    if (dragWindow != NULL)
        dragWindow->move(x - dragOffset[0] , y - dragOffset[1]);
    else
        ERR("DragTo where? No window given.\n");
}

void PointerDevice::dragOff()
{
    if (!dragWindow)
    {
        ERR("Drag off when no window was dragged.\n");
        return;
    }

    dragWindow->releaseController();
    dragWindow = NULL;
    dragOffset[0] = dragOffset[1] = 0;

    XIUngrabDevice(x11->dpy, this->id, CurrentTime);
    TRACE("Ungrabbing device.\n");
    XFlush(x11->dpy);

}

bool PointerDevice::isDragging()
{
    return (dragWindow != NULL);
}

/**
 * Switches dragging on for the given window. 
 * The coordinates are used for the offset to maintain the window's relative
 * position to the mouse cursor.
 */
bool PointerDevice::dragOn(WMWindow* win, int x, int y)
{
    if (win == NULL)
    {
        ERR("NULL window given for drag\n");
        return false;
    }

    if (win->setController(this))
    {
        XIEventMask mask;
        unsigned char bits[4] = {0};

        mask.mask_len = sizeof(bits);
        mask.deviceid = this->id;
        mask.mask = bits;
        SetBit(bits, XI_ButtonPress);
        SetBit(bits, XI_Motion);
        SetBit(bits, XI_ButtonRelease);

        dragWindow = win;
        dragOffset[0] = x;
        dragOffset[1] = y;

        if (XIGrabDevice(x11->dpy, this->id, win->getWindowBar(), CurrentTime,
                         None, GrabModeAsync, GrabModeAsync, False, &mask))
            TRACE("Grab failed\n");
        return true;
    }

    return false;
}

bool PointerDevice::resizeOn(WMWindow* win, Window button, int x, int y)
{
    if (win == NULL)
    {
        ERR("NULL window given for resize\n");
        return false;
    }

    if (win->addResizer(this))
    {
        TRACE("Device %s is resizing\n", name.c_str());
        resizeWindow = win;
        resizeOffset[0] = x;
        resizeOffset[1] = y;
        resizeButton = button;
        XIEventMask mask;
        unsigned char bits[4] = {0};

        mask.mask_len = sizeof(bits);
        mask.deviceid = this->id;
        mask.mask = bits;
        SetBit(bits, XI_ButtonPress);
        SetBit(bits, XI_ButtonRelease);
        SetBit(bits, XI_Motion);

        XIGrabDevice(x11->dpy, this->id, win->getWindowBar(), CurrentTime,
                     None, GrabModeAsync, GrabModeAsync, False, &mask);
        return true;
    }

    return false;
}

void PointerDevice::resizeOff()
{
    if (!resizeWindow)
    {
        ERR("Resize off when no window was being resized.\n");
        return;
    }
    
    resizeWindow->releaseResizer(this);
    resizeWindow = NULL;
    resizeOffset[0] = resizeOffset[1] = 0;

    XIUngrabDevice(x11->dpy, this->id, CurrentTime);
}

bool PointerDevice::isResizing()
{
    return resizeWindow != NULL;
}

void PointerDevice::resizeTo(int x, int y)
{
    if (resizeWindow != NULL)
    {
        resizeWindow->resizeDirected(resizeButton, 
                                     x - resizeOffset[0] , 
                                     y - resizeOffset[1]); 
        resizeOffset[0] = x;
        resizeOffset[1] = y;
    }
    else
        ERR("resizeTo where? No window given.\n");
}

/**
 * Enables device events on the given windows. 
 */
void PointerDevice::setWMEvents(WMWindow* window)
{
    XIEventMask mask;
    unsigned char bits[4] = {0};
    XIGrabModifiers modifiers[4] = {0};
    int failed = 0;

    modifiers[0].modifiers = 0;
    modifiers[1].modifiers = 0x1;
    modifiers[2].modifiers = 0x10;
    modifiers[3].modifiers = 0x11;

    mask.mask = bits;
    mask.mask_len = sizeof(bits);
    mask.deviceid = XIAllMasterDevices;
    SetBit(bits, XI_ButtonPress);

    XISelectEvents(x11->dpy, window->getButtonMinimize(), &mask, 1);
    XISelectEvents(x11->dpy, window->getResizeBtNE(), &mask, 1);
    XISelectEvents(x11->dpy, window->getResizeBtNW(), &mask, 1);
    XISelectEvents(x11->dpy, window->getResizeBtSE(), &mask, 1);
    XISelectEvents(x11->dpy, window->getResizeBtSW(), &mask, 1);

    SetBit(bits, XI_ButtonRelease);
    XISelectEvents(x11->dpy, window->getButtonClose(), &mask, 1);

    SetBit(bits, XI_Motion);
    XISelectEvents(x11->dpy, window->getWindowBar(), &mask, 1);
    XISelectEvents(x11->dpy, window->getResizeBar(), &mask, 1);

    XFlush(x11->dpy);
    TRACE("Events set on windows %x, %x\n", (int)window->getWindowBar(),
            (int)window->getButtonClose());


    failed = XIGrabButton(x11->dpy, this->id, Button1, window->getClientWindow(),
                          None, GrabModeSync, GrabModeAsync, False, &mask, 4,
                          modifiers);

    for (int i= 0; i < failed; i++)
        ERR("Modifiers %#x failed\n", modifiers[i]);
}

void PointerDevice::setDockEvents(Window win)
{
    XIEventMask mask;
    unsigned char bits[4] = {0};

    mask.mask = bits;
    mask.mask_len = sizeof(bits);
    mask.deviceid = XIAllMasterDevices;
    SetBit(bits, XI_ButtonPress);

    XISelectEvents(x11->dpy, win, &mask, 1);
}

void PointerDevice::generatePointerImage(int number){
    int bare_cursor_width,
        bare_cursor_height,
        total_width,
        total_height;
    stringstream s;
    s << number;
    std::string text = std::string(s.str());

    cairo_surface_t* dummy_surface;
    cairo_surface_t* main_surface;
    cairo_t* cr;

    TRACE("Generating cursor from %s\n",Config::getInstance()->crsImage);
    cairo_surface_t* png_cursor = 
	    cairo_image_surface_create_from_png(Config::getInstance()->crsImage);
    bare_cursor_width = cairo_image_surface_get_width(png_cursor);
    bare_cursor_height = cairo_image_surface_get_height(png_cursor);
    
    dummy_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,10,10);
    cr = cairo_create(dummy_surface);
    cairo_text_extents_t est;
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, 
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, Config::getInstance()->idFontSize);

    cairo_text_extents(cr,text.c_str(),&est);
    TRACE("Generated ID pointer text (%2.0fx%2.0f)\n",est.width,est.height);

    total_width = (int)(Config::getInstance()->idXOffset + est.width + est.x_bearing);	
    total_height = (int)(Config::getInstance()->idYOffset + est.height + est.y_bearing);	

    main_surface = cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32,
            total_width,
            total_height
            );

    cr = cairo_create(main_surface);
    cairo_set_source_surface(cr,png_cursor,0,0);
    cairo_paint (cr);

    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, 
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, Config::getInstance()->idFontSize);

    cairo_set_source_rgb(cr,255,0,0);
    cairo_move_to(cr,Config::getInstance()->idXOffset,Config::getInstance()->idYOffset);
    cairo_show_text(cr,text.c_str());
    
    cairo_surface_write_to_png(main_surface,"/tmp/.mpwm_pointer.png");
    
    cairo_destroy(cr);
    cairo_surface_destroy(dummy_surface);
    cairo_surface_destroy(main_surface);
    cairo_surface_destroy(png_cursor);

    /*FIXME: I should use internal API*/
    /*/usr/include/X11/Xcursor/Xcursor.h*/
    /*XCursorImage* XcursorFileLoadImage(FILE* file,int size)*/
    /*Cursor XcursorImageLoadCursor(Display* dpy,const XcursorImage* image)*/
    system("echo \"24 0 0 /tmp/.mpwm_pointer.png \" > /tmp/.mpwm_pointer.cfg");
    system(XCURSORGEN" /tmp/.mpwm_pointer.cfg /tmp/.mpwm_pointer.cur");
}
