# $Id$

EXTRA_DIST = compview.cpp
MAINTAINERCLEANFILES = Makefile.in $(GLADE_FILES)

CELLRENDERER_HH = \
	cellrenderer_gradient.h	  cellrenderer_time.h	cellrenderer_timetrack.h   cellrenderer_value.h
CELLRENDERER_CC = \
	cellrenderer_gradient.cpp cellrenderer_time.cpp cellrenderer_timetrack.cpp cellrenderer_value.cpp

WORKAREARENDERER_HH = \
	renderer_bbox.h	  renderer_canvas.h   renderer_dragbox.h   renderer_ducks.h   renderer_grid.h			\
	renderer_guides.h   renderer_timecode.h	  renderer_bonesetup.h   workarearenderer.h
WORKAREARENDERER_CC = \
	renderer_bbox.cpp renderer_canvas.cpp renderer_dragbox.cpp renderer_ducks.cpp renderer_grid.cpp			\
	renderer_guides.cpp renderer_timecode.cpp renderer_bonesetup.cpp workarearenderer.cpp

PALETTE_HH = \
	mod_palette/dock_palbrowse.h   mod_palette/dock_paledit.h   mod_palette/mod_palette.h
PALETTE_CC = \
	mod_palette/dock_palbrowse.cpp mod_palette/dock_paledit.cpp mod_palette/mod_palette.cpp

MOD_MIRROR_HH = \
	mod_mirror/mod_mirror.h	  mod_mirror/state_mirror.h
MOD_MIRROR_CC = \
	mod_mirror/mod_mirror.cpp mod_mirror/state_mirror.cpp

DIALOG_HH = \
	dialog_color.h	 dialog_gradient.h   dialog_keyframe.h	 dialog_preview.h   dialog_setup.h			\
	dialog_soundselect.h   dialog_waypoint.h
DIALOG_CC = \
	dialog_color.cpp dialog_gradient.cpp dialog_keyframe.cpp dialog_preview.cpp dialog_setup.cpp			\
	dialog_soundselect.cpp dialog_waypoint.cpp

DOCK_HH = \
	dialog_tooloptions.h   dock_canvases.h	 dock_canvasspecific.h	 dock_children.h   dock_curves.h		\
	dock_history.h	 dock_info.h   dock_keyframes.h	  dock_layergroups.h   dock_layers.h   dock_metadata.h		\
	dock_navigator.h   dock_params.h   dock_timetrack.h   dockable.h   dockbook.h	dockdialog.h   dockmanager.h
DOCK_CC = \
	dialog_tooloptions.cpp dock_canvases.cpp dock_canvasspecific.cpp dock_children.cpp dock_curves.cpp		\
	dock_history.cpp dock_info.cpp dock_keyframes.cpp dock_layergroups.cpp dock_layers.cpp dock_metadata.cpp	\
	dock_navigator.cpp dock_params.cpp dock_timetrack.cpp dockable.cpp dockbook.cpp dockdialog.cpp dockmanager.cpp

WIDGET_HH = \
	widget_canvaschooser.h	 widget_color.h	  widget_coloredit.h   widget_compselect.h   widget_curves.h		\
	widget_defaults.h   widget_distance.h	widget_enum.h	widget_filename.h   widget_gradient.h			\
	widget_sound.h	 widget_time.h	 widget_timeslider.h   widget_value.h	widget_vector.h	  widget_waypoint.h	\
	widget_waypointmodel.h   widget_bonechooser.h
WIDGET_CC = \
	widget_canvaschooser.cpp widget_color.cpp widget_coloredit.cpp widget_compselect.cpp widget_curves.cpp		\
	widget_defaults.cpp widget_distance.cpp widget_enum.cpp widget_filename.cpp widget_gradient.cpp			\
	widget_sound.cpp widget_time.cpp widget_timeslider.cpp widget_value.cpp widget_vector.cpp widget_waypoint.cpp	\
	widget_waypointmodel.cpp widget_bonechooser.cpp

STATE_HH = \
	state_bline.h	state_circle.h	 state_draw.h	state_eyedrop.h	  state_fill.h	 state_gradient.h		\
	state_normal.h	 state_polygon.h   state_rectangle.h   state_rotate.h	state_scale.h	state_sketch.h		\
	state_smoothmove.h   state_star.h   state_stroke.h   state_text.h   state_width.h   state_zoom.h
