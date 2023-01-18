/* === S Y N F I G ========================================================= */
/*!	\file iconcontroller.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008 Paul Wise
**  Copyright (c) 2009 Gerco Ballintijn
**	Copyright (c) 2009, 2011 Carlos López
**	Copyright (c) 2009 Nikita Kitaev
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "iconcontroller.h"

#include <gtkmm/button.h>
#include <gtkmm/stock.h>
#include <gtkmm/window.h>
#include <gui/localization.h>
#include <synfig/general.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfigapp/action.h>

#include <gtkmm/icontheme.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"png"
#endif

/* === M E T H O D S ======================================================= */

static std::map< int, Glib::RefPtr<Gdk::Pixbuf> > _tree_pixbuf_table_value_type;

IconController::IconController()
{
	icon_factory=Gtk::IconFactory::create();
}

IconController::~IconController()
{
	_tree_pixbuf_table_value_type.clear();

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
	INIT_STOCK_ICON(bone,"tool_skeleton_icon." IMAGE_EXT,_("Skeleton Tool"));
	INIT_STOCK_ICON(eyedrop, "tool_eyedrop_icon." IMAGE_EXT, _("Eyedropper Tool"));
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
	INIT_STOCK_ICON(export, "action_doc_saveas_icon." IMAGE_EXT, _("Export"));
	INIT_STOCK_ICON(save_all, "action_doc_saveall_icon." IMAGE_EXT, _("Save All"));
	INIT_STOCK_ICON(redo, "action_doc_redo_icon." IMAGE_EXT, _("Redo"));
	INIT_STOCK_ICON(undo, "action_doc_undo_icon." IMAGE_EXT, _("Undo"));

	// Ghost Layers
	INIT_STOCK_ICON(layer_ghost_group, "layer_other_ghostgroup_icon." IMAGE_EXT, _("Group Ghost"));

	INIT_STOCK_ICON(info, "info_icon." IMAGE_EXT, _("Info Tool"));
	INIT_STOCK_ICON(group, "set_icon." IMAGE_EXT, _("Set"));

	INIT_STOCK_ICON(duplicate, "duplicate_icon." IMAGE_EXT, _("Duplicate"));
	INIT_STOCK_ICON(encapsulate, "group_icon." IMAGE_EXT, _("Group"));
	INIT_STOCK_ICON(encapsulate_switch, "layer_other_switch_icon." IMAGE_EXT, _("Group into Switch"));
	INIT_STOCK_ICON(encapsulate_filter, "layer_other_filtergroup_icon." IMAGE_EXT, _("Group into Filter"));
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

	INIT_STOCK_ICON(set_outline_color, "set_outline_color_icon." IMAGE_EXT, _("Set as Outline"));
	INIT_STOCK_ICON(set_fill_color, "set_fill_color_icon." IMAGE_EXT, _("Set as Fill"));

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
	INIT_STOCK_ICON(offpeak_interpolation, "action_offpeak_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Off-Peak"));
	INIT_STOCK_ICON(rounded_interpolation, "action_rounded_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Rounded"));
	INIT_STOCK_ICON(innerrounded_interpolation, "action_innerrounded_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Inner Rounded"));
	INIT_STOCK_ICON(squared_interpolation, "action_squared_interpolation_icon." IMAGE_EXT, _("Set Interpolation to Squared"));

	INIT_STOCK_ICON(toggle_duck_position, "duck_position_icon." IMAGE_EXT, _("Toggle position handles"));
	INIT_STOCK_ICON(toggle_duck_vertex, "duck_vertex_icon." IMAGE_EXT, _("Toggle vertex handles"));
	INIT_STOCK_ICON(toggle_duck_tangent, "duck_tangent_icon." IMAGE_EXT, _("Toggle tangent handles"));
	INIT_STOCK_ICON(toggle_duck_radius, "duck_radius_icon." IMAGE_EXT, _("Toggle radius handles"));
	INIT_STOCK_ICON(toggle_duck_width, "duck_width_icon." IMAGE_EXT, _("Toggle width handles"));
	INIT_STOCK_ICON(toggle_duck_angle, "duck_angle_icon." IMAGE_EXT, _("Toggle angle handles"));

	INIT_STOCK_ICON(toggle_show_grid, "show_grid_icon." IMAGE_EXT, _("Toggle show grid"));
	INIT_STOCK_ICON(toggle_snap_grid, "snap_grid_icon." IMAGE_EXT, _("Toggle snap grid"));
	INIT_STOCK_ICON(toggle_show_guide, "show_guideline_icon." IMAGE_EXT, _("Toggle show guide"));
	INIT_STOCK_ICON(toggle_snap_guide, "snap_guideline_icon." IMAGE_EXT, _("Toggle snap guide"));

	INIT_STOCK_ICON(toggle_onion_skin, "onion_skin_icon." IMAGE_EXT, _("Toggle onion skin"));

	INIT_STOCK_ICON(toggle_background_rendering, "background_rendering_icon." IMAGE_EXT, _("Toggle background rendering"));

	INIT_STOCK_ICON(increase_resolution, "incr_resolution_icon." IMAGE_EXT, _("Increase resolution"));
	INIT_STOCK_ICON(decrease_resolution, "decr_resolution_icon." IMAGE_EXT, _("Decrease resolution"));

	INIT_STOCK_ICON(preview_options, "preview_options_icon." IMAGE_EXT, _("Preview Options Dialog"));
	INIT_STOCK_ICON(render_options, "render_options_icon." IMAGE_EXT, _("Render Options Dialog"));

	INIT_STOCK_ICON(utils_chain_link_on, "utils_chain_link_on_icon." IMAGE_EXT, _("Linked"));
	INIT_STOCK_ICON(utils_chain_link_off, "utils_chain_link_off_icon." IMAGE_EXT, _("Unlinked"));
	INIT_STOCK_ICON(utils_timetrack_align, "utils_timetrack_align_icon." IMAGE_EXT, _("Utils Timetrack align"));

