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

/* === G L O B A L S ======================================================= */

// map Type::identifier -> Pixbuf
static std::map< int, Glib::RefPtr<Gdk::Pixbuf> > _tree_pixbuf_table_value_type;

// map ID -> (icon name, label)
static const std::map<std::string, std::pair<const char*, const char*>> known_icon_list = {
	// Types
	{"type_bool", {"type_bool_icon", N_("Bool")}},
	{"type_integer", {"type_integer_icon", N_("Integer")}},
	{"type_angle", {"type_angle_icon", N_("Angle")}},
	{"type_time", {"type_time_icon", N_("Time")}},
	{"type_real", {"type_real_icon", N_("Real")}},
	{"type_vector", {"type_vector_icon", N_("Vector")}},
	{"type_color", {"type_color_icon", N_("Color")}},
	{"type_segment", {"type_segment_icon", N_("Segment")}},
	{"type_blinepoint", {"type_splinepoint_icon", N_("Spline Point")}},
	{"type_list", {"type_list_icon", N_("List")}},
	{"type_string", {"type_string_icon", N_("String")}},
	{"type_canvas", {"type_canvas_icon", N_("Canvas")}},
	{"type_gradient", {"type_gradient_icon", N_("Gradient")}},

	// ToolBox Tools
	{"normal", {"tool_normal_icon", N_("Transform Tool")}},
	{"polygon", {"tool_polyline_icon", N_("Polygon Tool")}},
	{"bline", {"tool_spline_icon", N_("Spline Tool")}},
	{"bone", {"tool_skeleton_icon", N_("Skeleton Tool")}},
	{"eyedrop", {"tool_eyedrop_icon", N_("Eyedropper Tool")}},
	{"fill", {"tool_fill_icon", N_("Fill Tool")}},
	{"draw", {"tool_draw_icon", N_("Draw Tool")}},
	{"lasso", {"tool_cutout_icon", N_("Cutout Tool")}},
	{"brush", {"tool_brush_icon", N_("Brush Tool")}},
	{"sketch", {"tool_sketch_icon", N_("Sketch Tool")}},
	{"circle", {"tool_circle_icon", N_("Circle Tool")}},
	{"rectangle", {"tool_rectangle_icon", N_("Rectangle Tool")}},
	{"smooth_move", {"tool_smooth_move_icon", N_("SmoothMove Tool")}},
	{"rotate", {"tool_rotate_icon", N_("Rotate Tool")}},
	{"width", {"tool_width_icon", N_("Width Tool")}},
	{"scale", {"tool_scale_icon", N_("Scale Tool")}},
	{"zoom", {"tool_zoom_icon", N_("Zoom Tool")}},
	{"mirror", {"tool_mirror_icon", N_("Mirror Tool")}},
	{"text", {"tool_text_icon", N_("Text Tool")}},
	{"gradient", {"tool_gradient_icon", N_("Gradient Tool")}},
	{"star", {"tool_star_icon", N_("Star Tool")}},
	// ToolBox Others
	{"reset_colors", {"reset_colors_icon", N_("Reset Colors")}},
	{"swap_colors", {"swap_colors_icon", N_("Swap Colors")}},
	{"value_node", {"valuenode_icon", N_("ValueNode")}},
	{"valuenode_forbidanimation", {"valuenode_forbidanimation_icon", N_("ValueNode Forbid Animation")}},
	{"rename", {"rename_icon", N_("Rename")}},
	{"canvas", {"canvas_icon", N_("Canvas")}},
	{"canvas_new", {"canvas_icon", N_("New Canvas")}},

	// Document Related Actions
	{"about", {"about_icon", N_("About")}},
	{"new_doc", {"action_doc_new_icon", N_("New")}},
	{"open", {"action_doc_open_icon", N_("Open")}},
	{"save", {"action_doc_save_icon", N_("Save")}},
	{"save_as", {"action_doc_saveas_icon", N_("Save As")}},
	{"export", {"action_doc_saveas_icon", N_("Export")}},
	{"save_all", {"action_doc_saveall_icon", N_("Save All")}},
	{"redo", {"action_doc_redo_icon", N_("Redo")}},
	{"undo", {"action_doc_undo_icon", N_("Undo")}},

	// Ghost Layers
	{"layer_ghost_group", {"layer_other_ghostgroup_icon", N_("Group Ghost")}},

	{"info", {"info_icon", N_("Info Tool")}},
	{"group", {"set_icon", N_("Set")}},

	{"duplicate", {"duplicate_icon", N_("Duplicate")}},
	{"encapsulate", {"group_icon", N_("Group")}},
	{"encapsulate_switch", {"layer_other_switch_icon", N_("Group into Switch")}},
	{"encapsulate_filter", {"layer_other_filtergroup_icon", N_("Group into Filter")}},
	{"select_all_child_layers", {"select_all_child_layers_icon", N_("Select All Child Layers")}},

	{"clear_undo", {"clear_undo_icon", N_("Clear Undo Stack")}},
	{"clear_redo", {"clear_redo_icon", N_("Clear Redo Stack")}},

	{"children", {"library_icon", N_("Library")}},
	{"curves", {"graphs_icon", N_("Graphs")}},
	{"keyframes", {"keyframe_icon", N_("Keyframes")}},
	{"meta_data", {"meta_data_icon", N_("MetaData")}},
	{"navigator", {"navigator_icon", N_("Navigator")}},
	{"timetrack", {"time_track_icon", N_("Time Track")}},
	{"history", {"history_icon", N_("History")}},
	{"palette", {"palette_icon", N_("Palette")}},
	{"params", {"parameters_icon", N_("Parameters")}},

	{"keyframe_lock_past_off", {"keyframe_lock_past_off_icon", N_("Past keyframes unlocked")}},
	{"keyframe_lock_past_on", {"keyframe_lock_past_on_icon", N_("Past keyframes locked")}},
	{"keyframe_lock_future_off", {"keyframe_lock_future_off_icon", N_("Future keyframes unlocked")}},
	{"keyframe_lock_future_on", {"keyframe_lock_future_on_icon", N_("Future keyframes locked")}},

	{"animate_mode_off", {"animate_mode_off_icon", N_("Animate Mode Off")}},
	{"animate_mode_on", {"animate_mode_on_icon", N_("Animate Mode On")}},

	{"jack", {"jack_icon", N_("JACK")}},

	{"set_outline_color", {"set_outline_color_icon", N_("Set as Outline")}},
	{"set_fill_color", {"set_fill_color_icon", N_("Set as Fill")}},

	{"animate_seek_begin", {"animate_seek_begin_icon", N_("Seek to Begin")}},
	{"animate_seek_prev_keyframe", {"animate_seek_prev_keyframe_icon", N_("Seek to Previous Keyframe")}},
	{"animate_seek_prev_frame", {"animate_seek_prev_frame_icon", N_("Seek to Previous Frame")}},
	{"animate_play", {"animate_play_icon", N_("Play")}},
	{"animate_stop", {"animate_stop_icon", N_("Stop")}},
	{"animate_pause", {"animate_pause_icon", N_("Pause")}},
	{"animate_seek_next_frame", {"animate_seek_next_frame_icon", N_("Seek to Next frame")}},
	{"animate_seek_next_keyframe", {"animate_seek_next_keyframe_icon", N_("Seek to Next Keyframe")}},
	{"animate_seek_end", {"animate_seek_end_icon", N_("Seek to End")}},
	{"animate_loop", {"animate_loop_icon", N_("Animate Loop")}},
	{"animate_bounds", {"animate_bounds_icon", N_("Play Bounds")}},
	{"animate_bound_lower", {"animate_bound_lower_icon", N_("Lower Bound")}},
	{"animate_bound_upper", {"animate_bound_upper_icon", N_("Upper Bound")}},

	{"add_to_group", {"action_add_to_set_icon", N_("Add Layer to Set")}},
	{"remove_from_group", {"action_remove_from_set_icon", N_("Remove Layer from Set")}},
	{"set_desc", {"action_set_layer_description_icon", N_("Set Layer Description")}},
	{"export", {"action_export_icon", N_("Export Value Node")}},
	{"unexport", {"action_unexport_icon", N_("Unexport Value Node")}},
	{"flat_interpolation", {"action_flat_interpolation_icon", N_("Set Interpolation to Flat")}},
	{"interpolate_interpolation", {"action_interpolate_interpolation_icon", N_("Set Interpolation to Interpolate")}},
	{"peak_interpolation", {"action_peak_interpolation_icon", N_("Set Interpolation to Peak")}},
	{"offpeak_interpolation", {"action_offpeak_interpolation_icon", N_("Set Interpolation to Off-Peak")}},
	{"rounded_interpolation", {"action_rounded_interpolation_icon", N_("Set Interpolation to Rounded")}},
	{"innerrounded_interpolation", {"action_innerrounded_interpolation_icon", N_("Set Interpolation to Inner Rounded")}},
	{"squared_interpolation", {"action_squared_interpolation_icon", N_("Set Interpolation to Squared")}},

	{"toggle_duck_position", {"duck_position_icon", N_("Toggle position handles")}},
	{"toggle_duck_vertex", {"duck_vertex_icon", N_("Toggle vertex handles")}},
	{"toggle_duck_tangent", {"duck_tangent_icon", N_("Toggle tangent handles")}},
	{"toggle_duck_radius", {"duck_radius_icon", N_("Toggle radius handles")}},
	{"toggle_duck_width", {"duck_width_icon", N_("Toggle width handles")}},
	{"toggle_duck_angle", {"duck_angle_icon", N_("Toggle angle handles")}},

	{"toggle_show_grid", {"show_grid_icon", N_("Toggle show grid")}},
	{"toggle_snap_grid", {"snap_grid_icon", N_("Toggle snap grid")}},
	{"toggle_show_guide", {"show_guideline_icon", N_("Toggle show guide")}},
	{"toggle_snap_guide", {"snap_guideline_icon", N_("Toggle snap guide")}},

	{"toggle_onion_skin", {"onion_skin_icon", N_("Toggle onion skin")}},

	{"toggle_background_rendering", {"background_rendering_icon", N_("Toggle background rendering")}},

	{"increase_resolution", {"incr_resolution_icon", N_("Increase resolution")}},
	{"decrease_resolution", {"decr_resolution_icon", N_("Decrease resolution")}},

	{"preview_options", {"preview_options_icon", N_("Preview Options Dialog")}},
	{"render_options", {"render_options_icon", N_("Render Options Dialog")}},

	{"utils_chain_link_on", {"utils_chain_link_on_icon", N_("Linked")}},
	{"utils_chain_link_off", {"utils_chain_link_off_icon", N_("Unlinked")}},
	{"utils_timetrack_align", {"utils_timetrack_align_icon", N_("Utils Timetrack align")}},
};

