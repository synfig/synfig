/* === S Y N F I G ========================================================= */
/*!	\file iconcontroller.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008 Paul Wise
**  Copyright (c) 2009 Gerco Ballintijn
**	Copyright (c) 2009, 2011 Carlos López
**	Copyright (c) 2009 Nikita Kitaev
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>

#include "iconcontroller.h"
#include <synfig/valuenodes/valuenode_const.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <synfigapp/action.h>
#include <synfig/interpolation.h>

#include <gdkmm.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"png"
#endif

/* === M E T H O D S ======================================================= */

static std::map< int, Glib::RefPtr<Gdk::Pixbuf> > _tree_pixbuf_table_value_type;
static Glib::RefPtr<Gdk::Pixbuf> _tree_pixbuf_table_interpolation[(int)INTERPOLATION_CLAMPED+1];

IconController::IconController()
{
	icon_factory=Gtk::IconFactory::create();
}

IconController::~IconController()
{
	_tree_pixbuf_table_value_type.clear();
	for(int i(0);i<((int)INTERPOLATION_CLAMPED+1);i++)
		_tree_pixbuf_table_interpolation[i]=Glib::RefPtr<Gdk::Pixbuf>();

	icon_factory->remove_default();
}

void
IconController::init_icon(const synfig::String &name, const synfig::String &iconfile, const synfig::String& desc)
{
	Gtk::StockItem stockitem(Gtk::StockID("synfig-" + name), desc);
	Gtk::Stock::add(stockitem);
	Glib::RefPtr<Gtk::IconSet> icon_set = Gtk::IconSet::create();
	Gtk::IconSource icon_source;
	icon_source.set_direction_wildcarded();
	icon_source.set_state_wildcarded();
	icon_source.set_size_wildcarded();
	icon_source.set_filename(iconfile);
	icon_set->add_source(icon_source);
	icon_factory->add(stockitem.get_stock_id(), icon_set);
}

void
IconController::init_icon_clone(const synfig::String &name, const synfig::String& desc)
{
	Gtk::StockItem stockitem(Gtk::StockID("synfig-" + name), desc);
	Gtk::Stock::add(stockitem);
	Glib::RefPtr<Gtk::IconSet> icon_set = Gtk::IconSet::create();
	if (Gtk::Stock::lookup(stockitem.get_stock_id(), icon_set))
		icon_factory->add(stockitem.get_stock_id(), icon_set);
}

