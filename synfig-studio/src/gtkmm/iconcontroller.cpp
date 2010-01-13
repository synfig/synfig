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
**	Copyright (c) 2009 Carlos López
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

#include "iconcontroller.h"
#include <synfig/valuenode_const.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <synfigapp/action.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

#ifdef WIN32
#	ifdef IMAGE_DIR
#		undef IMAGE_DIR
#		define IMAGE_DIR "share\\pixmaps"
#	endif
#endif

#ifndef IMAGE_DIR
#	define IMAGE_DIR "/usr/local/share/pixmaps"
#endif


#ifndef IMAGE_EXT
#	define IMAGE_EXT	"png"
#endif

/* === M E T H O D S ======================================================= */

static Glib::RefPtr<Gdk::Pixbuf> _tree_pixbuf_table_value_type[(int)synfig::ValueBase::TYPE_END];

#ifdef WIN32
IconController::IconController(const synfig::String& basepath)
#else
IconController::IconController(const synfig::String& /*basepath*/)
#endif
{
	Gtk::IconSource icon_source;
	icon_source.set_direction_wildcarded();
	icon_source.set_state_wildcarded();
	icon_source.set_size_wildcarded();
	icon_factory=Gtk::IconFactory::create();

	std::string path_to_icons;
#ifdef WIN32
	path_to_icons=basepath+ETL_DIRECTORY_SEPARATOR+".."+ETL_DIRECTORY_SEPARATOR+IMAGE_DIR;
#else
	path_to_icons=IMAGE_DIR;
#endif

	char* synfig_root=getenv("SYNFIG_ROOT");
	if(synfig_root) {
		path_to_icons=synfig_root;
		path_to_icons+=ETL_DIRECTORY_SEPARATOR;
		path_to_icons+="share";
		path_to_icons+=ETL_DIRECTORY_SEPARATOR;
		path_to_icons+="pixmaps";
		path_to_icons+=ETL_DIRECTORY_SEPARATOR;
		path_to_icons+="synfigstudio";
	}
	path_to_icons+=ETL_DIRECTORY_SEPARATOR;

	try{
	Gtk::Window::set_default_icon_from_file(path_to_icons+"synfig_icon."+IMAGE_EXT);
	} catch(...)
	{
		synfig::warning("Unable to open "+path_to_icons+"synfig_icon."+IMAGE_EXT);
	}

/*
#define INIT_STOCK_ICON(name,iconfile,desc)							\
	stock_##name=Gtk::StockItem(Gtk::StockID("synfig-" #name),desc);   			\
	Gtk::Stock::add(stock_##name);								\
	icon_source.set_filename(path_to_icons+iconfile);							\
	icon_##name.add_source(icon_source);						\
	icon_factory->add(stock_##name.get_stock_id(),icon_##name)
*/
#define INIT_STOCK_ICON(name,iconfile,desc){							\
	Gtk::StockItem stockitem(Gtk::StockID("synfig-" #name),desc); \
	Gtk::Stock::add(stockitem);								\
	Gtk::IconSet icon_set;									\
	icon_source.set_filename(path_to_icons+iconfile);							\
	icon_set.add_source(icon_source);						\
	icon_factory->add(stockitem.get_stock_id(),icon_set); \
	}

#define INIT_STOCK_ICON_CLONE(name,stockid,desc){							\
	Gtk::StockItem stockitem(Gtk::StockID("synfig-" #name),desc); \
	Gtk::Stock::add(stockitem);								\
	Gtk::IconSet icon_set;									\
	if(Gtk::Stock::lookup(stockitem.get_stock_id(),icon_set))	\
	icon_factory->add(stockitem.get_stock_id(),icon_set); \
	}

#define INIT_STOCK_ITEM(name,desc)							\
	stock_##name=Gtk::StockItem(Gtk::StockID("synfig-" #name),desc);   			\
	Gtk::Stock::add(stock_##name);

	INIT_STOCK_ICON(bool,"bool_icon."IMAGE_EXT,_("Bool"));
	INIT_STOCK_ICON(integer,"integer_icon."IMAGE_EXT,_("Integer"));
	INIT_STOCK_ICON(angle,"angle_icon."IMAGE_EXT,_("Angle"));
	INIT_STOCK_ICON(time,"time_icon."IMAGE_EXT,_("Time"));
	INIT_STOCK_ICON(real,"real_icon."IMAGE_EXT,_("Real"));
	INIT_STOCK_ICON(vector,"vector_icon."IMAGE_EXT,_("Vector"));
	INIT_STOCK_ICON(color,"color_icon."IMAGE_EXT,_("Color"));
	INIT_STOCK_ICON(segment,"segment_icon."IMAGE_EXT,_("Segment"));
	INIT_STOCK_ICON(blinepoint,"blinepoint_icon."IMAGE_EXT,_("BLine Point"));
	INIT_STOCK_ICON(list,"list_icon."IMAGE_EXT,_("Rename"));
	INIT_STOCK_ICON(canvas,"canvas_icon."IMAGE_EXT,_("Canvas"));
	INIT_STOCK_ICON(string,"string_icon."IMAGE_EXT,_("Rename"));

	INIT_STOCK_ICON(reset_colors,"reset_colors_icon."IMAGE_EXT,_("Reset Colors"));
	INIT_STOCK_ICON(swap_colors,"swap_colors_icon."IMAGE_EXT,_("Swap Colors"));
	INIT_STOCK_ICON(value_node,"valuenode_icon."IMAGE_EXT,_("ValueNode"));
	INIT_STOCK_ICON(about,"about_icon."IMAGE_EXT,_("About"));
	INIT_STOCK_ICON(rename,"rename_icon."IMAGE_EXT,_("Rename"));
	INIT_STOCK_ICON(canvas_pointer,"canvas_pointer_icon."IMAGE_EXT,_("Rename"));
	INIT_STOCK_ICON(canvas_new,"canvas_icon."IMAGE_EXT,_("New Canvas"));
	INIT_STOCK_ICON(saveall,"saveall_icon."IMAGE_EXT,_("Save All"));
	INIT_STOCK_ICON(layer,"layer_icon."IMAGE_EXT,_("Layer"));
	INIT_STOCK_ICON(layer_pastecanvas,"pastecanvas_icon."IMAGE_EXT,_("Paste Canvas"));
	INIT_STOCK_ICON(plant,"plant_icon."IMAGE_EXT,"");
	INIT_STOCK_ICON(group,"group_icon."IMAGE_EXT,_("Group"));
	INIT_STOCK_ICON(grid_enable,"grid_enable_icon."IMAGE_EXT,_("Show Grid"));
	INIT_STOCK_ICON(grid_disable,"grid_disable_icon."IMAGE_EXT,_("Hide Grid"));
	INIT_STOCK_ICON(grid_snap_enable,"grid_snap_enable_icon."IMAGE_EXT,_("Enable Grid Snap"));
	INIT_STOCK_ICON(grid_snap_disable,"grid_snap_disable_icon."IMAGE_EXT,_("Disable Grid Snap"));
	INIT_STOCK_ICON(duplicate,"duplicate_icon."IMAGE_EXT,_("Duplicate"));
	INIT_STOCK_ICON(encapsulate,"encapsulate_icon."IMAGE_EXT,_("Encapsulate"));
	INIT_STOCK_ICON(select_all_child_layers,"select_all_child_layers_icon."IMAGE_EXT,_("Select All Child Layers"));

	INIT_STOCK_ICON(clear_undo,"clear_undo_icon."IMAGE_EXT,_("Clear Undo Stack"));
	INIT_STOCK_ICON(clear_redo,"clear_redo_icon."IMAGE_EXT,_("Clear Redo Stack"));

	INIT_STOCK_ICON(children,"children_icon."IMAGE_EXT,_("Children"));
	INIT_STOCK_ICON(curves,"curves_icon."IMAGE_EXT,_("Curves"));
	INIT_STOCK_ICON(keyframes,"keyframe_icon."IMAGE_EXT,_("Keyframes"));
	INIT_STOCK_ICON(meta_data,"meta_data_icon."IMAGE_EXT,_("MetaData"));
	INIT_STOCK_ICON(navigator,"navigator_icon."IMAGE_EXT,_("Navigator"));
	INIT_STOCK_ICON(timetrack,"time_track_icon."IMAGE_EXT,_("Time Track"));

	INIT_STOCK_ICON(keyframe_lock_all,"keyframe_lock_all."IMAGE_EXT,_("All Keyframes Locked"));
	INIT_STOCK_ICON(keyframe_lock_past,"keyframe_lock_past."IMAGE_EXT,_("Past Keyframes Locked"));
	INIT_STOCK_ICON(keyframe_lock_future,"keyframe_lock_future."IMAGE_EXT,_("Future Keyframes Locked"));
	INIT_STOCK_ICON(keyframe_lock_none,"keyframe_lock_none."IMAGE_EXT,_("No Keyframes Locked"));

	INIT_STOCK_ICON(set_outline_color,"set_outline_color."IMAGE_EXT,_("Set as Outline"));
	INIT_STOCK_ICON(set_fill_color,"set_fill_color."IMAGE_EXT,_("Set as Fill"));

	INIT_STOCK_ICON(seek_begin,"seek_begin."IMAGE_EXT,_("Seek to Begin"));
	INIT_STOCK_ICON(seek_prev_frame,"seek_prev_frame."IMAGE_EXT,_("Previous Frame"));
	INIT_STOCK_ICON(seek_next_frame,"seek_next_frame."IMAGE_EXT,_("Next Frame"));
	INIT_STOCK_ICON(seek_end,"seek_end."IMAGE_EXT,_("Seek to End"));

	INIT_STOCK_ICON(toggle_duck_position,"duck_position_icon."IMAGE_EXT,_("Toggle position ducks"));
	INIT_STOCK_ICON(toggle_duck_vertex,"duck_vertex_icon."IMAGE_EXT,_("Toggle vertex ducks"));
	INIT_STOCK_ICON(toggle_duck_tangent,"duck_tangent_icon."IMAGE_EXT,_("Toggle tangent ducks"));
	INIT_STOCK_ICON(toggle_duck_radius,"duck_radius_icon."IMAGE_EXT,_("Toggle radius ducks"));
	INIT_STOCK_ICON(toggle_duck_width,"duck_width_icon."IMAGE_EXT,_("Toggle width ducks"));
	INIT_STOCK_ICON(toggle_duck_angle,"duck_angle_icon."IMAGE_EXT,_("Toggle angle ducks"));

	INIT_STOCK_ICON(toggle_show_grid,"show_grid_icon."IMAGE_EXT,_("Toggle show grid"));
	INIT_STOCK_ICON(toggle_snap_grid,"snap_grid_icon."IMAGE_EXT,_("Toggle snap grid"));

	INIT_STOCK_ICON(toggle_onion_skin,"onion_skin_icon."IMAGE_EXT,_("Toggle onion skin"));

	INIT_STOCK_ICON(increase_resolution,"incr_resolution_icon."IMAGE_EXT,_("Increase resolution"));
	INIT_STOCK_ICON(decrease_resolution,"decr_resolution_icon."IMAGE_EXT,_("Decrease resolution"));

	INIT_STOCK_ICON(preview_options,"preview_options_icon."IMAGE_EXT,_("Preview Options Dialog"));
	INIT_STOCK_ICON(render_options,"render_options_icon."IMAGE_EXT,_("Render Options Dialog"));

	INIT_STOCK_ICON_CLONE(cvs_add,"gtk-add",_("CVS Add"));
	INIT_STOCK_ICON_CLONE(cvs_update,"gtk-open",_("CVS Update"));
	INIT_STOCK_ICON_CLONE(cvs_commit,"gtk-save",_("CVS Commit"));
	INIT_STOCK_ICON_CLONE(cvs_revert,"gtk-revert",_("CVS Revert"));

	// Tools
	INIT_STOCK_ICON(normal,"normal_icon."IMAGE_EXT,_("Normal Tool"));
	INIT_STOCK_ICON(transform,"transform_icon."IMAGE_EXT,_("Transform Tool"));
	INIT_STOCK_ICON(polygon,"polyline_icon."IMAGE_EXT,_("Polygon Tool"));
	INIT_STOCK_ICON(bline,"bline_icon."IMAGE_EXT,_("BLine Tool"));
	INIT_STOCK_ICON(eyedrop,"eyedrop_icon."IMAGE_EXT,_("Eyedrop Tool"));
	INIT_STOCK_ICON(fill,"fill_icon."IMAGE_EXT,_("Fill Tool"));
	INIT_STOCK_ICON(draw,"draw_icon."IMAGE_EXT,_("Draw Tool"));
	INIT_STOCK_ICON(sketch,"sketch_icon."IMAGE_EXT,_("Sketch Tool"));
	INIT_STOCK_ICON(circle,"circle_icon."IMAGE_EXT,_("Circle Tool"));
	INIT_STOCK_ICON(rectangle,"rectangle_icon."IMAGE_EXT,_("Rectangle Tool"));
	INIT_STOCK_ICON(smooth_move,"smooth_move_icon."IMAGE_EXT,_("SmoothMove Tool"));
	INIT_STOCK_ICON(rotate,"rotate_icon."IMAGE_EXT,"Rotate Tool");
	INIT_STOCK_ICON(width,"width_icon."IMAGE_EXT,_("Width Tool"));
	INIT_STOCK_ICON(scale,"scale_icon."IMAGE_EXT,"Scale Tool");
	INIT_STOCK_ICON(zoom,"zoom_icon."IMAGE_EXT,_("Zoom Tool"));
	INIT_STOCK_ICON(info,"info_icon."IMAGE_EXT,_("Info Tool"));
	INIT_STOCK_ICON(mirror,"mirror_icon."IMAGE_EXT,_("Mirror Tool"));
	INIT_STOCK_ICON(text,"text_icon."IMAGE_EXT,"Text Tool");
	INIT_STOCK_ICON(gradient,"gradient_icon."IMAGE_EXT,_("Gradient Tool"));
	INIT_STOCK_ICON(star,"star_icon."IMAGE_EXT,_("Star Tool"));

	icon_factory->add_default();

	Gtk::IconSize::register_new("synfig-small_icon",12,12);
	Gtk::IconSize::register_new("synfig-small_icon_16x16",16,16);

	for(int i(0);i<(int)ValueBase::TYPE_END;i++)
		_tree_pixbuf_table_value_type[i]=Gtk::Button().render_icon(value_icon(ValueBase::Type(i)),Gtk::ICON_SIZE_SMALL_TOOLBAR);

}

IconController::~IconController()
{
	for(int i(0);i<(int)ValueBase::TYPE_END;i++)
		_tree_pixbuf_table_value_type[i]=Glib::RefPtr<Gdk::Pixbuf>();

	icon_factory->remove_default();
}

Gdk::Cursor
IconController::get_normal_cursor()
{
	return Gdk::Cursor(Gdk::TOP_LEFT_ARROW);
}

Gdk::Cursor
IconController::get_tool_cursor(const Glib::ustring& name,const Glib::RefPtr<Gdk::Window>& window)
{
	Glib::RefPtr<Gdk::Pixmap> pixmap;
	pixmap=Gdk::Pixmap::create(window, 64, 64, 8);
	pixmap->set_colormap(window->get_colormap());
	//pixmap->set_colormap(Gdk::Colormap::create(pixmap->get_visual(),false));
	Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	pixbuf=Gtk::Button().render_icon(Gtk::StockID("synfig-"+name),Gtk::ICON_SIZE_SMALL_TOOLBAR);

	pixbuf->render_to_drawable_alpha(
		pixmap,
		0,0,	// SOURCE X,Y
		0,0,	// DEST X Y
		-1,-1,	// WIDTH HEIGHT
		Gdk::PIXBUF_ALPHA_FULL,	// (ignored)
		64,		//int alpha_threshold,
		Gdk::RGB_DITHER_MAX,		//RgbDither dither,
		2,2	//int x_dither, int y_dither
	);
/*
	pixmap->draw_pixbuf(
		Glib::RefPtr<const Gdk::GC>(0),	// GC
		pixbuf,
		0, 0, // Source X,Y
		0, 0, // Dest X,Y
		-1, -1, // Width, Height
		Gdk::RGB_DITHER_MAX, // Dither
		0,0 // Dither X,Y
	);
*/

	Gdk::Color FG("#000000");
	Gdk::Color BG("#FF00FF");

  	return Gdk::Cursor(pixmap, pixmap, FG, BG, 0, 0);
}

Gtk::StockID
studio::value_icon(synfig::ValueBase::Type type)
{
		switch(type)
		{
		case ValueBase::TYPE_BOOL:
			return Gtk::StockID("synfig-bool");
			break;
		case ValueBase::TYPE_INTEGER:
			return Gtk::StockID("synfig-integer");
			break;
		case ValueBase::TYPE_ANGLE:
			return Gtk::StockID("synfig-angle");
			break;
		case ValueBase::TYPE_TIME:
			return Gtk::StockID("synfig-time");
			break;
		case ValueBase::TYPE_REAL:
			return Gtk::StockID("synfig-real");
			break;
		case ValueBase::TYPE_VECTOR:
			return Gtk::StockID("synfig-vector");
			break;
		case ValueBase::TYPE_COLOR:
			return Gtk::StockID("synfig-color");
			break;
		case ValueBase::TYPE_SEGMENT:
			return Gtk::StockID("synfig-segment");
			break;
		case ValueBase::TYPE_BLINEPOINT:
			return Gtk::StockID("synfig-blinepoint");
			break;
		case ValueBase::TYPE_LIST:
			return Gtk::StockID("synfig-list");
			break;
		case ValueBase::TYPE_CANVAS:
			return Gtk::StockID("synfig-canvas_pointer");
			break;
		case ValueBase::TYPE_STRING:
			return Gtk::StockID("synfig-string");
			break;
		case ValueBase::TYPE_GRADIENT:
			return Gtk::StockID("synfig-gradient");
			break;
		case ValueBase::TYPE_NIL:
		default:
			return Gtk::StockID("synfig-unknown");
			break;
		}
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
studio::get_tree_pixbuf(synfig::ValueBase::Type type)
{
	//return Gtk::Button().render_icon(value_icon(type),Gtk::ICON_SIZE_SMALL_TOOLBAR);
	return _tree_pixbuf_table_value_type[int(type)];
}

#ifdef WIN32
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

#ifdef WIN32
#define DELETE TEMPORARY_DELETE_MACRO
#undef TEMPORARY_DELETE_MACRO
#endif

Gtk::StockID
studio::layer_icon(const synfig::String &layer)
{
	if(layer=="PasteCanvas" || layer=="pastecanvas" || layer=="paste_canvas")
		return Gtk::StockID("synfig-layer_pastecanvas");
	else if(layer=="rotate")
		return Gtk::StockID("synfig-rotate");
	else if(layer=="zoom")
		return Gtk::StockID("synfig-zoom");
	else if(layer=="region")
		return Gtk::StockID("synfig-bline");
	else if(layer=="polygon")
		return Gtk::StockID("synfig-polygon");
	else if(layer=="outline")
		return Gtk::StockID("synfig-width");
	else if(layer=="circle")
		return Gtk::StockID("synfig-circle");
	else if(layer=="rectangle")
		return Gtk::StockID("synfig-rectangle");
	else if(layer=="star")
		return Gtk::StockID("synfig-star");
	else if(layer=="plant")
		return Gtk::StockID("synfig-plant");
	else if(layer=="text")
		return Gtk::StockID("synfig-text");
	else if(layer.find("gradient")!=String::npos)
		return Gtk::StockID("synfig-gradient");
	else
		return Gtk::StockID("synfig-layer");
}

Glib::RefPtr<Gdk::Pixbuf>
studio::get_tree_pixbuf_layer(const synfig::String &layer)
{
	return Gtk::Button().render_icon(layer_icon(layer),Gtk::ICON_SIZE_SMALL_TOOLBAR);
}
