#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysym.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>

unsigned long  _pid;
Atom           _atomPID;
Display       *_display;
Window        proofWin;


Window g1bson_create_window(Display *dpy) {
  Window win = XCreateSimpleWindow(
    dpy, DefaultRootWindow(dpy), 0, 0, 200,
    200, 0, 0, WhitePixel(dpy, 0));

  Window subwindow = XCreateSimpleWindow(
    dpy, win, 50, 50, 50, 50, 0, 0,
    BlackPixel(dpy, 0));

  XMapWindow(dpy, subwindow);
  XMapWindow(dpy, win);
  XSync(dpy, False);
  return win;
}


// Return 1 if XI2 is available, 0 otherwise
int g1bson_has_xi2(Display *dpy) {
  int major, minor;
  int rc;

  // We support XI 2.0
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


void g1bson_select_events(Display *dpy, Window win) {
  XIEventMask evmasks[2];
  unsigned char mask1[(XI_LASTEVENT + 7)/8];
  unsigned char mask2[(XI_LASTEVENT + 7)/8];

  memset(mask1, 0, sizeof(mask1));

  /* select for button and key events from all master devices */
  if (0) {
    XISetMask(mask1, XI_ButtonPress);
    XISetMask(mask1, XI_ButtonRelease);
    XISetMask(mask1, XI_KeyPress);
    XISetMask(mask1, XI_KeyRelease);
    evmasks[0].deviceid = XIAllMasterDevices;
    evmasks[0].mask_len = sizeof(mask1);
    evmasks[0].mask = mask1;
  }

    /* Select for motion from the default cursor */
  if (0) {
    memset(mask2, 0, sizeof(mask2));
    XISetMask(mask2, XI_Motion);

    evmasks[1].deviceid = 2;
    evmasks[1].mask_len = sizeof(mask2);
    evmasks[1].mask = mask2;
  }

  if (1) {
    XISetMask(mask1, XI_HierarchyChanged);
    evmasks[0].mask = mask1;
    evmasks[0].mask_len = sizeof(mask1);
    evmasks[0].deviceid = XIAllDevices;

    XISelectEvents(dpy, win, evmasks, 1);
  }

  XFlush(dpy);
}


void g1bson_list_devices(Display *display, int deviceid) {
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
    if (!dev->enabled) {
      printf(" - this device is disabled!\n");
    }
  }

  XIFreeDeviceInfo(info);
}


