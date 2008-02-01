
EXT=png
SYNFIG=synfig

EXTRA_DIST=studio_about.sif
IMAGES=fill_icon.$(EXT) normal_icon.$(EXT) sif_icon.$(EXT) synfig_icon.$(EXT) saveall_icon.$(EXT) bool_icon.$(EXT) integer_icon.$(EXT) angle_icon.$(EXT) segment_icon.$(EXT) blinepoint_icon.$(EXT) list_icon.$(EXT) canvas_pointer_icon.$(EXT) string_icon.$(EXT) eyedrop_icon.$(EXT) about_icon.$(EXT) splash_screen.$(EXT) canvas_icon.$(EXT) vector_icon.$(EXT) real_icon.$(EXT) color_icon.$(EXT) valuenode_icon.$(EXT) polygon_icon.$(EXT) bline_icon.$(EXT) layer_icon.$(EXT) duplicate_icon.$(EXT) gradient_icon.$(EXT) keyframe_lock_all.$(EXT) keyframe_lock_past.$(EXT) keyframe_lock_future.$(EXT) keyframe_lock_none.$(EXT)

CLEANFILES=$(IMAGES)


all: $(IMAGES)

.SUFFIXES:.sif .tif .png

clean:
	$(RM) $(CLEANFILES)

.sif.tif:
	$(SYNFIG) -q $< -o $@ --time 0

.sif.png:
	$(SYNFIG) -q $< -o $@ --time 0

keyframe_lock_all.$(EXT): keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0

keyframe_lock_past.$(EXT): keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c PastOnly

keyframe_lock_future.$(EXT): keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c FutureOnly

keyframe_lock_none.$(EXT): keyframe_lock_icon.sif
	$(SYNFIG) -q $< -o $@ --time 0 -c Disabled

