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
    /*
    XISetMask(mask1, XI_ButtonPress);
    XISetMask(mask1, XI_ButtonRelease);
    XISetMask(mask1, XI_KeyPress);
    XISetMask(mask1, XI_KeyRelease);

    evmasks[0].deviceid = XIAllMasterDevices;
    evmasks[0].mask_len = sizeof(mask1);
    evmasks[0].mask = mask1;
    */


    /* Select for motion from the default cursor */
    /*
    memset(mask2, 0, sizeof(mask2));
    XISetMask(mask2, XI_Motion);

    evmasks[1].deviceid = 2;
    evmasks[1].mask_len = sizeof(mask2);
    evmasks[1].mask = mask2;
    */

    XISetMask(mask1, XI_HierarchyChanged);
    evmasks[0].mask = mask1;
    evmasks[0].mask_len = sizeof(mask1);
    evmasks[0].deviceid = XIAllDevices;

    XISelectEvents(dpy, win, evmasks, 1);
    XFlush(dpy);
}

static void list_devices(Display *display, int deviceid)
{
    XIDeviceInfo *info, *dev;
    int ndevices;
    int i;
    const char *type = "";

    info = XIQueryDevice(display, deviceid, &ndevices);

    for(i = 0; i < ndevices; i++) {
        dev = &info[i];

        printf("'%s' (%d)\n", dev->name, dev->deviceid);
        switch(dev->use) {
            case XIMasterPointer: type = "master pointer"; break;
            case XIMasterKeyboard: type = "master keyboard"; break;
            case XISlavePointer: type = "slave pointer"; break;
            case XISlaveKeyboard: type = "slave keyboard"; break;
            case XIFloatingSlave: type = "floating slave"; break;
        }

        printf(" - is a %s\n", type);
        printf(" - current pairing/attachment: %d\n", dev->attachment);
        if (!dev->enabled)
            printf(" - this device is disabled!\n");
    }

    XIFreeDeviceInfo(info);
}

static void create_remove_master(Display *dpy, int xi_opcode)
{
    XIAddMasterInfo add;
    XIRemoveMasterInfo remove;
    XEvent ev;
    int new_master_id;

    add.type = XIAddMaster;
    add.name = "My new master";
    add.send_core = True;
    add.enable = True;

    XIChangeHierarchy(dpy, (XIAnyHierarchyChangeInfo*)&add, 1);
    XFlush(dpy);

    //printf("New master device:");
    //list_devices(dpy, add->deviceid);
    //new_master_id = add.deviceid;

    /*
    unsigned char mask[2] = { 0, 0 };
    XIEventMask evmask;

    evmask.mask = mask;
    evmask.mask_len = sizeof(mask);
    evmask.deviceid = XIAllDevices;


    XISetMask(mask, XI_HierarchyChanged);
    XISelectEvents(dpy, DefaultRootWindow(dpy), &evmask, 1);
    //XISelectEvents(dpy, 0, &evmask, 1);
    */

    while(1)
    {
        XGenericEventCookie *cookie = &ev.xcookie;

        printf("?\n");

        XNextEvent(dpy, (XEvent*)&ev);

        printf(".\n");

        if (cookie->type != GenericEvent ||
            cookie->extension != xi_opcode ||
            !XGetEventData(dpy, cookie)) {
          printf("cont\n");
          continue;
        }

        if (cookie->evtype == XI_HierarchyChanged)
        {
        printf("wtf\n");
            XIHierarchyEvent *event = cookie->data;
            if ((event->flags & XIMasterAdded))
            {
                int i;
                XIHierarchyInfo *info;

                for (i = 0; i < event->num_info; i++)
                {
                    info = &event->info[i];
                    if (info->flags & XIMasterAdded)
                    {
                        printf("New master device:");
                        list_devices(dpy, info->deviceid);

                        new_master_id = info->deviceid;
                    }
                }

                XFreeEventData(dpy, cookie);
                break;
            }
        } else {
          printf(".");
        }

        XFreeEventData(dpy, cookie);
    }


    printf("Removing new master devices again.\n");

    remove.type = XIRemoveMaster;
    remove.deviceid = new_master_id;
    remove.return_mode = XIAttachToMaster;
    remove.return_pointer = 2; /* VCP */
    remove.return_keyboard = 3; /* VCK */

    XIChangeHierarchy(dpy, (XIAnyHierarchyChangeInfo*)&remove, 1);
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

win = create_win(dpy);

    select_events(dpy, win);

    if (1) {
      list_devices(dpy, XIAllDevices);
      create_remove_master(dpy, xi_opcode);
    }

    if (0) {
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
    }

    return 0;
}

