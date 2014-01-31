/*--
  --*/


#include "Manager.h"
#include "logger.h"

/** 
 * Application entry point.
 */
int main(int argc, char** argv)
{
    Manager* manager;
    manager = new Manager();
    manager->init((argc >= 2) ? argv[1] : NULL);
    manager->loop();
    manager->takedown();
    delete manager;
}