STATE_CC = \
	state_bline.cpp state_circle.cpp state_draw.cpp state_eyedrop.cpp state_fill.cpp state_gradient.cpp		\
	state_normal.cpp state_polygon.cpp state_rectangle.cpp state_rotate.cpp state_scale.cpp state_sketch.cpp	\
	state_smoothmove.cpp state_star.cpp state_stroke.cpp state_text.cpp state_width.cpp state_zoom.cpp

TREEVIEW_HH = \
	childrentree.h	 keyframetree.h	  layergrouptree.h   layertree.h
TREEVIEW_CC = \
	childrentree.cpp keyframetree.cpp layergrouptree.cpp layertree.cpp

TREESTORE_HH = \
	canvastreestore.h   childrentreestore.h	  historytreestore.h   keyframetreestore.h   layergrouptreestore.h	\
	layerparamtreestore.h	layertreestore.h   metadatatreestore.h
TREESTORE_CC = \
	canvastreestore.cpp childrentreestore.cpp historytreestore.cpp keyframetreestore.cpp layergrouptreestore.cpp	\
	layerparamtreestore.cpp layertreestore.cpp metadatatreestore.cpp

DUCKTRANSFORM_HH = ducktransform_rotate.h ducktransform_scale.h ducktransform_translate.h ducktransform_matrix.h \
	duck.h
DUCKTRANSFORM_CC = \
	duck.cpp

EVENTS_HH = event_layerclick.h event_mouse.h

ACTION_MANAGERS_HH = \
	groupactionmanager.h   keyframeactionmanager.h	 layeractionmanager.h
ACTION_MANAGERS_CC = \
	groupactionmanager.cpp keyframeactionmanager.cpp layeractionmanager.cpp

OTHER_HH = compview.h eventkey.h smach.h \
	about.h	  adjust_window.h   app.h   asyncrenderer.h   audiocontainer.h	 autorecover.h	 canvasoptions.h	\
	canvasproperties.h   canvasview.h   devicetracker.h   dialogsettings.h	 duckmatic.h   general.h                \
	iconcontroller.h instance.h   ipc.h   keymapsettings.h	module.h onemoment.h preview.h   renddesc.h             \
	render.h splash.h statemanager.h toolbox.h valuelink.h workarea.h zoomdial.h
OTHER_CC = main.cpp \
	about.cpp adjust_window.cpp app.cpp asyncrenderer.cpp audiocontainer.cpp autorecover.cpp canvasoptions.cpp	\
	canvasproperties.cpp canvasview.cpp devicetracker.cpp dialogsettings.cpp duckmatic.cpp iconcontroller.cpp	\
	instance.cpp ipc.cpp keymapsettings.cpp module.cpp onemoment.cpp preview.cpp renddesc.cpp render.cpp		\
	splash.cpp statemanager.cpp toolbox.cpp valuelink.cpp workarea.cpp zoomdial.cpp

INCLUDES = -I$(top_srcdir)/src

bin_PROGRAMS = synfigstudio
synfigstudio_SOURCES = \
	$(MOD_MIRROR_CC) $(MOD_MIRROR_HH) $(PALETTE_CC) $(PALETTE_HH) $(WORKAREARENDERER_CC) $(WORKAREARENDERER_HH)	\
	$(ACTION_MANAGERS_HH) $(ACTION_MANAGERS_CC) $(DOCK_HH) $(DOCK_CC) $(OTHER_HH) $(OTHER_CC) $(EVENTS_HH)		\
	$(DUCKTRANSFORM_HH) $(DUCKTRANSFORM_CC) $(TREEVIEW_HH) $(TREEVIEW_CC) $(TREESTORE_HH) $(TREESTORE_CC)		\
	$(STATE_CC) $(STATE_HH) $(WIDGET_CC) $(WIDGET_HH) $(CELLRENDERER_HH) $(CELLRENDERER_CC)				\
	$(DIALOG_HH) $(DIALOG_CC)

synfigstudio_LDADD = ../synfigapp/libsynfigapp.la @SYNFIG_LIBS@ @GTKMM_LIBS@ @FMOD_LIBS@
synfigstudio_LDFLAGS = -dlopen self
synfigstudio_CXXFLAGS = @SYNFIG_CFLAGS@ @GTKMM_CFLAGS@ -DIMAGE_DIR=\"$(imagedir)\" -DIMAGE_EXT=\"$(imageext)\" "-DLOCALEDIR=\"$(localedir)\""
