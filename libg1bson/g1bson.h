#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysym.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>

Window g1bson_create_window(Display *dpy);
int g1bson_has_xi2(Display *dpy);
void g1bson_select_events(Display *dpy, Window win);
void g1bson_list_devices(Display *display, int deviceid);
void g1bson_create_remove_master(Display *dpy, int xi_opcode);
void g1bson_add_mpx_for_window (
  Display *dsp,
  char *name,
  int *master_kbd,
  int *slave_kbd,
  int *master_ptr,
  int *slave_ptr
);
void g1bson_drop_mpx (Display *dsp, int mpx);
void g1bson_fake_keystroke (
Display *dsp, Screen *screen, Window window_id, char symbol,
int id_master_kbd, int id_slave_kbd,
int id_master_ptr, int id_slave_ptr
);
Window g1bson_search(Display *, Window w);
void g1bson_windows_matching_pid(Display *display, Window wRoot, unsigned long pid);
int g1bson_test(int argc, char **argv);
void g1bson_fake_mouse(Display *dsp,  int id_master_ptr, int x, int y);