void g1bson_create_remove_master(Display *dpy, int xi_opcode) {
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
                      g1bson_list_devices(dpy, info->deviceid);

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


void g1bson_add_mpx_for_window (
  Display *dsp,
  char *name,
  int *master_kbd,
  int *slave_kbd,
  int *master_ptr,
  int *slave_ptr
) {
  XIAddMasterInfo add;
  int ndevices;
  XIDeviceInfo *devices, *device;
  int i;

  // add the device

  add.type = XIAddMaster;
  add.name = name;
  add.send_core = True;
  add.enable = True;

  XIChangeHierarchy (dsp,
                     (XIAnyHierarchyChangeInfo*) &add,
                     1);

  // now see whether it's in the list

  *master_kbd = -1;
  *slave_kbd = -1;
  *master_ptr = -1;

  devices = XIQueryDevice(dsp,
                          XIAllDevices, &ndevices);

  for (i = 0; i < ndevices; i++) {
    device = &devices[i];

    int j;
    int m = 1;
    for (j = 0; j<strlen(name); j++) {
      if (device->name[j] != name[j]) {
        m = 0;
      }
    }

    if (m) {
        switch (device->use)
          {
          case XIMasterKeyboard:
            *master_kbd = device->deviceid;
            break;

          case XISlaveKeyboard:
            *slave_kbd = device->deviceid;
            break;

          case XIMasterPointer:
            *master_ptr = device->deviceid;
            break;

          case XISlavePointer:
            *slave_ptr = device->deviceid;
            break;
          }
      }
  }

  if (*master_kbd==-1 || *slave_kbd==-1 || *master_ptr==-1 || *slave_ptr==-1)
  {
    printf ("The new pointer '%s' could not be created.\n", name);
  }

  XIFreeDeviceInfo(devices);
}


void g1bson_drop_mpx (Display *dsp, int mpx) {
  XIRemoveMasterInfo drop;

  drop.type = XIRemoveMaster;
  drop.deviceid = mpx;
  drop.return_mode = XIAttachToMaster;
  drop.return_pointer = 2; // keyboard
  drop.return_keyboard = 3; // mouse

  XIChangeHierarchy (dsp,
                     (XIAnyHierarchyChangeInfo*) &drop,
                     1);
  XFlush(dsp);
}


void g1bson_fake_keystroke (
Display *dsp, int window_id, char symbol,
int xid_master_kbd, int xid_slave_kbd,
int xid_master_ptr, int xid_slave_ptr
) {

  printf("typing %d\n", window_id);

  int code = XKeysymToKeycode (dsp, symbol);

  int dummy[1] = { 0 };

  int current_pointer;
  XDevice *dev;

  dev = XOpenDevice (dsp,
                     xid_slave_kbd);

  XIGetClientPointer (dsp,
                      None,
                      &current_pointer);

  XISetClientPointer (dsp,
                      None,
                      xid_master_ptr);

  XSetInputFocus (dsp,
                  (Window) window_id, PointerRoot,
                  CurrentTime);

  if (XTestFakeDeviceKeyEvent (dsp,
                               dev,
                               code,
                               True,
                               dummy, 0, CurrentTime)==0)
  {
    printf ("Faking key event failed.\n");
  }

  XFlush (dsp);

  if (XTestFakeDeviceKeyEvent (dsp,
                                   dev,
                                   code,
                                   False,
                                   dummy, 0, CurrentTime)==0)
  {
    printf ("Faking key event failed 2.\n");
  }
  XFlush (dsp);

  XISetClientPointer (dsp,
                      None,
                      current_pointer);
  
  XCloseDevice (dsp,
                dev);

  XFlush (dsp);
}


void g1bson_search(Window w) {
  // Get the PID for the current Window.
  Atom           type;
  int            format;
  unsigned long  nItems;
  unsigned long  bytesAfter;
  unsigned char *propPID = 0;
  if(Success == XGetWindowProperty(_display, w, _atomPID, 0, 1, False, XA_CARDINAL,
                                   &type, &format, &nItems, &bytesAfter, &propPID))
  {
    if(propPID != 0)
    {

      printf("PID: %lu\n", (*((unsigned long *)propPID)));
      proofWin = w;

      XFree(propPID);
    }
  }

  // Recurse into child windows.
  Window    wRoot;
  Window    wParent;
  Window   *wChild;
  unsigned  nChildren;
  if(0 != XQueryTree(_display, w, &wRoot, &wParent, &wChild, &nChildren))
  {
    unsigned i;
    for(i = 0; i < nChildren; i++) {
      g1bson_search(wChild[i]);
    }
  }
}


void g1bson_windows_matching_pid(Display *display, Window wRoot, unsigned long pid) {
  // Get the PID property atom.
  _atomPID = XInternAtom(display, "_NET_WM_PID", True);
  if(_atomPID == None)
  {
    printf("No such atom\n");
    return;
  }

  _display = display;
  _pid = pid;

  g1bson_search(wRoot);
}


int main(int argc, char **argv) {
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

  if (!g1bson_has_xi2(dpy)) {
    return -1;
  }

  if (1) {
    int xid_master_kbd, xid_slave_kbd,
        xid_master_ptr, xid_slave_ptr;
    int current_pointer;

    g1bson_add_mpx_for_window (dpy, "test", // name must be uniq
      &xid_master_kbd,
      &xid_slave_kbd,
      &xid_master_ptr,
      &xid_slave_ptr);

    Screen *screen = XDefaultScreenOfDisplay(dpy);
    Window rootWindow = XRootWindowOfScreen(screen);

    g1bson_windows_matching_pid(dpy, rootWindow, 0);

    g1bson_fake_keystroke(dpy, proofWin, 'X', xid_master_kbd, xid_slave_kbd, xid_master_ptr, xid_slave_ptr);

    g1bson_drop_mpx(dpy, xid_master_kbd);

  }

  if (0) {
    /* Create a simple window */
    win = g1bson_create_window(dpy);

    /* select for XI2 events */
    g1bson_select_events(dpy, win);

    while(1) {
      XGenericEventCookie *cookie = &ev.xcookie;

      XNextEvent(dpy, &ev);

      if (cookie->type != GenericEvent ||
        cookie->extension != xi_opcode) {
        continue;
      }

      if (XGetEventData(dpy, cookie))
      {
        printf("Event type %d received\n", cookie->evtype);
        XFreeEventData(dpy, &ev.xcookie);
      }
    }
  }

  return 0;
}
