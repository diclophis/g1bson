# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(mpwm,1.1, peter.hutterer@who-t.net)
AM_INIT_AUTOMAKE(foreign)
AM_MAINTAINER_MODE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_CHECK_PROG([XCURSORGEN],[xcursorgen],[`which xcursorgen`])

# XSetClientPointer is only in the MPX aware lib, so we're all good if it
# exists.
AC_CHECK_LIB([Xi], [XSetClientPointer]) 
#PKG_CHECK_MODULES(MPWM, X Xext [Xi >= 1.2.9] Xcursor cairo [inputproto >= 1.9.99.8])

AC_SUBST(MPWM_LIBS)

AC_OUTPUT([Makefile
           src/Makefile
	   images/Makefile])
