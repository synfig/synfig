
EXT=png
SINFG=sinfg

EXTRA_DIST=studio_about.sif
IMAGES=fill_icon.$(EXT) normal_icon.$(EXT) sif_icon.$(EXT) sinfg_icon.$(EXT) saveall_icon.$(EXT) bool_icon.$(EXT) integer_icon.$(EXT) angle_icon.$(EXT) segment_icon.$(EXT) blinepoint_icon.$(EXT) list_icon.$(EXT) canvas_pointer_icon.$(EXT) string_icon.$(EXT) eyedrop_icon.$(EXT) about_icon.$(EXT) about_dialog.$(EXT) canvas_icon.$(EXT) vector_icon.$(EXT) real_icon.$(EXT) color_icon.$(EXT) valuenode_icon.$(EXT) polygon_icon.$(EXT) bline_icon.$(EXT) layer_icon.$(EXT) duplicate_icon.$(EXT) gradient_icon.$(EXT) keyframe_lock_all.$(EXT) keyframe_lock_past.$(EXT) keyframe_lock_future.$(EXT) keyframe_lock_none.$(EXT)

CLEANFILES=$(IMAGES)


all: $(IMAGES)

.SUFFIXES:.sif .tif .png

clean:
	$(RM) $(CLEANFILES)

.sif.tif:
	$(SINFG) -q $< -o $@ --time 0

.sif.png:
	$(SINFG) -q $< -o $@ --time 0

keyframe_lock_all.$(EXT): keyframe_lock_icon.sif
	$(SINFG) -q $< -o $@ --time 0

keyframe_lock_past.$(EXT): keyframe_lock_icon.sif
	$(SINFG) -q $< -o $@ --time 0 -c PastOnly

keyframe_lock_future.$(EXT): keyframe_lock_icon.sif
	$(SINFG) -q $< -o $@ --time 0 -c FutureOnly

keyframe_lock_none.$(EXT): keyframe_lock_icon.sif
	$(SINFG) -q $< -o $@ --time 0 -c Disabled

