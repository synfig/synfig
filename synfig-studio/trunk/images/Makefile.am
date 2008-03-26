
SYNFIG=synfig
EXT=@imageext@

EXTRA_DIST=installer_logo.sif installer_logo_osx.sif splash_screen.sif splash_screen-0.61.07.sif about_icon.sif angle_icon.sif bline_icon.sif blinepoint_icon.sif bool_icon.sif canvas_icon.sif canvas_pointer_icon.sif children_icon.sif circle_icon.sif clear_redo_icon.sif clear_undo_icon.sif color_icon.sif curves_icon.sif draw_icon.sif duplicate_icon.sif encapsulate_icon.sif eyedrop_icon.sif fill_icon.sif gradient_icon.sif group_icon.sif info_icon.sif integer_icon.sif keyframe_icon.sif keyframe_lock_icon.sif layer_icon.sif list_icon.sif logo.sif meta_data_icon.sif mirror_icon.sif navigator_icon.sif normal_icon.sif pastecanvas_icon.sif polygon_icon.sif real_icon.sif rectangle_icon.sif rotate_icon.sif saveall_icon.sif scale_icon.sif segment_icon.sif select_all_child_layers_icon.sif sif_icon.sif sketch_icon.sif smooth_move_icon.sif string_icon.sif reset_colors_icon.sif swap_colors_icon.sif synfig_icon.sif time_icon.sif time_track_icon.sif valuenode_icon.sif vector_icon.sif wallpaper.sif width_icon.sif zoom_icon.sif rename_icon.sif plant_icon.sif text_icon.sif polyline_icon.sif star_icon.sif
IMAGES=installer_logo.$(EXT) installer_logo_osx.$(EXT) mirror_icon.$(EXT) time_icon.$(EXT) time_track_icon.$(EXT) curves_icon.$(EXT) pastecanvas_icon.$(EXT) group_icon.$(EXT) clear_redo_icon.$(EXT) clear_undo_icon.$(EXT) navigator_icon.$(EXT) info_icon.$(EXT) zoom_icon.$(EXT) meta_data_icon.$(EXT) children_icon.$(EXT) keyframe_icon.$(EXT) reset_colors_icon.$(EXT) swap_colors_icon.$(EXT) rotate_icon.$(EXT) scale_icon.$(EXT) smooth_move_icon.$(EXT) width_icon.$(EXT) rectangle_icon.$(EXT) circle_icon.$(EXT) draw_icon.$(EXT) sketch_icon.$(EXT) fill_icon.$(EXT) normal_icon.$(EXT) select_all_child_layers_icon.$(EXT) sif_icon.$(EXT) synfig_icon.$(EXT) saveall_icon.$(EXT) bool_icon.$(EXT) integer_icon.$(EXT) angle_icon.$(EXT) segment_icon.$(EXT) blinepoint_icon.$(EXT) list_icon.$(EXT) canvas_pointer_icon.$(EXT) string_icon.$(EXT) eyedrop_icon.$(EXT) about_icon.$(EXT) splash_screen.$(EXT) canvas_icon.$(EXT) vector_icon.$(EXT) real_icon.$(EXT) color_icon.$(EXT) valuenode_icon.$(EXT) polygon_icon.$(EXT) bline_icon.$(EXT) layer_icon.$(EXT) duplicate_icon.$(EXT) encapsulate_icon.$(EXT) gradient_icon.$(EXT) keyframe_lock_all.$(EXT) keyframe_lock_past.$(EXT) keyframe_lock_future.$(EXT) keyframe_lock_none.$(EXT) rename_icon.$(EXT) plant_icon.$(EXT) text_icon.$(EXT) polyline_icon.$(EXT) star_icon.$(EXT)

MAINTAINERCLEANFILES=Makefile.in
CLEANFILES=$(IMAGES) images.nsh unimages.nsh installer_logo.bmp sif_icon.ico synfig_icon.ico icons.nsh unicons.nsh

image_DATA=$(IMAGES)


all: $(IMAGES)

SUFFIXES=.sif .tif .png

#.SUFFIXES: $(SUFFIXES)

PNGTOICO=png2ico

.sif.ico:
	$(SYNFIG) $< -w 16 -h 16 -o $*-16x16.png $< -w 32 -h 32 -o $*-32x32.png $< -w 48 -h 48 -o $*-48x48.png $< -w 64 -h 64 -o $*-64x64.png
	cp -f `which $(PNGTOICO)`.* ./
	$(PNGTOICO) $@ $*-16x16.png $*-32x32.png $*-48x48.png $*-64x64.png
	rm -f ./$(PNGTOICO).* $*-16x16.png $*-32x32.png $*-48x48.png $*-64x64.png
	echo "  File \"images\\$@\"" >>./icons.nsh
	echo "  Delete \"\$$INSTDIR\\share\\pixmaps\\$@\"" >>./unicons.nsh

clean:
	$(RM) $(CLEANFILES)

.sif.bmp:
	$(SYNFIG) -q $< -o $@ --time 0

.sif.$(EXT):
	$(SYNFIG) -q $< -o $@ --time 0
	echo "  File \"images\\$@\"" >>./images.nsh
	echo "  Delete \"\$$INSTDIR\\share\\pixmaps\\$@\"" >>./unimages.nsh

keyframe_lock_all.$(EXT): $(srcdir)/keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0
	echo "  File \"images\\$@\"" >>./images.nsh
	echo "  Delete \"\$$INSTDIR\\share\\pixmaps\\$@\"" >>./unimages.nsh

keyframe_lock_past.$(EXT): $(srcdir)/keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c PastOnly
	echo "  File \"images\\$@\"" >>./images.nsh
	echo "  Delete \"\$$INSTDIR\\share\\pixmaps\\$@\"" >>./unimages.nsh

keyframe_lock_future.$(EXT): $(srcdir)/keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c FutureOnly
	echo "  File \"images\\$@\"" >>./images.nsh
	echo "  Delete \"\$$INSTDIR\\share\\pixmaps\\$@\"" >>./unimages.nsh

keyframe_lock_none.$(EXT): $(srcdir)/keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c Disabled
	echo "  File \"images\\$@\"" >>./images.nsh
	echo "  Delete \"\$$INSTDIR\\share\\pixmaps\\$@\"" >>./unimages.nsh

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

#splash_screen.$(EXT): splash_screen.sif
#	$(SYNFIG) -q splash_screen.sif -o splash_screen.$(EXT) --time 0

#about_icon.$(EXT): about_icon.sif
#	$(SYNFIG) -q about_icon.sif -o about_icon.$(EXT) -w 128 -h 128 --time 0

#canvas_icon.$(EXT): canvas_icon.sif
#	$(SYNFIG) -q canvas_icon.sif -o canvas_icon.$(EXT) -w 128 -h 128 --time 0
