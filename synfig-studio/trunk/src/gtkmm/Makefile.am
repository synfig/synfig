# $Id: Makefile.am,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $

MAINTAINERCLEANFILES=Makefile.in $(GLADE_FILES)

CELLRENDERER_HH=cellrenderer_gradient.h cellrenderer_timetrack.h cellrenderer_time.h cellrenderer_value.h
CELLRENDERER_CC=cellrenderer_gradient.cpp cellrenderer_timetrack.cpp cellrenderer_time.cpp cellrenderer_value.cpp

WORKAREARENDERER_HH=renderer_bbox.h renderer_dragbox.h renderer_ducks.h renderer_grid.h renderer_guides.h renderer_timecode.h renderer_canvas.h workarearenderer.h
WORKAREARENDERER_CC=renderer_bbox.cpp renderer_dragbox.cpp renderer_ducks.cpp renderer_grid.cpp renderer_guides.cpp renderer_timecode.cpp renderer_canvas.cpp workarearenderer.cpp

PALETTE_HH=mod_palette/mod_palette.h mod_palette/dock_paledit.h mod_palette/dock_palbrowse.h
PALETTE_CC=mod_palette/mod_palette.cpp mod_palette/dock_paledit.cpp mod_palette/dock_palbrowse.cpp

MOD_MIRROR_HH=mod_mirror/mod_mirror.h mod_mirror/state_mirror.h
MOD_MIRROR_CC=mod_mirror/mod_mirror.cpp mod_mirror/state_mirror.cpp

DIALOG_HH=dialog_setup.h dialog_waypoint.h dialog_gradient.h dialog_color.h dialog_keyframe.h dialog_preview.h dialog_soundselect.h
DIALOG_CC=dialog_setup.cpp dialog_waypoint.cpp dialog_gradient.cpp dialog_color.cpp dialog_keyframe.cpp dialog_preview.cpp dialog_soundselect.cpp

DOCK_HH=dock_layergroups.h dock_children.h dock_canvasspecific.h dock_metadata.h dock_layers.h dock_params.h dock_keyframes.h dialog_tooloptions.h dock_canvases.h dock_history.h dockmanager.h dockbook.h dockdialog.h dockable.h dock_info.h dock_navigator.h dock_curves.h dock_timetrack.h
DOCK_CC=dock_layergroups.cpp dock_children.cpp dock_canvasspecific.cpp dock_metadata.cpp dock_layers.cpp dock_params.cpp dock_keyframes.cpp dialog_tooloptions.cpp dock_canvases.cpp dock_history.cpp dockmanager.cpp dockbook.cpp dockdialog.cpp dockable.cpp dock_info.cpp dock_navigator.cpp dock_curves.cpp dock_timetrack.cpp

WIDGET_HH=widget_compselect.h widget_gradient.h widget_color.h widget_enum.h widget_canvaschooser.h widget_time.h widget_filename.h widget_vector.h widget_value.h widget_defaults.h widget_coloredit.h widget_waypoint.h widget_waypointmodel.h widget_distance.h widget_sound.h widget_timeslider.h widget_curves.h
WIDGET_CC=widget_compselect.cpp widget_gradient.cpp widget_color.cpp widget_enum.cpp widget_canvaschooser.cpp widget_time.cpp widget_filename.cpp widget_vector.cpp widget_value.cpp widget_defaults.cpp widget_coloredit.cpp widget_waypoint.cpp widget_waypointmodel.cpp widget_distance.cpp widget_sound.cpp widget_timeslider.cpp widget_curves.cpp

STATE_HH=state_normal.h state_eyedrop.h state_draw.h state_stroke.h state_fill.h state_bline.h state_polygon.h state_sketch.h state_gradient.h state_circle.h state_rectangle.h state_width.h state_smoothmove.h state_scale.h state_rotate.h state_zoom.h
STATE_CC=state_normal.cpp state_eyedrop.cpp state_draw.cpp state_stroke.cpp state_fill.cpp state_bline.cpp state_polygon.cpp state_sketch.cpp state_gradient.cpp state_circle.cpp state_rectangle.cpp state_width.cpp state_smoothmove.cpp state_scale.cpp state_rotate.cpp state_zoom.cpp