void
IconController::init_icons(const synfig::String& path_to_icons)
{
	try{
	Gtk::Window::set_default_icon_from_file(path_to_icons+"synfig_icon."+IMAGE_EXT);
	} catch(...)
	{
		synfig::warning("Unable to open "+path_to_icons+"synfig_icon."+IMAGE_EXT);
	}

#define INIT_STOCK_ICON(name,iconfile,desc) \
	init_icon(#name, (path_to_icons) + (iconfile), (desc));

#define INIT_STOCK_ICON_CLONE(name,desc) \
	init_icon_clone(#name, (desc));

#define INIT_STOCK_ITEM(name,desc) \
	stock_##name = Gtk::StockItem(Gtk::StockID("synfig-" #name),desc); \
	Gtk::Stock::add(stock_##name);

	// Types
	INIT_STOCK_ICON(type_bool, "type_bool_icon." IMAGE_EXT, _("Bool"));
	INIT_STOCK_ICON(type_integer, "type_integer_icon." IMAGE_EXT, _("Integer"));
	INIT_STOCK_ICON(type_angle, "type_angle_icon." IMAGE_EXT, _("Angle"));
	INIT_STOCK_ICON(type_time, "type_time_icon." IMAGE_EXT, _("Time"));
	INIT_STOCK_ICON(type_real, "type_real_icon." IMAGE_EXT, _("Real"));
	INIT_STOCK_ICON(type_vector, "type_vector_icon." IMAGE_EXT, _("Vector"));
	INIT_STOCK_ICON(type_color, "type_color_icon." IMAGE_EXT, _("Color"));
	INIT_STOCK_ICON(type_segment, "type_segment_icon." IMAGE_EXT, _("Segment"));
	INIT_STOCK_ICON(type_blinepoint, "type_splinepoint_icon." IMAGE_EXT, _("Spline Point"));
	INIT_STOCK_ICON(type_list, "type_list_icon." IMAGE_EXT, _("List"));
	INIT_STOCK_ICON(type_string, "type_string_icon." IMAGE_EXT, _("String"));
	INIT_STOCK_ICON(type_canvas, "type_canvas_icon." IMAGE_EXT, _("Canvas"));
	INIT_STOCK_ICON(type_gradient, "type_gradient_icon." IMAGE_EXT, _("Gradient"))

	// ToolBox Tools
	INIT_STOCK_ICON(normal, "tool_normal_icon." IMAGE_EXT, _("Transform Tool"));
	INIT_STOCK_ICON(polygon, "tool_polyline_icon." IMAGE_EXT, _("Polygon Tool"));
	INIT_STOCK_ICON(bline, "tool_spline_icon." IMAGE_EXT, _("Spline Tool"));
	INIT_STOCK_ICON(eyedrop, "tool_eyedrop_icon." IMAGE_EXT, _("Eyedrop Tool"));
	INIT_STOCK_ICON(fill, "tool_fill_icon." IMAGE_EXT, _("Fill Tool"));
	INIT_STOCK_ICON(draw, "tool_draw_icon." IMAGE_EXT, _("Draw Tool"));
	INIT_STOCK_ICON(lasso, "tool_cutout_icon." IMAGE_EXT, _("Cutout Tool"));
	INIT_STOCK_ICON(brush, "tool_brush_icon." IMAGE_EXT, _("Brush Tool"));
	INIT_STOCK_ICON(sketch, "tool_sketch_icon." IMAGE_EXT, _("Sketch Tool"));
	INIT_STOCK_ICON(circle, "tool_circle_icon." IMAGE_EXT, _("Circle Tool"));
	INIT_STOCK_ICON(rectangle, "tool_rectangle_icon." IMAGE_EXT, _("Rectangle Tool"));
	INIT_STOCK_ICON(smooth_move, "tool_smooth_move_icon." IMAGE_EXT, _("SmoothMove Tool"));
	INIT_STOCK_ICON(rotate, "tool_rotate_icon." IMAGE_EXT, _("Rotate Tool"));
	INIT_STOCK_ICON(width, "tool_width_icon." IMAGE_EXT, _("Width Tool"));
	INIT_STOCK_ICON(scale, "tool_scale_icon." IMAGE_EXT, _("Scale Tool"));
	INIT_STOCK_ICON(zoom, "tool_zoom_icon." IMAGE_EXT, _("Zoom Tool"));
	INIT_STOCK_ICON(mirror, "tool_mirror_icon." IMAGE_EXT, _("Mirror Tool"));
	INIT_STOCK_ICON(text, "tool_text_icon." IMAGE_EXT, _("Text Tool"));
	INIT_STOCK_ICON(gradient, "tool_gradient_icon." IMAGE_EXT, _("Gradient Tool"));
	INIT_STOCK_ICON(star, "tool_star_icon." IMAGE_EXT, _("Star Tool"));
	// ToolBox Others
	INIT_STOCK_ICON(reset_colors, "reset_colors_icon." IMAGE_EXT, _("Reset Colors"));
	INIT_STOCK_ICON(swap_colors, "swap_colors_icon." IMAGE_EXT, _("Swap Colors"));
	INIT_STOCK_ICON(value_node, "valuenode_icon." IMAGE_EXT, _("ValueNode"));
	INIT_STOCK_ICON(valuenode_forbidanimation, "valuenode_forbidanimation_icon." IMAGE_EXT, _("ValueNode Forbid Animation"));
	INIT_STOCK_ICON(rename, "rename_icon." IMAGE_EXT, _("Rename"));
	INIT_STOCK_ICON(canvas, "canvas_icon." IMAGE_EXT, _("Canvas"));
	INIT_STOCK_ICON(canvas_new, "canvas_icon." IMAGE_EXT, _("New Canvas"));

	// Document Related Actions
	INIT_STOCK_ICON(about, "about_icon." IMAGE_EXT, _("About"));
	INIT_STOCK_ICON(new_doc, "action_doc_new_icon." IMAGE_EXT, _("New"));
	INIT_STOCK_ICON(open, "action_doc_open_icon." IMAGE_EXT, _("Open"));
	INIT_STOCK_ICON(save, "action_doc_save_icon." IMAGE_EXT, _("Save"));
	INIT_STOCK_ICON(save_as, "action_doc_saveas_icon." IMAGE_EXT, _("Save As"));
	INIT_STOCK_ICON(save_all, "action_doc_saveall_icon." IMAGE_EXT, _("Save All"));
	INIT_STOCK_ICON(redo, "action_doc_redo_icon." IMAGE_EXT, _("Redo"));
	INIT_STOCK_ICON(undo, "action_doc_undo_icon." IMAGE_EXT, _("Undo"));

	// Layers
	INIT_STOCK_ICON(layer, "layer_icon." IMAGE_EXT, _("Layer"));
	// Blur Layers
	INIT_STOCK_ICON(layer_blur_blur, "layer_blur_blur_icon." IMAGE_EXT, _("Blur Layer"));
	INIT_STOCK_ICON(layer_blur_motion, "layer_blur_motion_icon." IMAGE_EXT, _("Motion Blur Layer"));
	INIT_STOCK_ICON(layer_blur_radial, "layer_blur_radial_icon." IMAGE_EXT, _("Radial Blur Layer"));
	// Distortion Layers
	INIT_STOCK_ICON(layer_distortion_curvewarp, "layer_distortion_curvewarp_icon." IMAGE_EXT, _("Curve Warp Layer"));
	INIT_STOCK_ICON(layer_distortion_insideout, "layer_distortion_insideout_icon." IMAGE_EXT, _("Inside Out Layer"));
	INIT_STOCK_ICON(layer_distortion_noise, "layer_distortion_noise_icon." IMAGE_EXT, _("Noise Distort Layer"));
	INIT_STOCK_ICON(layer_distortion_spherize, "layer_distortion_spherize_icon." IMAGE_EXT, _("Spherize Layer"));
	INIT_STOCK_ICON(layer_distortion_stretch, "layer_distortion_stretch_icon." IMAGE_EXT, _("Stretch Layer"));
	INIT_STOCK_ICON(layer_distortion_twirl, "layer_distortion_twirl_icon." IMAGE_EXT, _("Twirl Layer"));
	INIT_STOCK_ICON(layer_distortion_warp, "layer_distortion_warp_icon." IMAGE_EXT, _("Warp Layer"));
	// Example Layers
	INIT_STOCK_ICON(layer_example_metaballs, "layer_example_metaballs_icon." IMAGE_EXT, _("Metaballs Layer"));
	INIT_STOCK_ICON(layer_example_simplecircle, "layer_example_simplecircle_icon." IMAGE_EXT, _("Simple Circle Layer"));
	// Filter Layers
	INIT_STOCK_ICON(layer_filter_clamp, "layer_filter_clamp_icon." IMAGE_EXT, _("Clamp Layer"));
	INIT_STOCK_ICON(layer_filter_colorcorrect, "layer_filter_colorcorrect_icon." IMAGE_EXT, _("Color Correct Layer"));
	INIT_STOCK_ICON(layer_filter_halftone2, "layer_filter_halftone2_icon." IMAGE_EXT, _("Halftone 2 Layer"));
	INIT_STOCK_ICON(layer_filter_halftone3, "layer_filter_halftone3_icon." IMAGE_EXT, _("Halftone 3 Layer"));
	INIT_STOCK_ICON(layer_filter_lumakey, "layer_filter_lumakey_icon." IMAGE_EXT, _("Luma Key Layer"));
	// Fractal Layers
	INIT_STOCK_ICON(layer_fractal_julia, "layer_fractal_julia_icon." IMAGE_EXT, _("Julia Set Layer"));
	INIT_STOCK_ICON(layer_fractal_mandelbrot, "layer_fractal_mandelbrot_icon." IMAGE_EXT, _("Mandelbrot Set Layer"));
	// Geometry Layers
	INIT_STOCK_ICON(layer_geometry_checkerboard, "layer_geometry_checkerboard_icon." IMAGE_EXT, _("Checkerboard Layer"));
	INIT_STOCK_ICON(layer_geometry_circle, "layer_geometry_circle_icon." IMAGE_EXT, _("Circle Layer"));
	INIT_STOCK_ICON(layer_geometry_outline, "layer_geometry_outline_icon." IMAGE_EXT, _("Outline Layer"));
	INIT_STOCK_ICON(layer_geometry_advanced_outline, "layer_geometry_advanced_outline_icon." IMAGE_EXT, _("Advanced Outline Layer"));
	INIT_STOCK_ICON(layer_geometry_polygon, "layer_geometry_polygon_icon." IMAGE_EXT, _("Polygon Layer"));
	INIT_STOCK_ICON(layer_geometry_rectangle, "layer_geometry_rectangle_icon." IMAGE_EXT, _("Rectangle Layer"));
	INIT_STOCK_ICON(layer_geometry_region, "layer_geometry_region_icon." IMAGE_EXT, _("Region Layer"));
	INIT_STOCK_ICON(layer_geometry_solidcolor, "layer_geometry_solidcolor_icon." IMAGE_EXT, _("Solid Color Layer"));
	INIT_STOCK_ICON(layer_geometry_star, "layer_geometry_star_icon." IMAGE_EXT, _("Star Layer"));
	// Gradient Layers
	INIT_STOCK_ICON(layer_gradient_conical, "layer_gradient_conical_icon." IMAGE_EXT, _("Conical Gradient Layer"));
	INIT_STOCK_ICON(layer_gradient_curve, "layer_gradient_curve_icon." IMAGE_EXT, _("Curve Gradient Layer"));
	INIT_STOCK_ICON(layer_gradient_noise, "layer_gradient_noise_icon." IMAGE_EXT, _("Noise Gradient Layer"));
	INIT_STOCK_ICON(layer_gradient_linear, "layer_gradient_linear_icon." IMAGE_EXT, _("Linear Gradient Layer"));
	INIT_STOCK_ICON(layer_gradient_radial, "layer_gradient_radial_icon." IMAGE_EXT, _("Radial Gradient Layer"));
	INIT_STOCK_ICON(layer_gradient_spiral, "layer_gradient_spiral_icon." IMAGE_EXT, _("Spiral Gradient Layer"));
	// Other Layers
	INIT_STOCK_ICON(layer_other_duplicate, "layer_other_duplicate_icon." IMAGE_EXT, _("Duplicate Layer"));
	INIT_STOCK_ICON(layer_other_importimage, "layer_other_importimage_icon." IMAGE_EXT, _("Import Image Layer"));
	INIT_STOCK_ICON(layer_other_pastecanvas, "layer_other_group_icon." IMAGE_EXT, _("Group Layer"));
	INIT_STOCK_ICON(layer_other_plant, "layer_other_plant_icon." IMAGE_EXT, _("Plant Layer"));
	INIT_STOCK_ICON(layer_other_stroboscope, "layer_other_stroboscope_icon." IMAGE_EXT, _("Stroboscope Layer"));
	INIT_STOCK_ICON(layer_other_supersample, "layer_other_supersample_icon." IMAGE_EXT, _("Super Sample Layer"));
	INIT_STOCK_ICON(layer_other_sound, "layer_other_sound_icon." IMAGE_EXT, _("Sound Layer"));
	INIT_STOCK_ICON(layer_other_switch, "layer_other_switch_icon." IMAGE_EXT, _("Switch Layer"));
	INIT_STOCK_ICON(layer_other_skeleton, "layer_other_skeleton_icon." IMAGE_EXT, _("Skeleton Layer"));
	INIT_STOCK_ICON(layer_other_text, "layer_other_text_icon." IMAGE_EXT, _("Text Layer"));
	INIT_STOCK_ICON(layer_other_timeloop, "layer_other_timeloop_icon." IMAGE_EXT, _("Time Loop Layer"));
	INIT_STOCK_ICON(layer_other_xorpattern, "layer_other_xorpattern_icon." IMAGE_EXT, _("XOR Pattern Layer"));
	// Stylize Layers
	INIT_STOCK_ICON(layer_stylize_bevel, "layer_stylize_bevel_icon." IMAGE_EXT, _("Bevel Layer"));
	INIT_STOCK_ICON(layer_stylize_shade, "layer_stylize_shade_icon." IMAGE_EXT, _("Shade Layer"));
	// Transform Layers
	INIT_STOCK_ICON(layer_transform_rotate, "layer_transform_rotate_icon." IMAGE_EXT, _("Rotate Layer"));
	INIT_STOCK_ICON(layer_transform_translate, "layer_transform_translate_icon." IMAGE_EXT, _("Translate Layer"));
	INIT_STOCK_ICON(layer_transform_scale, "layer_transform_scale_icon." IMAGE_EXT, _("Scale Layer"));

	// Ghost Layers
	// TODO: icon for ghost group
	INIT_STOCK_ICON(layer_ghost_group, "layer_other_group_icon." IMAGE_EXT, _("Group Ghost"));
	//INIT_STOCK_ICON(layer_ghost_group, "layer_ghost_group_icon." IMAGE_EXT, _("Group Ghost"));

	INIT_STOCK_ICON(layer_new, "grid_enable_icon." IMAGE_EXT, _("Show Grid"));

	INIT_STOCK_ICON(info, "info_icon." IMAGE_EXT, _("Info Tool"));
	INIT_STOCK_ICON(group, "set_icon." IMAGE_EXT, _("Set"));

	INIT_STOCK_ICON(grid_enable, "grid_enable_icon." IMAGE_EXT, _("Show Grid"));
	INIT_STOCK_ICON(grid_disable, "grid_disable_icon." IMAGE_EXT, _("Hide Grid"));
	INIT_STOCK_ICON(grid_snap_enable, "grid_snap_enable_icon." IMAGE_EXT, _("Enable Grid Snap"));
	INIT_STOCK_ICON(grid_snap_disable, "grid_snap_disable_icon." IMAGE_EXT, _("Disable Grid Snap"));
	INIT_STOCK_ICON(duplicate, "duplicate_icon." IMAGE_EXT, _("Duplicate"));
	INIT_STOCK_ICON(encapsulate, "group_icon." IMAGE_EXT, _("Group"));
	// TODO: icon for 'Group Layer into Switch' action
	INIT_STOCK_ICON(encapsulate_switch, "layer_other_switch_icon." IMAGE_EXT, _("Group into Switch"));
	// TODO: icon for 'Group Layer into Filter' action
	INIT_STOCK_ICON(encapsulate_filter, "layer_icon." IMAGE_EXT, _("Group into Filter"));
	INIT_STOCK_ICON(select_all_child_layers, "select_all_child_layers_icon." IMAGE_EXT, _("Select All Child Layers"));

	INIT_STOCK_ICON(clear_undo, "clear_undo_icon." IMAGE_EXT, _("Clear Undo Stack"));
	INIT_STOCK_ICON(clear_redo, "clear_redo_icon." IMAGE_EXT, _("Clear Redo Stack"));

	INIT_STOCK_ICON(children, "library_icon." IMAGE_EXT, _("Library"));
	INIT_STOCK_ICON(curves, "graphs_icon." IMAGE_EXT, _("Graphs"));
	INIT_STOCK_ICON(keyframes, "keyframe_icon." IMAGE_EXT, _("Keyframes"));
	INIT_STOCK_ICON(meta_data, "meta_data_icon." IMAGE_EXT, _("MetaData"));
	INIT_STOCK_ICON(navigator, "navigator_icon." IMAGE_EXT, _("Navigator"));
	INIT_STOCK_ICON(timetrack, "time_track_icon." IMAGE_EXT, _("Time Track"));
	INIT_STOCK_ICON(history, "history_icon." IMAGE_EXT, _("History"));
	INIT_STOCK_ICON(palette, "palette_icon." IMAGE_EXT, _("Palette"));
	INIT_STOCK_ICON(params, "parameters_icon." IMAGE_EXT, _("Parameters"));

	INIT_STOCK_ICON(keyframe_lock_past_off, "keyframe_lock_past_off_icon." IMAGE_EXT, _("Past keyframes unlocked"));
	INIT_STOCK_ICON(keyframe_lock_past_on, "keyframe_lock_past_on_icon." IMAGE_EXT, _("Past keyframes locked"));
	INIT_STOCK_ICON(keyframe_lock_future_off, "keyframe_lock_future_off_icon." IMAGE_EXT, _("Future keyframes unlocked"));
	INIT_STOCK_ICON(keyframe_lock_future_on, "keyframe_lock_future_on_icon." IMAGE_EXT, _("Future keyframes locked"));

	INIT_STOCK_ICON(animate_mode_off, "animate_mode_off_icon." IMAGE_EXT, _("Animate Mode Off"));
	INIT_STOCK_ICON(animate_mode_on, "animate_mode_on_icon." IMAGE_EXT, _("Animate Mode On"));
	
	INIT_STOCK_ICON(jack, "jack_icon." IMAGE_EXT, _("JACK"));

	INIT_STOCK_ICON(set_outline_color, "set_outline_color." IMAGE_EXT, _("Set as Outline"));
	INIT_STOCK_ICON(set_fill_color, "set_fill_color." IMAGE_EXT, _("Set as Fill"));

	INIT_STOCK_ICON(animate_seek_begin, "animate_seek_begin_icon." IMAGE_EXT, _("Seek to Begin"));
	INIT_STOCK_ICON(animate_seek_prev_keyframe, "animate_seek_prev_keyframe_icon." IMAGE_EXT, _("Seek to Previous Keyframe"));
	INIT_STOCK_ICON(animate_seek_prev_frame, "animate_seek_prev_frame_icon." IMAGE_EXT, _("Seek to Previous Frame"));
	INIT_STOCK_ICON(animate_play, "animate_play_icon." IMAGE_EXT, _("Play"));
	INIT_STOCK_ICON(animate_stop, "animate_stop_icon." IMAGE_EXT, _("Stop"));
	INIT_STOCK_ICON(animate_pause, "animate_pause_icon." IMAGE_EXT, _("Pause"));
	INIT_STOCK_ICON(animate_seek_next_frame, "animate_seek_next_frame_icon." IMAGE_EXT, _("Seek to Next frame"));
	INIT_STOCK_ICON(animate_seek_next_keyframe, "animate_seek_next_keyframe_icon." IMAGE_EXT, _("Seek to Next Keyframe"));
	INIT_STOCK_ICON(animate_seek_end, "animate_seek_end_icon." IMAGE_EXT, _("Seek to End"));
	INIT_STOCK_ICON(animate_loop, "animate_loop_icon." IMAGE_EXT, _("Animate Loop"));
	INIT_STOCK_ICON(animate_bounds, "animate_bounds_icon." IMAGE_EXT, _("Play Bounds"));
	INIT_STOCK_ICON(animate_bound_lower, "animate_bound_lower_icon." IMAGE_EXT, _("Lower Bound"));
	INIT_STOCK_ICON(animate_bound_upper, "animate_bound_upper_icon." IMAGE_EXT, _("Upper Bound"));

	INIT_STOCK_ICON(add_to_group, "action_add_to_set_icon." IMAGE_EXT, _("Add Layer to Set"));
	INIT_STOCK_ICON(remove_from_group, "action_remove_from_set_icon." IMAGE_EXT, _("Remove Layer from Set"));
	INIT_STOCK_ICON(set_desc, "action_set_layer_description_icon." IMAGE_EXT, _("Set Layer Description"));
	INIT_STOCK_ICON(export, "action_export_icon." IMAGE_EXT, _("Export Value Node"));
	INIT_STOCK_ICON(unexport, "action_unexport_icon." IMAGE_EXT, _("Unexport Value Node"));
	INIT_STOCK_ICON(flat_interpolation, "action_flat_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Flat"));
	INIT_STOCK_ICON(interpolate_interpolation, "action_interpolate_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Interpolate"));
	INIT_STOCK_ICON(peak_interpolation, "action_peak_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Peak"));
	INIT_STOCK_ICON(rounded_interpolation, "action_rounded_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Rounded"));
	INIT_STOCK_ICON(squared_interpolation, "action_squared_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Squared"));

	INIT_STOCK_ICON(toggle_duck_position, "duck_position_icon." IMAGE_EXT, _("Toggle position handles"));
	INIT_STOCK_ICON(toggle_duck_vertex, "duck_vertex_icon." IMAGE_EXT, _("Toggle vertex handles"));
	INIT_STOCK_ICON(toggle_duck_tangent, "duck_tangent_icon." IMAGE_EXT, _("Toggle tangent handles"));
	INIT_STOCK_ICON(toggle_duck_radius, "duck_radius_icon." IMAGE_EXT, _("Toggle radius handles"));
	INIT_STOCK_ICON(toggle_duck_width, "duck_width_icon." IMAGE_EXT, _("Toggle width handles"));
	INIT_STOCK_ICON(toggle_duck_angle, "duck_angle_icon." IMAGE_EXT, _("Toggle angle handles"));

	INIT_STOCK_ICON(toggle_show_grid, "show_grid_icon." IMAGE_EXT, _("Toggle show grid"));
	INIT_STOCK_ICON(toggle_snap_grid, "snap_grid_icon." IMAGE_EXT, _("Toggle snap grid"));
	INIT_STOCK_ICON(toggle_show_guide, "show_guide_icon." IMAGE_EXT, _("Toggle show guide"));
	INIT_STOCK_ICON(toggle_snap_guide, "snap_guide_icon." IMAGE_EXT, _("Toggle snap guide"));

	INIT_STOCK_ICON(toggle_onion_skin, "onion_skin_icon." IMAGE_EXT, _("Toggle onion skin"));

	INIT_STOCK_ICON(toggle_background_rendering, "background_rendering_icon." IMAGE_EXT, _("Toggle background rendering"));

	INIT_STOCK_ICON(increase_resolution, "incr_resolution_icon." IMAGE_EXT, _("Increase resolution"));
	INIT_STOCK_ICON(decrease_resolution, "decr_resolution_icon." IMAGE_EXT, _("Decrease resolution"));

	INIT_STOCK_ICON(preview_options, "preview_options_icon." IMAGE_EXT, _("Preview Options Dialog"));
	INIT_STOCK_ICON(render_options, "render_options_icon." IMAGE_EXT, _("Render Options Dialog"));

	INIT_STOCK_ICON(interpolation_type_clamped, "interpolation_type_clamped_icon." IMAGE_EXT, _("Clamped interpolation"));
	INIT_STOCK_ICON(interpolation_type_tcb, "interpolation_type_tcb_icon." IMAGE_EXT, _("TCB interpolation"));
	INIT_STOCK_ICON(interpolation_type_const, "interpolation_type_const_icon." IMAGE_EXT, _("Const interpolation"));
	INIT_STOCK_ICON(interpolation_type_ease, "interpolation_type_ease_icon." IMAGE_EXT, _("Ease In/Out interpolation"));
	INIT_STOCK_ICON(interpolation_type_linear, "interpolation_type_linear_icon." IMAGE_EXT, _("Linear interpolation"));

	INIT_STOCK_ICON(utils_chain_link_on, "utils_chain_link_on_icon." IMAGE_EXT, _("Linked"));
	INIT_STOCK_ICON(utils_chain_link_off, "utils_chain_link_off_icon." IMAGE_EXT, _("Unlinked"));
	INIT_STOCK_ICON(utils_timetrack_align, "utils_timetrack_align_icon." IMAGE_EXT, _("Utils Timetrack align"));

	INIT_STOCK_ICON_CLONE(cvs_add, _("CVS Add"));
	INIT_STOCK_ICON_CLONE(cvs_update, _("CVS Update"));
	INIT_STOCK_ICON_CLONE(cvs_commit, _("CVS Commit"));
	INIT_STOCK_ICON_CLONE(cvs_revert, _("CVS Revert"));

