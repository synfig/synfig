# $Header: /opt/voria/cvs/studio/src/Makefile.am,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $

MAINTAINERCLEANFILES=Makefile.in
EXTRA_DIST=template.cpp template.h

SUBDIRS=sinfgapp gtkmm

#if WINDOWSYS_GTKMM
#WINDOWSYS_LIBS=$(top_builddir)/src/gtkmm/libwindowsys.a @GTKMM_LIBS@
#SUBDIRS=sinfgapp gtkmm
#else
#WINDOWSYS_LIBS=$(top_builddir)/src/win32/libwindowsys.a @WIN32_LIBS@
#SUBDIRS=win32
#endif

#bin_PROGRAMS	=	sinfgstudio
#sinfgstudio_SOURCES = main.cpp app.cpp app.h instance.cpp instance.h actions.cpp actions.h toolbox.cpp toolbox.h compview.cpp compview.h canvasview.cpp canvasview.h workarea.cpp workarea.h canvasproperties.cpp canvasproperties.h
# view.h view.cpp workarea.cpp workarea.h toolbox.cpp toolbox.h trackview.cpp trackview.h actions.cpp actions.h compview.cpp compview.h
#sinfgstudio_LDADD = @SINFG_LIBS@ $(WINDOWSYS_LIBS)
#sinfgstudio_LDFLAGS=-dlopen self
