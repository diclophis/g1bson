#ifndef __XCONN_H__
#define __XCONN_H__
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>

class XConn {
    public: 
        Display* dpy;
        int screen;
        Visual* vis;
        long black;
        long white;
        Window root;
        int depth;
        Colormap cmap;
        int width;
        int height;
        int XI_presence;
        XEventClass presence_class;

        Atom wm_protocols;
        Atom wm_delete_window;

    public:
        /* Connect to given X Server, fill with default variables */
        XConn(char* display);

        ~XConn();

    private:
        void connect(char* display);
        void getDefaults();
};

#endif