#undef INIT_STOCK_ICON
#undef INIT_STOCK_ICON_CLONE
#undef INIT_STOCK_ITEM

	icon_factory->add_default();

	Gtk::IconSize::register_new("synfig-tiny_icon", 8, 8);
	Gtk::IconSize::register_new("synfig-small_icon",12,12);
	Gtk::IconSize::register_new("synfig-small_icon_16x16",16,16);

	for(Type *type = Type::get_first(); type != NULL; type = type->get_next())
		_tree_pixbuf_table_value_type[type->identifier]=Gtk::Button().render_icon_pixbuf(value_icon(*type),Gtk::ICON_SIZE_SMALL_TOOLBAR);

	for(int i(0);i<((int)INTERPOLATION_CLAMPED+1);i++)
		_tree_pixbuf_table_interpolation[i]=Gtk::Button().render_icon_pixbuf(interpolation_icon(Interpolation(i)),Gtk::ICON_SIZE_SMALL_TOOLBAR);
}

Glib::RefPtr<Gdk::Cursor>
IconController::get_normal_cursor()
{
	return Gdk::Cursor::create(Gdk::TOP_LEFT_ARROW);
}

Glib::RefPtr<Gdk::Cursor>
IconController::get_tool_cursor(const Glib::ustring& name,const Glib::RefPtr<Gdk::Window>& window)
{
	//this function is never called
	//it is commented out in WorkArea::refresh_cursor()
	assert(0);
	// \todo Do we still need it?

	Glib::RefPtr<Gdk::Pixbuf> pixbuf =
		Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-"+name),Gtk::ICON_SIZE_SMALL_TOOLBAR);
  	return Gdk::Cursor::create(window->get_display(), pixbuf, 0, 0);
}