// map layer name -> icon name
static const std::map<std::string, std::string> layer_icon_names = {
	// Blur Layers
	{"blur",          "layer_blur_blur_icon"},
	{"motion_blur",   "layer_blur_motion_icon"},
	{"radial_blur",   "layer_blur_radial_icon"},
	// Distortion Layers
	{"curve_warp",           "layer_distortion_curvewarp_icon"},
	{"inside_out",           "layer_distortion_insideout_icon"},
	{"noise_distort",        "layer_distortion_noise_icon"},
	{"skeleton_deformation", "layer_distortion_skeletondeformation_icon"},
	{"spherize",             "layer_distortion_spherize_icon"},
	{"stretch",              "layer_distortion_stretch_icon"},
	{"twirl",                "layer_distortion_twirl_icon"},
	{"warp",                 "layer_distortion_warp_icon"},
	// Example Layers
	{"metaballs",            "layer_example_metaballs_icon"},
	{"simple_circle",        "layer_example_simplecircle_icon"},
	// Filter Layers
	{"chromakey",    "layer_filter_chromakey_icon"},
	{"clamp",        "layer_filter_clamp_icon"},
	{"colorcorrect", "layer_filter_colorcorrect_icon"},
	{"halftone2",    "layer_filter_halftone2_icon"},
	{"halftone3",    "layer_filter_halftone3_icon"},
	{"lumakey",      "layer_filter_lumakey_icon"},
	// Fractal Layers
	{"mandelbrot",   "layer_fractal_mandelbrot_icon"},
	{"julia",        "layer_fractal_julia_icon"},
	// Geometry Layers
	{"checker_board",    "layer_geometry_checkerboard_icon"},
	{"circle",           "layer_geometry_circle_icon"},
	{"outline",          "layer_geometry_outline_icon"},
	{"advanced_outline", "layer_geometry_advanced_outline_icon"},
	{"polygon",          "layer_geometry_polygon_icon"},
	{"rectangle",        "layer_geometry_rectangle_icon"},
	{"region",           "layer_geometry_region_icon"},
	{"solid_color",      "layer_geometry_solidcolor_icon"},
	{"star",             "layer_geometry_star_icon"},
	// Gradient Layers
	{"conical_gradient", "layer_gradient_conical_icon"},
	{"curve_gradient",   "layer_gradient_curve_icon"},
	{"noise",            "layer_gradient_noise_icon"},
	{"linear_gradient",  "layer_gradient_linear_icon"},
	{"radial_gradient",  "layer_gradient_radial_icon"},
	{"spiral_gradient",  "layer_gradient_spiral_icon"},
	// Other Layers
	{"duplicate",    "layer_other_duplicate_icon"},
	{"import",       "layer_other_importimage_icon"},
	{"filter_group", "layer_other_filtergroup_icon"},
	{"group",        "layer_other_group_icon"}, // "paste_canvas"
	{"plant",        "layer_other_plant_icon"},
	{"freetime",     "layer_other_freetime_icon"},
	{"stroboscope",  "layer_other_stroboscope_icon"},
	{"skeleton",     "layer_other_skeleton_icon"},
	{"super_sample", "layer_other_supersample_icon"},
	{"switch",       "layer_other_switch_icon"},
	{"text",         "layer_other_text_icon"},
	{"sound",        "layer_other_sound_icon"},
	{"timeloop",     "layer_other_timeloop_icon"},
	{"xor_pattern",  "layer_other_xorpattern_icon"},
	// Stylize Layers
	{"bevel",        "layer_stylize_bevel_icon"},
	{"shade",        "layer_stylize_shade_icon"},
	// Transform Layers
	{"rotate",       "layer_transform_rotate_icon"},
	{"translate",    "layer_transform_translate_icon"},
	{"zoom",         "layer_transform_scale_icon"},
	// Fake Layers
	{"ghost_group",  "layer_other_ghostgroup_icon"},
};

