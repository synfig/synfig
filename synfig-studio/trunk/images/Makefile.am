
SYNFIG=synfig
EXT=@imageext@

EXTRA_DIST=studio_about.sif
IMAGES=mirror_icon.$(EXT) time_track_icon.$(EXT) pastecanvas_icon.$(EXT) group_icon.$(EXT) clear_redo_icon.$(EXT) clear_undo_icon.$(EXT) navigator_icon.$(EXT) info_icon.$(EXT) zoom_icon.$(EXT) meta_data_icon.$(EXT) children_icon.$(EXT) keyframe_icon.$(EXT) swap_colors_icon.$(EXT) rotate_icon.$(EXT) scale_icon.$(EXT) smooth_move_icon.$(EXT) width_icon.$(EXT) rectangle_icon.$(EXT) circle_icon.$(EXT) draw_icon.$(EXT) sketch_icon.$(EXT) fill_icon.$(EXT) normal_icon.$(EXT) sif_icon.$(EXT) synfig_icon.$(EXT) saveall_icon.$(EXT) bool_icon.$(EXT) integer_icon.$(EXT) angle_icon.$(EXT) segment_icon.$(EXT) blinepoint_icon.$(EXT) list_icon.$(EXT) canvas_pointer_icon.$(EXT) string_icon.$(EXT) eyedrop_icon.$(EXT) about_icon.$(EXT) about_dialog.$(EXT) canvas_icon.$(EXT) vector_icon.$(EXT) real_icon.$(EXT) color_icon.$(EXT) valuenode_icon.$(EXT) polygon_icon.$(EXT) bline_icon.$(EXT) layer_icon.$(EXT) duplicate_icon.$(EXT) gradient_icon.$(EXT) keyframe_lock_all.$(EXT) keyframe_lock_past.$(EXT) keyframe_lock_future.$(EXT) keyframe_lock_none.$(EXT)

CLEANFILES=$(IMAGES)

imagedir=@imagedir@

image_DATA=$(IMAGES)


all: $(IMAGES)

SUFFIXES=.sif .tif .png

#.SUFFIXES: $(SUFFIXES)

PNGTOICO=pngtoico
CONVERT=convert

sif_icon.ico: $(srcdir)/sif_icon.sif
	$(SYNFIG) $< -w 16 -h 16 -o sif_icon_16.gif sif_icon.sif -w 32 -h 32 -o sif_icon_32.gif sif_icon.sif -w 64 -h 64 -o sif_icon_64.gif
	$(CONVERT) sif_icon_16.gif sif_icon_16.png
	$(CONVERT) sif_icon_32.gif sif_icon_32.png
	$(CONVERT) sif_icon_64.gif sif_icon_64.png
	$(PNGTOICO) sif_icon_64.png > $@
#	$(PNGTOICO) sif_icon_16.png sif_icon_32.png sif_icon_64.png > $@

clean:
	$(RM) $(CLEANFILES)

.sif.bmp:
	$(SYNFIG) -q $< -o $@ --time 0

.sif.$(EXT):
	$(SYNFIG) -q $< -o $@ --time 0

keyframe_lock_all.$(EXT): $(srcdir)/keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0

keyframe_lock_past.$(EXT): $(srcdir)/keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c PastOnly

keyframe_lock_future.$(EXT): $(srcdir)/keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c FutureOnly

keyframe_lock_none.$(EXT): $(srcdir)/keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c Disabled

#.sif.png:
#	$(SYNFIG) -q $< -o $@ --time 0
	
#datanode_icon.$(EXT): datanode_icon.sif
#	$(SYNFIG) -q datanode_icon.sif -o datanode_icon.$(EXT) -w 128 -h 128 --time 0

#color_icon.$(EXT): color_icon.sif
#	$(SYNFIG) -q color_icon.sif -o color_icon.$(EXT) -w 128 -h 128 --time 0

#real_icon.$(EXT): real_icon.sif
#	$(SYNFIG) -q real_icon.sif -o real_icon.$(EXT) -w 128 -h 128 --time 0

#vector_icon.$(EXT): vector_icon.sif
#	$(SYNFIG) -q vector_icon.sif -o vector_icon.$(EXT) -w 128 -h 128 --time 0

#about_dialog.$(EXT): about_dialog.sif
#	$(SYNFIG) -q about_dialog.sif -o about_dialog.$(EXT) --time 0

#about_icon.$(EXT): about_icon.sif
#	$(SYNFIG) -q about_icon.sif -o about_icon.$(EXT) -w 128 -h 128 --time 0

#canvas_icon.$(EXT): canvas_icon.sif
#	$(SYNFIG) -q canvas_icon.sif -o canvas_icon.$(EXT) -w 128 -h 128 --time 0
