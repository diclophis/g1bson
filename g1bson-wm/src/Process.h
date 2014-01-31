/* $Id: Process.h,v 1.2 2006/06/16 07:36:54 whot Exp $ */

/*--
  --*/

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "logger.h"
#include<vector>
#include<map>

#include "WMWindow.h"
/**
 * Describes a process running connected to the X server. As soon as the
 * window manager spots the _CLIENT_PID atom set on a window, a new process is
 * created (or updated if the process already exists).
 * Note that in order for this to work the client's Xlib needs the FORCE_PID
 * hack.
 */

using namespace std;

class WMWindow;
class PointerDevice;

class Process
{
    private:
    static map<int,Process*> processes;

    public:
    static Process* getProcess(int pid);

    private:
    int pid;
    vector<WMWindow*> windows;
    PointerDevice* owner;

    private:
    Process(int pid);

    public:
    int getPID() { return pid; };
    void addWindow(WMWindow* win);
    void removeWindow(WMWindow* win);
    void setOwner(PointerDevice* owner);
    PointerDevice* getOwner();

};

#endif
