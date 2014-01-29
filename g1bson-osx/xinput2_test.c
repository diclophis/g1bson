/* gcc -o part1 `pkg-config --cflags --libs xi` part1.c */
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

static Window create_win(Display *dpy)
{
    Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 200,
            200, 0, 0, WhitePixel(dpy, 0));
    Window subwindow = XCreateSimpleWindow(dpy, win, 50, 50, 50, 50, 0, 0,
            BlackPixel(dpy, 0));

    XMapWindow(dpy, subwindow);
    XMapWindow(dpy, win);
    XSync(dpy, False);
    return win;
}


/* Return 1 if XI2 is available, 0 otherwise */
static int has_xi2(Display *dpy)
{
    int major, minor;
    int rc;

    /* We support XI 2.0 */
    major = 2;
    minor = 0;

    rc = XIQueryVersion(dpy, &major, &minor);
    if (rc == BadRequest) {
        printf("No XI2 support. Server supports version %d.%d only.\n", major, minor);
        return 0;
    } else if (rc != Success) {
        fprintf(stderr, "Internal Error! This is a bug in Xlib.\n");
    }

    printf("XI2 supported. Server provides version %d.%d.\n", major, minor);

    return 1;
}

static void select_events(Display *dpy, Window win)
{
    XIEventMask evmasks[2];
    unsigned char mask1[(XI_LASTEVENT + 7)/8];
    unsigned char mask2[(XI_LASTEVENT + 7)/8];

    memset(mask1, 0, sizeof(mask1));

    /* select for button and key events from all master devices */
    XISetMask(mask1, XI_ButtonPress);
    XISetMask(mask1, XI_ButtonRelease);
    XISetMask(mask1, XI_KeyPress);
    XISetMask(mask1, XI_KeyRelease);

    evmasks[0].deviceid = XIAllMasterDevices;
    evmasks[0].mask_len = sizeof(mask1);
    evmasks[0].mask = mask1;

    memset(mask2, 0, sizeof(mask2));

    /* Select for motion from the default cursor */
    XISetMask(mask2, XI_Motion);

    evmasks[1].deviceid = 2; /* the default cursor */
    evmasks[1].mask_len = sizeof(mask2);
    evmasks[1].mask = mask2;

    XISelectEvents(dpy, win, evmasks, 2);
    XFlush(dpy);
}

int main (int argc, char **argv)
{
    Display *dpy;
    int xi_opcode, event, error;
    Window win;
    XEvent ev;

    dpy = XOpenDisplay("localhost:0");

    if (!dpy) {
        fprintf(stderr, "Failed to open display.\n");
        return -1;
    }

    if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error)) {
           printf("X Input extension not available.\n");
              return -1;
    }

    if (!has_xi2(dpy))
        return -1;

    /* Create a simple window */
    win = create_win(dpy);

    /* select for XI2 events */
    select_events(dpy, win);

    while(1) {
        XGenericEventCookie *cookie = &ev.xcookie;

        XNextEvent(dpy, &ev);

        if (cookie->type != GenericEvent ||
            cookie->extension != xi_opcode)
            continue;

        if (XGetEventData(dpy, cookie))
        {
            printf("Event type %d received\n", cookie->evtype);
            XFreeEventData(dpy, &ev.xcookie);
        }
    }

    return 0;
}