/* === M E T H O D S ======================================================= */

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
IconController::init_icon(const synfig::String& name, const synfig::filesystem::Path& iconfile, const synfig::String& desc)
{
	Gtk::StockItem stockitem(Gtk::StockID("synfig-" + name), desc);
	Gtk::Stock::add(stockitem);
	Glib::RefPtr<Gtk::IconSet> icon_set = Gtk::IconSet::create();
	Gtk::IconSource icon_source;
	icon_source.set_direction_wildcarded();
	icon_source.set_state_wildcarded();
	icon_source.set_size_wildcarded();
	icon_source.set_filename(iconfile.u8string());
	icon_set->add_source(icon_source);
	icon_factory->add(stockitem.get_stock_id(), icon_set);
}

void
IconController::init_icons(const synfig::filesystem::Path& path_to_icons)
{
	std::string u8path = path_to_icons.u8string();
	try{
		Gtk::Window::set_default_icon_from_file(u8path + "/synfig_icon." IMAGE_EXT);
	} catch(...) {
		synfig::warning(_("Unable to open %s%s"), u8path.c_str(), "/synfig_icon." IMAGE_EXT);
	}


	for (const auto& item : known_icon_list)
		init_icon(item.first, u8path + '/' + item.second.first + "." IMAGE_EXT, _(item.second.second));

	icon_factory->add_default();

	Gtk::IconSize::register_new("synfig-tiny_icon", 8, 8);
	Gtk::IconSize::register_new("synfig-small_icon",12,12);
	Gtk::IconSize::register_new("synfig-small_icon_16x16",16,16);

	int width_small_toolbar, height_small_toolbar;
	Gtk::IconSize::lookup(Gtk::ICON_SIZE_SMALL_TOOLBAR, width_small_toolbar, height_small_toolbar);

	for (Type *type = Type::get_first(); type != nullptr; type = type->get_next()) {
		std::string icon_name = value_icon_name(*type);

		if (!Gtk::IconTheme::get_default()->has_icon(icon_name))
			icon_name = "image-missing";

		Glib::RefPtr<Gdk::Pixbuf> icon = Gtk::IconTheme::get_default()->load_icon(icon_name, height_small_toolbar, Gtk::ICON_LOOKUP_FORCE_SIZE);
		_tree_pixbuf_table_value_type[type->identifier] = icon;
	}
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

std::string
studio::get_action_icon_name(const synfigapp::Action::BookEntry& action)
{
	// maps action task -> icon name
	const std::map<std::string, std::string> action_icon_map = {
		{"add",         "list-add"},
		{"insert",      "list-add"},
		{"remove",      "edit-delete"},
		{"connect",     "gtk-connect"},
		{"disconnect",  "gtk-disconnect"},
		{"raise",       "go-up"},
		{"lower",       "go-down"},
		{"move_top",    "go-top"},
		{"move_bottom", "go-bottom"},
		{"set_on",      "gtk-yes"},
		{"set_off",     "gtk-no"},
	};
	auto iter = action_icon_map.find(action.task);
	if (iter != action_icon_map.end())
		return iter->second;

	auto iter2 = known_icon_list.find(action.task);
	if (iter2 != known_icon_list.end())
		return iter2->second.first;
	return "image-missing";
}

std::string
studio::layer_icon_name(const synfig::String& layer_name)
{
	auto iter = layer_icon_names.find(layer_name);
	if (iter != layer_icon_names.end())
		return iter->second;

	return "layer_icon";
}

Glib::RefPtr<Gdk::Pixbuf>
studio::get_tree_pixbuf_layer(const synfig::String &layer)
{
	return get_tree_pixbuf_from_icon_name(layer_icon_name(layer));
}

std::string
studio::state_icon_name(const synfig::String& state)
{
	auto iter = known_icon_list.find(state);
	if (iter == known_icon_list.end()) {
		synfig::warning(_("state icon name not defined: %s"), state.c_str());
		return "image-missing";
	}
	return iter->second.first;
}

Glib::RefPtr<Gdk::Pixbuf>
studio::get_tree_pixbuf_from_icon_name(const synfig::String& icon_name)
{
	int width, height;
	Gtk::IconSize::lookup(Gtk::ICON_SIZE_SMALL_TOOLBAR, width, height);
	Glib::RefPtr<Gdk::Pixbuf> icon = Gtk::IconTheme::get_default()->load_icon(icon_name, height, Gtk::ICON_LOOKUP_FORCE_SIZE);
	if (!icon)
		icon = Gtk::IconTheme::get_default()->load_icon("image-missing", height, Gtk::ICON_LOOKUP_FORCE_SIZE);
	return icon;
}