Gtk::StockID
studio::value_icon(Type &type)
{
	if (type == type_bool)
		return Gtk::StockID("synfig-type_bool");
	if (type == type_integer)
		return Gtk::StockID("synfig-type_integer");
	if (type == type_angle)
		return Gtk::StockID("synfig-type_angle");
	if (type == type_time)
		return Gtk::StockID("synfig-type_time");
	if (type == type_real)
		return Gtk::StockID("synfig-type_real");
	if (type == type_vector)
		return Gtk::StockID("synfig-type_vector");
	if (type == type_color)
		return Gtk::StockID("synfig-type_color");
	if (type == type_segment)
		return Gtk::StockID("synfig-type_segment");
	if (type == type_bline_point)
		return Gtk::StockID("synfig-type_blinepoint");
	if (type == type_list)
		return Gtk::StockID("synfig-type_list");
	if (type == type_canvas)
		return Gtk::StockID("synfig-type_canvas");
	if (type == type_string)
		return Gtk::StockID("synfig-type_string");
	if (type == type_gradient)
		return Gtk::StockID("synfig-type_gradient");

	return Gtk::StockID("synfig-unknown");
}

Gtk::StockID
studio::interpolation_icon(synfig::Interpolation type)
{
	switch(type)
	{
		case INTERPOLATION_CLAMPED:
			return Gtk::StockID("synfig-interpolation_type_clamped");
		case INTERPOLATION_TCB:
			return Gtk::StockID("synfig-interpolation_type_tcb");
		case INTERPOLATION_CONSTANT:
			return Gtk::StockID("synfig-interpolation_type_const");
		case INTERPOLATION_HALT:
			return Gtk::StockID("synfig-interpolation_type_ease");
		case INTERPOLATION_LINEAR:
			return Gtk::StockID("synfig-interpolation_type_linear");
		case INTERPOLATION_MANUAL:
		case INTERPOLATION_UNDEFINED:
		case INTERPOLATION_NIL:
		default:
			break;
	}
	return Gtk::StockID();
}


