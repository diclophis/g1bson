# g1bson: MPX Pair Hacking env

## Key Technologies

  * Multi-Cursor Window Manager: http://multicursor-wm.sourceforge.net/
  * Multi-Pointer X: http://en.wikipedia.org/wiki/Multi-Pointer_X
  * Xinput 2: http://www.x.org/wiki/guide/extensions/#index2h2
  * XQuartz: http://xquartz.macosforge.org/

## g1bson-vagrant

Ubuntu 13.10 Linux test kitchen 

## g1bson-osx

XCode project for application that bridges OSX mouse to X11

## g1bson-wm

X11 Window Manager with extended multi-pointer support

    # OSX
    aclocal && automake --add-missing && autoconf && PKG_CONFIG_PATH=/opt/X11/lib/pkgconfig CXXFLAGS=-I/opt/X11/include ./configure

    # Linux
    aclocal && automake --add-missing && autoconf && ./configure

## Research

  * http://tronche.com/gui/x/xlib/display/opening.html
  * https://github.com/esjeon/xinput2-touch
  * http://lwn.net/Articles/337898/
  * http://people.freedesktop.org/~whot/xi2-recipes/
  * http://keithp.com/blogs/Cursor\_tracking/
  * http://www.x.org/archive/X11R7.5/doc/man/man3/XSendEvent.3.html
  * http://stackoverflow.com/questions/13714831/controlling-multiple-pointers-with-xlib-or-xinput-in-ubuntu-linux
  * http://xquartz.macosforge.org/trac/wiki/quartz-wm
  * https://wiki.archlinux.org/index.php/Multi-pointer_X
  * https://wiki.archlinux.org/index.php/Xorg_multiseat
  * http://stackoverflow.com/questions/14561267/how-to-read-mouse-click-event-from-x-server
  * ftp://ftp.yellowdoglinux.com/.2/nslu2/sources/cvs/linux-input/ruby/xfree86/tuntitko-postevent.c
  * http://www.x.org/wiki/Development/Documentation/XorgInputHOWTO/
  * http://www.x.org/releases/X11R7.6/doc/inputproto/XI2proto.txt
  * http://en.wikipedia.org/wiki/X.Org_Server
  * http://linux.die.net/man/3/rootwindowofscreen
  * http://tronche.com/gui/x/xlib/window-information/XQueryTree.html
  * http://tronche.com/gui/x/xlib/display/display-macros.html
  * https://developer.apple.com/library/mac/documentation/cocoa/Reference/ApplicationKit/Classes/NSEvent_Class/Reference/Reference.html#//apple_ref/occ/instm/NSEvent/deltaX
  * http://src.gnu-darwin.org/ports/games/tenebrae/work/tenebrae_0/macosx/in_osx.m
  * http://lapcatsoftware.com/blog/2007/05/16/working-without-a-nib-part-1/
  * http://stackoverflow.com/questions/6388219/creating-nsapplication-in-cocoa
  * http://stackoverflow.com/questions/1829706/how-to-query-x11-display-resolution
  * http://askubuntu.com/questions/34657/how-to-make-x-org-listen-to-remote-connections-on-port-6000