#undef INIT_STOCK_ICON

	icon_factory->add_default();

	Gtk::IconSize::register_new("synfig-tiny_icon", 8, 8);
	Gtk::IconSize::register_new("synfig-small_icon",12,12);
	Gtk::IconSize::register_new("synfig-small_icon_16x16",16,16);

	int width_small_toolbar, height_small_toolbar;
	Gtk::IconSize::lookup(Gtk::ICON_SIZE_SMALL_TOOLBAR, width_small_toolbar, height_small_toolbar);

	for(Type *type = Type::get_first(); type != nullptr; type = type->get_next()) {
		Glib::RefPtr<Gdk::Pixbuf> icon = Gtk::IconTheme::get_default()->load_icon(value_icon_name(*type), height_small_toolbar, Gtk::ICON_LOOKUP_FORCE_SIZE);
		if (!icon)
			icon = Gtk::IconTheme::get_default()->load_icon("image-missing", height_small_toolbar, Gtk::ICON_LOOKUP_FORCE_SIZE);
		_tree_pixbuf_table_value_type[type->identifier] = icon;
	}
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

std::string
studio::value_icon_name(Type &type)
{
	if (type == type_bool)
		return "type_bool_icon";
	if (type == type_integer)
		return "type_integer_icon";
	if (type == type_angle)
		return "type_angle_icon";
	if (type == type_time)
		return "type_time_icon";
	if (type == type_real)
		return "type_real_icon";
	if (type == type_vector)
		return "type_vector_icon";
	if (type == type_color)
		return "type_color_icon";
	if (type == type_segment)
		return "type_segment_icon";
	if (type == type_bline_point)
		return "type_splinepoint_icon";
	if (type == type_list)
		return "type_list_icon";
	if (type == type_canvas)
		return "type_canvas_icon";
	if (type == type_string)
		return "type_string_icon";
	if (type == type_gradient)
		return "type_gradient_icon";
	if (!type.description.name.empty())
		synfig::warning(_("no icon for value type: \"%s\""), type.description.name.c_str());
	return "image-missing";
}

std::string
studio::interpolation_icon_name(synfig::Interpolation type)
{
	switch(type)
	{
		case INTERPOLATION_CLAMPED:
			return "interpolation_type_clamped_icon";
		case INTERPOLATION_TCB:
			return "interpolation_type_tcb_icon";
		case INTERPOLATION_CONSTANT:
			return "interpolation_type_const_icon";
		case INTERPOLATION_HALT:
			return "interpolation_type_ease_icon";
		case INTERPOLATION_LINEAR:
			return "interpolation_type_linear_icon";
		case INTERPOLATION_MANUAL:
		case INTERPOLATION_UNDEFINED:
		case INTERPOLATION_NIL:
		default:
			break;
	}
	return "image-missing";
}


std::string
studio::valuenode_icon_name(synfig::ValueNode::Handle value_node)
{
	if (ValueNode_Const::Handle::cast_dynamic(value_node))
		return value_icon_name(value_node->get_type());
	else
		return "value_node_icon";
}