Gtk::StockID
studio::valuenode_icon(etl::handle<synfig::ValueNode> value_node)
{
	if(handle<ValueNode_Const>::cast_dynamic(value_node))
	{
		return value_icon(value_node->get_type());
	}
	else
	{
		return Gtk::StockID("synfig-value_node");
	}
}

Glib::RefPtr<Gdk::Pixbuf>
studio::get_tree_pixbuf(Type &type)
{
	//return Gtk::Button().render_icon_pixbuf(value_icon(type),Gtk::ICON_SIZE_SMALL_TOOLBAR);
	return _tree_pixbuf_table_value_type[type.identifier];
}

Glib::RefPtr<Gdk::Pixbuf>
studio::get_interpolation_pixbuf(synfig::Interpolation type)
{
	return _tree_pixbuf_table_interpolation[int(type)];
}

#ifdef _WIN32
#define TEMPORARY_DELETE_MACRO DELETE
#undef DELETE
#endif

Gtk::StockID
studio::get_action_stock_id(const synfigapp::Action::BookEntry& action)
{
	Gtk::StockID stock_id;
	if(action.task=="add")				stock_id=Gtk::Stock::ADD;
	else if(action.task=="connect")		stock_id=Gtk::Stock::CONNECT;
	else if(action.task=="disconnect")	stock_id=Gtk::Stock::DISCONNECT;
	else if(action.task=="insert")		stock_id=Gtk::Stock::ADD;
	else if(action.task=="lower")		stock_id=Gtk::Stock::GO_DOWN;
	else if(action.task=="move_bottom")	stock_id=Gtk::Stock::GOTO_BOTTOM;
	else if(action.task=="move_top")	stock_id=Gtk::Stock::GOTO_TOP;
	else if(action.task=="raise")		stock_id=Gtk::Stock::GO_UP;
	else if(action.task=="remove")		stock_id=Gtk::Stock::DELETE;
	else if(action.task=="set_off")		stock_id=Gtk::Stock::NO;
	else if(action.task=="set_on")		stock_id=Gtk::Stock::YES;
	else								stock_id=Gtk::StockID("synfig-"+
															  action.task);
	return stock_id;
}

