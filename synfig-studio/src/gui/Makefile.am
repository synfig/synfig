# $Id$

EXTRA_DIST = \
	compview.cpp

MAINTAINERCLEANFILES = \
	Makefile.in

synfigstudio_src = main.cpp

#include makefile inserts from subdirectories
include actionmanagers/Makefile_insert
include cellrenderer/Makefile_insert
include dialogs/Makefile_insert
include dials/Makefile_insert
include docks/Makefile_insert
include modules/Makefile_insert
#makefile inserts for modules are included by modules/Makefile_insert
include states/Makefile_insert
include trees/Makefile_insert
include widgets/Makefile_insert
include workarearenderer/Makefile_insert

DUCKTRANSFORM_HH = \
	ducktransform_rotate.h \
	ducktransform_scale.h \
	ducktransform_translate.h \
	ducktransform_matrix.h \
	ducktransform_origin.h \
	duck.h

DUCKTRANSFORM_CC = \
	duck.cpp

EVENTS_HH = \
	event_keyboard.h \
	event_layerclick.h \
	event_mouse.h

OTHER_HH = \
	compview.h \
	eventkey.h \
	smach.h \
	timemodel.h \
	app.h \
	asyncrenderer.h \
	audiocontainer.h \
	autorecover.h \
	canvasview.h \
	devicetracker.h \
	dialogsettings.h \
	duckmatic.h \
	localization.h \
	iconcontroller.h \
	instance.h \
	ipc.h \
	keymapsettings.h \
	onemoment.h \
	preview.h \
	renddesc.h \
	render.h \
	resourcehelper.h \
	splash.h \
	statemanager.h \
	valuelink.h \
	workarea.h \
	main_win32.h \
	mainwindow.h \
	helpers.h

OTHER_CC = \
	timemodel.cpp \
	app.cpp \
	asyncrenderer.cpp \
	audiocontainer.cpp \
	autorecover.cpp \
	canvasview.cpp \
	devicetracker.cpp \
	dialogsettings.cpp \
	duckmatic.cpp \
	iconcontroller.cpp \
	instance.cpp \
	ipc.cpp \
	keymapsettings.cpp \
	onemoment.cpp \
	preview.cpp \
	renddesc.cpp \
	render.cpp \
	resourcehelper.cpp \
	splash.cpp \
	statemanager.cpp \
	valuelink.cpp \
	workarea.cpp \
	main_win32.cpp \
	mainwindow.cpp \
	helpers.cpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/src

bin_PROGRAMS = synfigstudio

synfigstudio_SOURCES = \
	$(synfigstudio_src) \
	$(DUCKTRANSFORM_HH) \
	$(DUCKTRANSFORM_CC) \
	$(EVENTS_HH) \
	$(OTHER_HH) \
	$(OTHER_CC)



synfigstudio_LDADD = \
	../synfigapp/libsynfigapp.la \
	@SYNFIG_LIBS@ \
	@GTKMM_LIBS@ \
	@JACK_LIBS@

synfigstudio_LDFLAGS = \
	-dlopen self

synfigstudio_CXXFLAGS = \
	@SYNFIG_CFLAGS@ \
	@GTKMM_CFLAGS@ \
	@JACK_CFLAGS@ \
	-DIMAGE_DIR=\"$(imagedir)\" \
	-DIMAGE_EXT=\"$(imageext)\" \
	-DPLUGIN_DIR=\"$(plugindir)\" \
	-DSOUND_DIR=\"$(sounddir)\" \
	-DUI_DIR=\"$(uidir)\" \
	-DLOCALEDIR=\"${LOCALEDIR}\"