Glib::RefPtr<Gdk::Pixbuf>
studio::get_tree_pixbuf(Type &type)
{
	//return Gtk::Button().render_icon_pixbuf(value_icon(type),Gtk::ICON_SIZE_SMALL_TOOLBAR);
	return _tree_pixbuf_table_value_type[type.identifier];
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

std::string
studio::layer_icon_name(const synfig::String &layer)
{
	// Blur Layers
	if(layer=="blur")
		return "layer_blur_blur_icon";
	else if(layer=="MotionBlur") // in the future should be "motion_blur"
		return "layer_blur_motion_icon";
	else if(layer=="radial_blur")
		return "layer_blur_radial_icon";
	// Distortion Layers
	else if(layer=="curve_warp")
		return "layer_distortion_curvewarp_icon";
	else if(layer=="inside_out")
		return "layer_distortion_insideout_icon";
	else if(layer=="noise_distort")
		return "layer_distortion_noise_icon";
	else if(layer=="skeleton_deformation")
		return "layer_distortion_skeletondeformation_icon";
	else if(layer=="spherize")
		return "layer_distortion_spherize_icon";
	else if(layer=="stretch")
		return "layer_distortion_stretch_icon";
	else if(layer=="twirl")
		return "layer_distortion_twirl_icon";
	else if(layer=="warp")
		return "layer_distortion_warp_icon";
	// Example Layers
	else if(layer=="metaballs")
		return "layer_example_metaballs_icon";
	else if(layer=="simple_circle")
		return "layer_example_simplecircle_icon";
	// Filter Layers
	else if(layer=="clamp")
		return "layer_filter_clamp_icon";
	else if(layer=="colorcorrect")
		return "layer_filter_colorcorrect_icon";
	else if(layer=="halftone2")
		return "layer_filter_halftone2_icon";
	else if(layer=="halftone3")
		return "layer_filter_halftone3_icon";
	else if(layer=="lumakey")
		return "layer_filter_lumakey_icon";
	// Fractal Layers
	else if(layer=="mandelbrot")
		return "layer_fractal_mandelbrot_icon";
	else if(layer=="julia")
		return "layer_fractal_julia_icon";
	// Geometry Layers
	else if(layer=="checker_board")
		return "layer_geometry_checkerboard_icon";
	else if(layer=="circle")
		return "layer_geometry_circle_icon";
	else if(layer=="outline")
		return "layer_geometry_outline_icon";
	else if(layer=="advanced_outline")
		return "layer_geometry_advanced_outline_icon";
	else if(layer=="polygon")
		return "layer_geometry_polygon_icon";
	else if(layer=="rectangle")
		return "layer_geometry_rectangle_icon";
	else if(layer=="region")
		return "layer_geometry_region_icon";
	else if(layer=="solid_color" || layer=="SolidColor")
		return "layer_geometry_solidcolor_icon";
	else if(layer=="star")
		return "layer_geometry_star_icon";
	// Gradient Layers
	else if(layer=="conical_gradient")
		return "layer_gradient_conical_icon";
	else if(layer=="curve_gradient")
		return "layer_gradient_curve_icon";
	else if(layer=="noise")
		return "layer_gradient_noise_icon";
	else if(layer=="linear_gradient")
		return "layer_gradient_linear_icon";
	else if(layer=="radial_gradient")
		return "layer_gradient_radial_icon";
	else if(layer=="spiral_gradient")
		return "layer_gradient_spiral_icon";
	// Other Layers
	else if(layer=="duplicate")
		return "layer_other_duplicate_icon";
	else if(layer=="importimage" || layer=="import")
		return "layer_other_importimage_icon";
	else if(layer=="filter_group")
		return "layer_other_filtergroup_icon";
	else if(layer=="group" || layer=="PasteCanvas" || layer=="pastecanvas" || layer=="paste_canvas")
		return "layer_other_group_icon";
	else if(layer=="plant")
		return "layer_other_plant_icon";
	else if(layer=="freetime")
		return "layer_other_freetime_icon";
	else if(layer=="stroboscope")
		return "layer_other_stroboscope_icon";
	else if(layer=="skeleton")
		return "layer_other_skeleton_icon";
	else if(layer=="super_sample")
		return "layer_other_supersample_icon";
	else if(layer=="switch")
		return "layer_other_switch_icon";
	else if(layer=="text")
		return "layer_other_text_icon";
	else if(layer=="sound")
		return "layer_other_sound_icon";
	else if(layer=="timeloop")
		return "layer_other_timeloop_icon";
	else if(layer=="xor_pattern")
		return "layer_other_xorpattern_icon";
	// Stylize Layers
	else if(layer=="bevel")
		return "layer_stylize_bevel_icon";
	else if(layer=="shade")
		return "layer_stylize_shade_icon";
	// Transform Layers
	else if(layer=="rotate")
		return "layer_transform_rotate_icon";
	else if(layer=="translate")
		return "layer_transform_translate_icon";
	else if(layer=="zoom")
		return "layer_transform_scale_icon";
	else if(layer=="ghost_group")
		return "layer_other_ghostgroup_icon";
	else
		return "layer_icon";
}

Glib::RefPtr<Gdk::Pixbuf>
studio::get_tree_pixbuf_layer(const synfig::String &layer)
{
	int width, height;
	Gtk::IconSize::lookup(Gtk::ICON_SIZE_SMALL_TOOLBAR, width, height);
	Glib::RefPtr<Gdk::Pixbuf> icon = Gtk::IconTheme::get_default()->load_icon(layer_icon_name(layer), height, Gtk::ICON_LOOKUP_FORCE_SIZE);
	if (!icon)
		icon = Gtk::IconTheme::get_default()->load_icon("image-missing", height, Gtk::ICON_LOOKUP_FORCE_SIZE);
	return icon;
}