#ifdef _WIN32
#define DELETE TEMPORARY_DELETE_MACRO
#undef TEMPORARY_DELETE_MACRO
#endif

Gtk::StockID
studio::layer_icon(const synfig::String &layer)
{
	// Blur Layers
	if(layer=="blur")
		return Gtk::StockID("synfig-layer_blur_blur");
	else if(layer=="MotionBlur") // in the future should be "motion_blur"
		return Gtk::StockID("synfig-layer_blur_motion");
	else if(layer=="radial_blur")
		return Gtk::StockID("synfig-layer_blur_radial");
	// Distortion Layers
	else if(layer=="curve_warp")
		return Gtk::StockID("synfig-layer_distortion_curvewarp");
	else if(layer=="inside_out")
		return Gtk::StockID("synfig-layer_distortion_insideout");
	else if(layer=="noise_distort")
		return Gtk::StockID("synfig-layer_distortion_noise");
	else if(layer=="spherize")
		return Gtk::StockID("synfig-layer_distortion_spherize");
	else if(layer=="stretch")
		return Gtk::StockID("synfig-layer_distortion_stretch");
	else if(layer=="twirl")
		return Gtk::StockID("synfig-layer_distortion_twirl");
	else if(layer=="warp")
		return Gtk::StockID("synfig-layer_distortion_warp");
	// Example Layers
	else if(layer=="metaballs")
		return Gtk::StockID("synfig-layer_example_metaballs");
	else if(layer=="simple_circle")
		return Gtk::StockID("synfig-layer_example_simplecircle");
	// Filter Layers
	else if(layer=="clamp")
		return Gtk::StockID("synfig-layer_filter_clamp");
	else if(layer=="colorcorrect")
		return Gtk::StockID("synfig-layer_filter_colorcorrect");
	else if(layer=="halftone2")
		return Gtk::StockID("synfig-layer_filter_halftone2");
	else if(layer=="halftone3")
		return Gtk::StockID("synfig-layer_filter_halftone3");
	else if(layer=="lumakey")
		return Gtk::StockID("synfig-layer_filter_lumakey");
	// Fractal Layers
	else if(layer=="mandelbrot")
		return Gtk::StockID("synfig-layer_fractal_mandelbrot");
	else if(layer=="julia")
		return Gtk::StockID("synfig-layer_fractal_julia");
	// Geometry Layers
	else if(layer=="checker_board")
		return Gtk::StockID("synfig-layer_geometry_checkerboard");
	else if(layer=="circle")
		return Gtk::StockID("synfig-layer_geometry_circle");
	else if(layer=="outline")
		return Gtk::StockID("synfig-layer_geometry_outline");
	else if(layer=="advanced_outline")
		return Gtk::StockID("synfig-layer_geometry_advanced_outline");
	else if(layer=="polygon")
		return Gtk::StockID("synfig-layer_geometry_polygon");
	else if(layer=="rectangle")
		return Gtk::StockID("synfig-layer_geometry_rectangle");
	else if(layer=="region")
		return Gtk::StockID("synfig-layer_geometry_region");
	else if(layer=="solid_color" || layer=="SolidColor")
		return Gtk::StockID("synfig-layer_geometry_solidcolor");
	else if(layer=="star")
		return Gtk::StockID("synfig-layer_geometry_star");
	// Gradient Layers
	else if(layer=="conical_gradient")
		return Gtk::StockID("synfig-layer_gradient_conical");
	else if(layer=="curve_gradient")
		return Gtk::StockID("synfig-layer_gradient_curve");
	else if(layer=="noise")
		return Gtk::StockID("synfig-layer_gradient_noise");
	else if(layer=="linear_gradient")
		return Gtk::StockID("synfig-layer_gradient_linear");
	else if(layer=="radial_gradient")
		return Gtk::StockID("synfig-layer_gradient_radial");
	else if(layer=="spiral_gradient")
		return Gtk::StockID("synfig-layer_gradient_spiral");
	// Other Layers
	else if(layer=="duplicate")
		return Gtk::StockID("synfig-layer_other_duplicate");
	else if(layer=="importimage" || layer=="import")
		return Gtk::StockID("synfig-layer_other_importimage");
	else if(layer=="group" || layer=="PasteCanvas" || layer=="pastecanvas" || layer=="paste_canvas")
		return Gtk::StockID("synfig-layer_other_pastecanvas");
	else if(layer=="plant")
		return Gtk::StockID("synfig-layer_other_plant");
	else if(layer=="stroboscope")
		return Gtk::StockID("synfig-layer_other_stroboscope");
	else if(layer=="skeleton")
		return Gtk::StockID("synfig-layer_other_skeleton");
	else if(layer=="super_sample")
		return Gtk::StockID("synfig-layer_other_supersample");
	else if(layer=="switch")
		return Gtk::StockID("synfig-layer_other_switch");
	else if(layer=="text")
		return Gtk::StockID("synfig-layer_other_text");
	else if(layer=="sound")
		return Gtk::StockID("synfig-layer_other_sound");
	else if(layer=="timeloop")
		return Gtk::StockID("synfig-layer_other_timeloop");
	else if(layer=="xor_pattern")
		return Gtk::StockID("synfig-layer_other_xorpattern");
	// Stylize Layers
	else if(layer=="bevel")
		return Gtk::StockID("synfig-layer_stylize_bevel");
	else if(layer=="shade")
		return Gtk::StockID("synfig-layer_stylize_shade");
	// Transform Layers
	else if(layer=="rotate")
		return Gtk::StockID("synfig-layer_transform_rotate");
	else if(layer=="translate")
		return Gtk::StockID("synfig-layer_transform_translate");
	else if(layer=="zoom")
		return Gtk::StockID("synfig-layer_transform_scale");
	else if(layer=="ghost_group")
		return Gtk::StockID("synfig-layer_ghost_group");
	else
		return Gtk::StockID("synfig-layer");
}

Glib::RefPtr<Gdk::Pixbuf>
studio::get_tree_pixbuf_layer(const synfig::String &layer)
{
	return Gtk::Button().render_icon_pixbuf(layer_icon(layer),Gtk::ICON_SIZE_SMALL_TOOLBAR);
}

