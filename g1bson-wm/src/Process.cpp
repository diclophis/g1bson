/* $Id: Process.cpp,v 1.2 2006/06/16 07:36:54 whot Exp $ */

/*--
 --*/

#include "Process.h"

map<int,Process*> Process::processes;

/* Singleton factory */
Process* Process::getProcess(int pid)
{
    Process* p;

    p = processes[pid];

    if (!p)
    {
        p = new Process(pid);
        processes[pid] = p;
    }

    return p;
}

Process::Process(int pid)
{
    this->pid = pid;
    windows.clear();
    owner = NULL;
}

void Process::addWindow(WMWindow* win)
{
    vector<WMWindow*>::const_iterator it = windows.begin();

    // check for duplicates
    while (it != windows.end())
    {
        if ((*it) == win)
            return;
        it++;
    }

    windows.push_back(win);
}

void Process::removeWindow(WMWindow* win)
{
    vector<WMWindow*>::iterator it = windows.begin();

    while (it != windows.end())
    {
        if ((*it) == win)
        {
            windows.erase(it);
            return;
        }
        it++;
    }
}

void Process::setOwner(PointerDevice* owner)
{
    this->owner = owner;

    vector<WMWindow*>::const_iterator it = windows.begin();

    while(it != windows.end())
    {
        (*it)->recolor();
        it++;
    }
}

PointerDevice* Process::getOwner()
{
    return owner;
}