TREEVIEW_HH=childrentree.h layertree.h keyframetree.h layergrouptree.h
TREEVIEW_CC=childrentree.cpp layertree.cpp keyframetree.cpp layergrouptree.cpp

TREESTORE_HH=layergrouptreestore.h metadatatreestore.h historytreestore.h keyframetreestore.h childrentreestore.h  canvastreestore.h layertreestore.h layerparamtreestore.h
TREESTORE_CC=layergrouptreestore.cpp metadatatreestore.cpp historytreestore.cpp keyframetreestore.cpp childrentreestore.cpp canvastreestore.cpp layertreestore.cpp layerparamtreestore.cpp

DUCKTRASNFORM_HH=duck.h ducktransform_translate.h ducktransform_scale.h ducktransform_rotate.h
DUCKTRANSFORM_CC=duck.cpp

EVENTS_HH=event_layerclick.h event_mouse.h
EVENTS_CC=

ACTION_MANAGERS_HH=layeractionmanager.h keyframeactionmanager.h groupactionmanager.h
ACTION_MANAGERS_CC=layeractionmanager.cpp keyframeactionmanager.cpp groupactionmanager.cpp

OTHER_HH=statemanager.h module.h ipc.h asyncrenderer.h smach.h duckmatic.h canvasoptions.h render.h app.h instance.h canvasview.h about.h iconcontroler.h toolbox.h workarea.h canvasproperties.h renddesc.h zoomdial.h eventkey.h onemoment.h devicetracker.h dialogsettings.h autorecover.h keymapsettings.h preview.h audiocontainer.h adjust_window.h valuelink.h
OTHER_CC=statemanager.cpp module.cpp ipc.cpp asyncrenderer.cpp duckmatic.cpp main.cpp canvasoptions.cpp render.cpp app.cpp instance.cpp canvasview.cpp about.cpp iconcontroler.cpp toolbox.cpp workarea.cpp canvasproperties.cpp renddesc.cpp zoomdial.cpp onemoment.cpp devicetracker.cpp dialogsettings.cpp autorecover.cpp keymapsettings.cpp preview.cpp audiocontainer.cpp adjust_window.cpp valuelink.cpp

INCLUDES=-I$(top_srcdir)/src

bin_PROGRAMS	=	synfigstudio
synfigstudio_SOURCES = $(MOD_MIRROR_CC) $(MOD_MIRROR_HH) $(PALETTE_CC) $(PALETTE_HH) $(WORKAREARENDERER_CC) $(WORKAREARENDERER_HH) $(ACTION_MANAGERS_HH) $(ACTION_MANAGERS_CC) $(DOCK_HH) $(DOCK_CC) $(OTHER_HH) $(OTHER_CC) $(EVENTS_HH) $(EVENTS_CC) $(DUCKTRANSFORM_HH) $(DUCKTRANSFORM_CC) $(TREEVIEW_HH) $(TREEVIEW_CC) $(TREESTORE_HH) $(TREESTORE_CC) $(STATE_CC) $(STATE_HH) $(WIDGET_CC) $(WIDGET_HH) $(DIALOG_HH) $(DIALOG_CC) $(CELLRENDERER_HH) $(CELLRENDERER_CC) 
synfigstudio_LDADD = -L../synfigapp -lsynfigapp @SYNFIG_LIBS@ @GTKMM_LIBS@ @FMOD_LIBS@
synfigstudio_LDFLAGS=-dlopen self
synfigstudio_CXXFLAGS=@SYNFIG_CFLAGS@ @GTKMM_CFLAGS@
