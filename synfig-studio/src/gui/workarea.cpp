/* === S Y N F I G ========================================================= */
/*!	\file workarea.cpp
**	\brief Work area
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2006 Yue Shi Lai
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2011 Nikita Kitaev
**	Copyright (c) 2016 caryoscelus
**  ......... ... 2018 Ivan Mahonin
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

#include <gtkmm/arrow.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/window.h>

#include <ETL/misc>

#include <synfig/general.h>

#include <synfig/blinepoint.h>
#include <synfig/rendering/renderer.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_bone.h>

#include <synfigapp/canvasinterface.h>

#include <gui/localization.h>

#include "helpers.h"
#include "canvasview.h"
#include "event_mouse.h"
#include "event_layerclick.h"
#include "event_keyboard.h"
#include "workarea.h"
#include "workarearenderer/workarearenderer.h"
#include "workarearenderer/renderer_background.h"
#include "workarearenderer/renderer_canvas.h"
#include "workarearenderer/renderer_grid.h"
#include "workarearenderer/renderer_guides.h"
#include "workarearenderer/renderer_timecode.h"
#include "workarearenderer/renderer_bonesetup.h"
#include "workarearenderer/renderer_ducks.h"
#include "workarearenderer/renderer_dragbox.h"
#include "workarearenderer/renderer_bbox.h"

#include <gui/exception_guard.h>
#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define THUMB_SIZE 128

#ifndef stratof
#define stratof(X) (atof((X).c_str()))
#define stratoi(X) (atoi((X).c_str()))
#endif

/* === G L O B A L S ======================================================= */

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

WorkArea::PushState::PushState(WorkArea &workarea):
	workarea(workarea),
	type_mask(workarea.get_type_mask()),
	allow_duck_clicks(workarea.get_allow_duck_clicks()),
	allow_bezier_clicks(workarea.get_allow_bezier_clicks()),
	allow_layer_clicks(workarea.get_allow_layer_clicks()) { }

WorkArea::PushState::~PushState()
{
	workarea.set_type_mask(type_mask);
	workarea.get_canvas_view()->toggle_duck_mask(Duck::TYPE_NONE);
	workarea.set_allow_duck_clicks(allow_duck_clicks);
	workarea.set_allow_bezier_clicks(allow_bezier_clicks);
	workarea.set_allow_layer_clicks(allow_layer_clicks);
}


WorkArea::DirtyTrap::DirtyTrap(WorkArea &work_area):
	work_area(work_area) { ++work_area.dirty_trap_count; }

WorkArea::DirtyTrap::~DirtyTrap()
{
	--work_area.dirty_trap_count;
	if (work_area.dirty_trap_queued) work_area.queue_render();
}


WorkArea::WorkArea(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface):
	Gtk::Grid(), /* 3 columns by 3 rows*/
	Duckmatic(canvas_interface),
	canvas_interface(canvas_interface),
	canvas(canvas_interface->get_canvas()),
	drawing_area(0),
	drawing_frame(0),
	hruler(0),
	vruler(0),
	scrollx_adjustment(Gtk::Adjustment::create(0,-4,4,0.01,0.1)),
	scrolly_adjustment(Gtk::Adjustment::create(0,-4,4,0.01,0.1)),
	zoomdial(0),
	curr_input_device(0),
	w(1),
	h(1),
	thumb_w(1),
	thumb_h(1),
	canvaswidth(0.0),
	canvasheight(0.0),
	pw(0.001),
	ph(0.001),
	last_event_time(0),
	progresscallback(0),
	drag_mode(DRAG_NONE),
	show_grid(false),
	show_guides(true),
	background_size(15,15),
	background_first_color(0.88, 0.88, 0.88),  /* light gray */
	background_second_color(0.65, 0.65, 0.65),  /* dark gray */
	jack_offset(0),
	low_resolution(false),
	meta_data_lock(false),
	last_focus_point(0,0),
	low_res_pixel_size(2),
	dirty_trap_count(0),
	dirty_trap_queued(0),
	onion_skin(false),
	highlight_active_bone(false),
	background_rendering(false),
	allow_duck_clicks(true),
	allow_bezier_clicks(true),
	allow_layer_clicks(true),
	curr_guide_is_x(false),
	solid_lines(true),
	timecode_width(0),
	timecode_height(0),
	bonesetup_width(0),
	bonesetup_height(0),
	active_bone_(0)
{
	// default onion
	onion_skins[0] = 1;
	onion_skins[1] = 0;

	renderer_canvas = new Renderer_Canvas();
	insert_renderer(new Renderer_Background,   0);
	insert_renderer(renderer_canvas,          10);
	insert_renderer(new Renderer_Grid,       100);
	insert_renderer(new Renderer_Guides,     200);
	insert_renderer(new Renderer_Ducks,      300);
	insert_renderer(new Renderer_BBox,       399);
	insert_renderer(new Renderer_Dragbox,    400);
	insert_renderer(new Renderer_Timecode,   500);
	insert_renderer(new Renderer_BoneSetup,  501);

	signal_duck_selection_changed().connect(sigc::mem_fun(*this,&studio::WorkArea::queue_draw));
	signal_duck_selection_single().connect(sigc::mem_fun(*this, &studio::WorkArea::on_duck_selection_single));
	signal_strokes_changed().connect(sigc::mem_fun(*this,&studio::WorkArea::queue_draw));
	signal_grid_changed().connect(sigc::mem_fun(*this,&studio::WorkArea::queue_draw));
	signal_grid_changed().connect(sigc::mem_fun(*this,&studio::WorkArea::save_meta_data));
	signal_sketch_saved().connect(sigc::mem_fun(*this,&studio::WorkArea::save_meta_data));
	canvas_interface->signal_active_bone_changed().connect(sigc::mem_fun(*this,&studio::WorkArea::set_active_bone_value_node));

	add_events(Gdk::KEY_PRESS_MASK);

	// Create drawing frame

  	drawing_area=manage(new class Gtk::DrawingArea());
	drawing_area->add_events( Gdk::KEY_PRESS_MASK      | Gdk::KEY_RELEASE_MASK
							| Gdk::BUTTON_PRESS_MASK   | Gdk::BUTTON_RELEASE_MASK
		                    | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK | Gdk::BUTTON3_MOTION_MASK
							| Gdk::POINTER_MOTION_MASK | Gdk::SCROLL_MASK         );
	drawing_area->set_hexpand(true);
	drawing_area->set_vexpand(true);
	drawing_area->show();
	drawing_frame=manage(new Gtk::Frame);
	drawing_frame->add(*drawing_area);
	drawing_frame->show();

	// Create the vertical and horizontal rulers

	hruler = manage(new Widget_Ruler(false));
	hruler->signal_event().connect(sigc::mem_fun(*this, &WorkArea::on_hruler_event));
	hruler->add_events( Gdk::BUTTON_PRESS_MASK   | Gdk::BUTTON_RELEASE_MASK
		              | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK | Gdk::POINTER_MOTION_MASK );
	hruler->show();
	hruler->set_hexpand(true);

	vruler = manage(new Widget_Ruler(true));
	vruler->signal_event().connect(sigc::mem_fun(*this, &WorkArea::on_vruler_event));
	vruler->add_events( Gdk::BUTTON_PRESS_MASK   | Gdk::BUTTON_RELEASE_MASK
		              | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK | Gdk::POINTER_MOTION_MASK );
	vruler->show();
	vruler->set_vexpand(true);

	// Create the menu button

	Gtk::Arrow *menubutton = manage(new Gtk::Arrow(Gtk::ARROW_RIGHT, Gtk::SHADOW_OUT));
	menubutton->set_size_request(18, 18);
	Gtk::EventBox *menubutton_box = manage(new Gtk::EventBox());
	menubutton_box->add(*menubutton);
	menubutton_box->add_events(Gdk::BUTTON_RELEASE_MASK);
	menubutton_box->signal_button_release_event().connect(
		sigc::bind_return(
			sigc::hide(
				sigc::mem_fun(*this, &WorkArea::popup_menu) ), true));
	menubutton_box->set_hexpand(false);
	menubutton_box->show_all();

	// Create scrollbars

	Gtk::Scrollbar *vscrollbar1 = manage(new Gtk::Scrollbar(get_scrolly_adjustment()));
	vscrollbar1->set_orientation(Gtk::ORIENTATION_VERTICAL);
	vscrollbar1->show();
	vscrollbar1->set_vexpand(true);

	Gtk::Scrollbar *hscrollbar1 = manage(new Gtk::Scrollbar(get_scrollx_adjustment()));
	hscrollbar1->set_hexpand(true);
	hscrollbar1->show();

	Gtk::IconSize iconsize = Gtk::IconSize::from_name("synfig-small_icon");
	zoomdial = manage(new ZoomDial(iconsize));
	zoomdial->signal_zoom_in().connect(sigc::mem_fun(*this, &studio::WorkArea::zoom_in));
	zoomdial->signal_zoom_out().connect(sigc::mem_fun(*this, &studio::WorkArea::zoom_out));
	zoomdial->signal_zoom_fit().connect(sigc::mem_fun(*this, &studio::WorkArea::zoom_fit));
	zoomdial->signal_zoom_norm().connect(sigc::mem_fun(*this, &studio::WorkArea::zoom_norm));
	zoomdial->signal_zoom_edit().connect(sigc::mem_fun(*this, &studio::WorkArea::zoom_edit));
	zoomdial->show();

	Gtk::Box *hbox = manage(new Gtk::Box());
	hbox->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	hbox->pack_end(*hscrollbar1, Gtk::PACK_EXPAND_WIDGET,0);
	hbox->pack_start(*zoomdial, Gtk::PACK_SHRINK,0);
	hbox->show();
	hbox->set_hexpand(true);

	// Layout
	attach(*menubutton_box, 0, 0, 1, 1);
	attach_next_to(*hruler, *menubutton_box, Gtk::POS_RIGHT, 1, 1);

	attach_next_to(*vruler, *menubutton_box, Gtk::POS_BOTTOM, 1, 1);
	attach_next_to(*drawing_frame, *vruler, Gtk::POS_RIGHT, 1, 1);
	attach_next_to(*vscrollbar1, *drawing_frame, Gtk::POS_RIGHT, 1, 1);

	attach_next_to(*hbox, *vruler, Gtk::POS_BOTTOM, 2, 1);


	// Attach signals

	drawing_area->signal_draw().connect(sigc::mem_fun(*this, &WorkArea::refresh));
	drawing_area->signal_event().connect(sigc::mem_fun(*this, &WorkArea::on_drawing_area_event));
	drawing_area->signal_size_allocate().connect(sigc::hide(sigc::mem_fun(*this, &WorkArea::refresh_dimension_info)));

	canvas_interface->signal_rend_desc_changed().connect(sigc::mem_fun(*this, &WorkArea::refresh_dimension_info));
	canvas_interface->signal_time_changed().connect(sigc::mem_fun(*this, &WorkArea::queue_draw));
	// When either of the scrolling adjustments change, then redraw.
	get_scrollx_adjustment()->signal_value_changed().connect(sigc::mem_fun(*this, &WorkArea::queue_scroll));
	get_scrolly_adjustment()->signal_value_changed().connect(sigc::mem_fun(*this, &WorkArea::queue_scroll));
	get_scrollx_adjustment()->signal_value_changed().connect(sigc::mem_fun(*this, &WorkArea::refresh_dimension_info));
	get_scrolly_adjustment()->signal_value_changed().connect(sigc::mem_fun(*this, &WorkArea::refresh_dimension_info));

	get_canvas()->signal_meta_data_changed("grid_size").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("grid_color").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("grid_snap").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("grid_show").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("guide_show").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("guide_x").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("guide_y").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("onion_skin").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("onion_skin_past").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("onion_skin_future").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("guide_snap").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("guide_color").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("sketch").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("solid_lines").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("background_size").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("background_first_color").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));
	get_canvas()->signal_meta_data_changed("background_second_color").connect(sigc::mem_fun(*this,&WorkArea::load_meta_data));


	set_focus_point(Point(0,0));

	// If no meta data in canvas, assume it's new file and save default
	if (!have_meta_data())
		save_meta_data();
	load_meta_data();

	// Load sketch
	{
		String data(canvas->get_meta_data("sketch"));
		if (!data.empty())
			if (!load_sketch(data))
				load_sketch(dirname(canvas->get_file_name()) + ETL_DIRECTORY_SEPARATOR + basename(data));
	}

	drawing_area->set_can_focus(true);
	refresh_dimension_info();
}

WorkArea::~WorkArea()
{
	set_drag_mode(DRAG_NONE);
	while(!renderer_set_.empty())
		erase_renderer(*renderer_set_.begin());
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		info("WorkArea::~WorkArea(): Deleted");
}

void
WorkArea::set_drag_mode(DragMode mode)
{
	if (drag_mode == mode) return;

	drag_mode = mode;

	if (lock_ducks && drag_mode == DRAG_NONE)
		lock_ducks.reset();
	else
	if (!lock_ducks && drag_mode != DRAG_NONE)
		lock_ducks = new LockDucks(get_canvas_view());
}

void
WorkArea::save_meta_data()
{
    ChangeLocale change_locale(LC_NUMERIC, "C");

    if(meta_data_lock)
		return;
	meta_data_lock=true;

	Vector s(get_grid_size());
	canvas_interface->set_meta_data("grid_size",strprintf("%f %f",s[0],s[1]));
	Color c(get_grid_color());
	canvas_interface->set_meta_data("grid_color",strprintf("%f %f %f",c.get_r(),c.get_g(),c.get_b()));
	c = get_guides_color();
	canvas_interface->set_meta_data("guide_color", strprintf("%f %f %f", c.get_r(), c.get_g(), c.get_b()));
	canvas_interface->set_meta_data("grid_snap", get_grid_snap() ? "1" : "0");
	canvas_interface->set_meta_data("guide_snap", get_guide_snap() ? "1" : "0");
	canvas_interface->set_meta_data("guide_show", get_show_guides() ? "1" : "0");
	canvas_interface->set_meta_data("grid_show", show_grid ? "1" : "0");
	canvas_interface->set_meta_data("jack_offset", strprintf("%f", (double)jack_offset));
	canvas_interface->set_meta_data("onion_skin", onion_skin ? "1" : "0");
	canvas_interface->set_meta_data("onion_skin_past", strprintf("%d", onion_skins[0]));
	canvas_interface->set_meta_data("onion_skin_future", strprintf("%d", onion_skins[1]));
	canvas_interface->set_meta_data("background_rendering", background_rendering ? "1" : "0");

	s = get_background_size();
	canvas_interface->set_meta_data("background_size", strprintf("%f %f", s[0], s[1]));
	c = get_background_first_color();
	canvas_interface->set_meta_data("background_first_color", strprintf("%f %f %f", c.get_r(), c.get_g(), c.get_b()));
	c = get_background_second_color();
	canvas_interface->set_meta_data("background_second_color", strprintf("%f %f %f", c.get_r(), c.get_g(), c.get_b()));

	{
		String data;
		GuideList::const_iterator iter;
		for(iter=get_guide_list_x().begin();iter!=get_guide_list_x().end();++iter)
		{
			if(!data.empty())
				data+=' ';
			data+=strprintf("%f",*iter);
		}
		if(!data.empty())
			canvas_interface->set_meta_data("guide_x",data);
		else if (!canvas->get_meta_data("guide_x").empty())
			canvas_interface->erase_meta_data("guide_x");

		data.clear();
		for(iter=get_guide_list_y().begin();iter!=get_guide_list_y().end();++iter)
		{
			if(!data.empty())
				data+=' ';
			data+=strprintf("%f",*iter);
		}
		if(!data.empty())
			canvas_interface->set_meta_data("guide_y",data);
		else if (!canvas->get_meta_data("guide_y").empty())
			canvas_interface->erase_meta_data("guide_y");
	}

	if(get_sketch_filename().size())
	{
		if(dirname(canvas->get_file_name())==dirname(get_sketch_filename()))
			canvas_interface->set_meta_data("sketch",basename(get_sketch_filename()));
		else
			canvas_interface->set_meta_data("sketch",get_sketch_filename());
	}

	meta_data_lock=false;
}

bool
WorkArea::have_meta_data()
{
	String data_size, data_show;

	data_size=canvas->get_meta_data("grid_size");
	data_show=canvas->get_meta_data("grid_show");

	if(data_size.empty() && !data_show.size())
		return false;

	return true;
}

void WorkArea::grab_focus()
{
	if (drawing_area)
		drawing_area->grab_focus();
}

void
WorkArea::load_meta_data()
{
	// we need to set locale careful, without calling functions and signals,
	// otherwise it can affect strings in GUI
    // ChangeLocale change_locale(LC_NUMERIC, "C");

    if(meta_data_lock)
		return;
	meta_data_lock=true;

	String data;

	data=canvas->get_meta_data("grid_size");
	if(!data.empty())
	{
		float gx(get_grid_size()[0]),gy(get_grid_size()[1]);

		String::iterator iter(find(data.begin(),data.end(),' '));
		String tmp(data.begin(),iter);

		{
		    ChangeLocale change_locale(LC_NUMERIC, "C");
			if(!tmp.empty())
				gx=stratof(tmp);
			else
				synfig::error("WorkArea::load_meta_data(): Unable to parse data for \"grid_size\", which was \"%s\"",data.c_str());

			if(iter==data.end())
				tmp.clear();
			else
				tmp=String(iter+1,data.end());

			if(!tmp.empty())
				gy=stratof(tmp);
			else
				synfig::error("WorkArea::load_meta_data(): Unable to parse data for \"grid_size\", which was \"%s\"",data.c_str());
		}

		set_grid_size(Vector(gx,gy));
	}

	data=canvas->get_meta_data("grid_color");
	if(!data.empty())
	{
		float gr(get_grid_color().get_r()),gg(get_grid_color().get_g()),gb(get_grid_color().get_b());

		String tmp;
		// Insert the string into a stream
		stringstream ss(data);
		// Create vector to hold our colors
		std::vector<String> tokens;

		int imaxcolor = 0;
		while (ss >> tmp && imaxcolor++ < 3)
			tokens.push_back(tmp);

		if (tokens.size() != 3 || imaxcolor > 3)
		{
			synfig::error("WorkArea::load_meta_data(): Unable to parse data for \"grid_color\", which was \"%s\". \"red green blue\" in [0,1] was expected",data.c_str());
			canvas_interface->get_ui_interface()->warning(_("Unable to set \"grid_color\""));
		}
		else
		{
		    ChangeLocale change_locale(LC_NUMERIC, "C");
			gr=atof(tokens.at(0).data());
			gg=atof(tokens.at(1).data());
			gb=atof(tokens.at(2).data());
		}

		set_grid_color(synfig::Color(gr,gg,gb));
	}

	data=canvas->get_meta_data("guide_color");
	if(!data.empty())
	{
		float gr(get_guides_color().get_r()),gg(get_guides_color().get_g()),gb(get_guides_color().get_b());

		String tmp;
		// Insert the string into a stream
		stringstream ss(data);
		// Create vector to hold our colors
		std::vector<String> tokens;

		int imaxcolor = 0;
		while (ss >> tmp && imaxcolor++ < 3)
			tokens.push_back(tmp);

		if (tokens.size() != 3 || imaxcolor > 3)
		{
			synfig::error("WorkArea::load_meta_data(): Unable to parse data for \"guide_color\", which was \"%s\". \"red green blue\" in [0,1] was expected",data.c_str());
			canvas_interface->get_ui_interface()->warning(_("Unable to set \"guide_color\""));
		}
		else
		{
		    ChangeLocale change_locale(LC_NUMERIC, "C");
			gr=atof(tokens.at(0).data());
			gg=atof(tokens.at(1).data());
			gb=atof(tokens.at(2).data());
		}

		set_guides_color(synfig::Color(gr,gg,gb));
	}

	data=canvas->get_meta_data("grid_show");
	if(data.size() && (data=="1" || data[0]=='t' || data[0]=='T'))
		show_grid=true;
	if(data.size() && (data=="0" || data[0]=='f' || data[0]=='F'))
		show_grid=false;

	data=canvas->get_meta_data("solid_lines");
	if(data.size() && (data=="1" || data[0]=='t' || data[0]=='T'))
		solid_lines=true;
	if(data.size() && (data=="0" || data[0]=='f' || data[0]=='F'))
		solid_lines=false;

	data=canvas->get_meta_data("guide_show");
	if(data.size() && (data=="1" || data[0]=='t' || data[0]=='T'))
		show_guides=true;
	if(data.size() && (data=="0" || data[0]=='f' || data[0]=='F'))
		show_guides=false;

	data=canvas->get_meta_data("grid_snap");
	if(data.size() && (data=="1" || data[0]=='t' || data[0]=='T'))
		set_grid_snap(true);
	if(data.size() && (data=="0" || data[0]=='f' || data[0]=='F'))
		set_grid_snap(false);

	data=canvas->get_meta_data("guide_snap");
	if(data.size() && (data=="1" || data[0]=='t' || data[0]=='T'))
		set_guide_snap(true);
	if(data.size() && (data=="0" || data[0]=='f' || data[0]=='F'))
		set_guide_snap(false);

	data=canvas->get_meta_data("onion_skin");
	if(data.size() && (data=="1" || data[0]=='t' || data[0]=='T'))
		set_onion_skin(true);
	if(data.size() && (data=="0" || data[0]=='f' || data[0]=='F'))
		set_onion_skin(false);

	bool render_required = false;
	data=canvas->get_meta_data("onion_skin_past");
	if(data.size())
	{
		int past_kf = stratoi(data);
		if (past_kf > ONION_SKIN_PAST) past_kf = ONION_SKIN_PAST;
		else if (past_kf < 0) past_kf =  0;

		if (past_kf != onion_skins[0])
		{
			onion_skins[0] = past_kf;
			render_required = true;
		}
	}
	data=canvas->get_meta_data("onion_skin_future");
	if(data.size())
	{
		int future_kf = stratoi(data);
		if (future_kf > ONION_SKIN_FUTURE) future_kf = ONION_SKIN_FUTURE;
		else if (future_kf < 0) future_kf =  0;

		if (future_kf != onion_skins[1])
		{
			onion_skins[1] = future_kf;
			render_required = true;
		}
	}
	// Update the canvas
	if (onion_skin && render_required) queue_render();

	data=canvas->get_meta_data("background_rendering");
	if(data.size() && (data=="1" || data[0]=='t' || data[0]=='T'))
		set_background_rendering(true);
	if(data.size() && (data=="0" || data[0]=='f' || data[0]=='F'))
		set_background_rendering(false);

	data=canvas->get_meta_data("guide_x");
	get_guide_list_x().clear();
	while(!data.empty())
	{
		String::iterator iter(find(data.begin(),data.end(),' '));
		String guide(data.begin(),iter);
	    ChangeLocale change_locale(LC_NUMERIC, "C");

		if(!guide.empty())
			get_guide_list_x().push_back(stratof(guide));

		if(iter==data.end())
			data.clear();
		else
			data=String(iter+1,data.end());
	}
	//sort(get_guide_list_x());

	data=canvas->get_meta_data("guide_y");
	get_guide_list_y().clear();
	while(!data.empty())
	{
		String::iterator iter(find(data.begin(),data.end(),' '));
		String guide(data.begin(),iter);
	    ChangeLocale change_locale(LC_NUMERIC, "C");

		if(!guide.empty())
			get_guide_list_y().push_back(stratof(guide));

		if(iter==data.end())
			data.clear();
		else
			data=String(iter+1,data.end());
	}
	//sort(get_guide_list_y());

	data = canvas->get_meta_data("jack_offset");
	if (!data.empty())
		jack_offset = stratof(data);

	data=canvas->get_meta_data("background_size");
	if(!data.empty())
	{
		float gx(get_background_size()[0]),gy(get_background_size()[1]);

		String::iterator iter(find(data.begin(),data.end(),' '));
		String tmp(data.begin(),iter);

		{
		    ChangeLocale change_locale(LC_NUMERIC, "C");
			if(!tmp.empty())
				gx=stratof(tmp);
			else
				synfig::error("WorkArea::load_meta_data(): Unable to parse data for \"background_size\", which was \"%s\"",data.c_str());

			if(iter==data.end())
				tmp.clear();
			else
				tmp=String(iter+1,data.end());

			if(!tmp.empty())
				gy=stratof(tmp);
			else
				synfig::error("WorkArea::load_meta_data(): Unable to parse data for \"background_size\", which was \"%s\"",data.c_str());
		}

		set_background_size(Vector(gx,gy));
	}

	data=canvas->get_meta_data("background_first_color");
	if(!data.empty())
	{
		float gr(get_background_first_color().get_r()),gg(get_background_first_color().get_g()),gb(get_background_first_color().get_b());

		String tmp;
		// Insert the string into a stream
		stringstream ss(data);
		// Create vector to hold our colors
		std::vector<String> tokens;

		int imaxcolor = 0;
		while (ss >> tmp && imaxcolor++ < 3)
			tokens.push_back(tmp);

		if (tokens.size() != 3 || imaxcolor > 3)
		{
			synfig::error("WorkArea::load_meta_data(): Unable to parse data for \"background_first_color\", which was \"%s\". \"red green blue\" in [0,1] was expected",data.c_str());
			canvas_interface->get_ui_interface()->warning(_("Unable to set \"background_first_color\""));
		}
		else
		{
		    ChangeLocale change_locale(LC_NUMERIC, "C");
			gr=atof(tokens.at(0).data());
			gg=atof(tokens.at(1).data());
			gb=atof(tokens.at(2).data());
		}

		set_background_first_color(synfig::Color(gr,gg,gb));
	}

	data=canvas->get_meta_data("background_second_color");
	if(!data.empty())
	{
		float gr(get_background_second_color().get_r()),gg(get_background_second_color().get_g()),gb(get_background_second_color().get_b());

		String tmp;
		// Insert the string into a stream
		stringstream ss(data);
		// Create vector to hold our colors
		std::vector<String> tokens;

		int imaxcolor = 0;
		while (ss >> tmp && imaxcolor++ < 3)
			tokens.push_back(tmp);

		if (tokens.size() != 3 || imaxcolor > 3)
		{
			synfig::error("WorkArea::load_meta_data(): Unable to parse data for \"background_second_color\", which was \"%s\". \"red green blue\" in [0,1] was expected",data.c_str());
			canvas_interface->get_ui_interface()->warning(_("Unable to set \"background_second_color\""));
		}
		else
		{
		    ChangeLocale change_locale(LC_NUMERIC, "C");
			gr=atof(tokens.at(0).data());
			gg=atof(tokens.at(1).data());
			gb=atof(tokens.at(2).data());
		}

		set_background_second_color(synfig::Color(gr,gg,gb));
	}

	meta_data_lock=false;
	queue_draw();
	signal_meta_data_changed()();
}

void
WorkArea::set_onion_skin(bool x)
{
	if (onion_skin == x)
		return;
	onion_skin = x;
	save_meta_data();
	queue_draw();
}

void WorkArea::set_onion_skins(int *onions)
{
	onion_skins[0] = onions[0];
	onion_skins[1] = onions[1];
	if (onion_skin)
		queue_draw();
	save_meta_data();
}

void
WorkArea::set_background_rendering(bool x)
{
	if (background_rendering == x)
		return;
	background_rendering = x;
	save_meta_data();
	queue_draw();
}

void
WorkArea::enable_grid()
{
	show_grid=true;
	save_meta_data();
	queue_draw();
}

void
WorkArea::disable_grid()
{
	show_grid=false;
	save_meta_data();
	queue_draw();
}

void
WorkArea::toggle_grid()
{
	show_grid=!show_grid;
	save_meta_data();
	queue_draw();
}

void
WorkArea::toggle_grid_snap()
{
	Duckmatic::toggle_grid_snap();
	save_meta_data();
	queue_draw();
}

void
WorkArea::set_show_guides(bool x)
{
	show_guides=x;
	save_meta_data();
	queue_draw();
}

void
WorkArea::toggle_guide_snap()
{
	Duckmatic::toggle_guide_snap();
	save_meta_data();
	queue_draw();
}

void
WorkArea::set_guides_color(const synfig::Color &c)
{
	Duckmatic::set_guides_color(c);
	save_meta_data();
	queue_draw();
}

void
WorkArea::set_jack_offset(const synfig::Time &x) {
	if (jack_offset == x) return;
	jack_offset = x;
	save_meta_data();
}

void
WorkArea::set_low_resolution_flag(bool x)
{
	if(x != low_resolution) {
		low_resolution=x;
		queue_render();
	}
}

void
WorkArea::toggle_low_resolution_flag()
{
	set_low_resolution_flag(!get_low_resolution_flag());
}

void
WorkArea::popup_menu()
{
	signal_popup_menu()();
}

void
WorkArea::set_grid_size(const synfig::Vector &s)
{
	Duckmatic::set_grid_size(s);
	save_meta_data();
	queue_draw();
}

void
WorkArea::set_grid_color(const synfig::Color &c)
{
	Duckmatic::set_grid_color(c);
	save_meta_data();
	queue_draw();
}

void
WorkArea::set_background_size(const synfig::Vector &s)
{
	if (background_size == s) return;
	background_size = s;
	background_pattern = Cairo::RefPtr<Cairo::SurfacePattern>();
	save_meta_data();
	queue_draw();
}

void
WorkArea::set_background_first_color(const synfig::Color &c)
{
	if (background_first_color == c) return;
	background_first_color = c;
	background_pattern = Cairo::RefPtr<Cairo::SurfacePattern>();;
	save_meta_data();
	queue_draw();
}

void
WorkArea::set_background_second_color(const synfig::Color &c)
{
	if (background_second_color == c) return;
	background_second_color = c;
	background_pattern = Cairo::RefPtr<Cairo::SurfacePattern>();
	save_meta_data();
	queue_draw();
}

const Cairo::RefPtr<Cairo::SurfacePattern>&
WorkArea::get_background_pattern() const
{
	if (!background_pattern) {
		int w = std::max(1, std::min(1000, (int)round(background_size[0])));
		int h = std::max(1, std::min(1000, (int)round(background_size[1])));
	    Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, w*2, h*2);
	    Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(surface);
	    context->set_source_rgb(background_first_color.get_r(), background_first_color.get_g(), background_first_color.get_b());
	    context->paint();
	    context->set_source_rgb(background_second_color.get_r(), background_second_color.get_g(), background_second_color.get_b());
	    context->rectangle(w, 0, w, h);
	    context->rectangle(0, h, w, h);
	    context->fill();
	    surface->flush();

	    background_pattern = Cairo::SurfacePattern::create(surface);
	    background_pattern->set_filter(Cairo::FILTER_NEAREST);
	    background_pattern->set_extend(Cairo::EXTEND_REPEAT);
	}
	return background_pattern;
}

void
WorkArea::set_focus_point(const synfig::Point &point)
{
	const synfig::Point& adjusted(point);

	synfig::RendDesc &rend_desc(get_canvas()->rend_desc());
	Real x_factor=(rend_desc.get_br()[0]-rend_desc.get_tl()[0]>0)?-1:1;
	Real y_factor=(rend_desc.get_br()[1]-rend_desc.get_tl()[1]>0)?-1:1;

	get_scrollx_adjustment()->set_value(adjusted[0]*x_factor);
	get_scrolly_adjustment()->set_value(adjusted[1]*y_factor);
}

synfig::Point
WorkArea::get_focus_point()const
{
	synfig::RendDesc &rend_desc(get_canvas()->rend_desc());
	Real x_factor=(rend_desc.get_br()[0]-rend_desc.get_tl()[0]>0)?-1:1;
	Real y_factor=(rend_desc.get_br()[1]-rend_desc.get_tl()[1]>0)?-1:1;
	return synfig::Point(get_scrollx_adjustment()->get_value()*x_factor, get_scrolly_adjustment()->get_value()*y_factor);
}

bool
WorkArea::on_key_press_event(GdkEventKey* event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	if (Smach::RESULT_OK == canvas_view->get_smach().process_event(
		EventKeyboard(EVENT_WORKAREA_KEY_DOWN, event->keyval, Gdk::ModifierType(event->state))))
			return true;

	if(get_selected_ducks().empty())
		return false;

	Real multiplier(1.0);

	if(Gdk::ModifierType(event->state)&GDK_SHIFT_MASK)
		multiplier=10.0;

	Vector nudge;
	switch(event->keyval)
	{
		case GDK_KEY_Left:
			nudge=Vector(-pw,0);
			break;
		case GDK_KEY_Right:
			nudge=Vector(pw,0);
			break;
		case GDK_KEY_Up:
			nudge=Vector(0,-ph);
			break;
		case GDK_KEY_Down:
			nudge=Vector(0,ph);
			break;
		default:
			return false;
			break;
	}

	synfigapp::Action::PassiveGrouper grouper(instance.get(),_("Nudge"));

	// Grid snap does not apply to nudging
	bool grid_snap_holder(get_grid_snap());
	bool guide_snap_holder(get_guide_snap());
	set_grid_snap(false);

	{
		LockDucks lock(get_canvas_view());
		start_duck_drag(get_selected_duck()->get_trans_point());
		translate_selected_ducks(get_selected_duck()->get_trans_point()+nudge*multiplier);
		end_duck_drag();
	}

	set_grid_snap(grid_snap_holder);
	set_guide_snap(guide_snap_holder);

	return true;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
WorkArea::on_key_release_event(GdkEventKey* event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	return Smach::RESULT_OK == canvas_view->get_smach().process_event(
		EventKeyboard(EVENT_WORKAREA_KEY_UP, event->keyval, Gdk::ModifierType(event->state)) );
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
WorkArea::on_drawing_area_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	synfig::Point mouse_pos;
    float bezier_click_pos(0);
	const float radius((abs(pw)+abs(ph))*4);
	int button_pressed(0);
	float pressure(0);
	Gdk::ModifierType modifier(Gdk::ModifierType(0));

	// Handle input stuff
	if (event->any.type==GDK_MOTION_NOTIFY)
	{
		GdkDevice *device = event->motion.device;
		modifier = Gdk::ModifierType(event->motion.state);

		// Calculate the position of the
		// input device in canvas coordinates

		/*std::string axes_str;
		int n_axes = gdk_device_get_n_axes(device);
		for (int i=0; i < n_axes; i++)
		{
			axes_str += etl::strprintf(" %f", event->motion.axes[i]);
		}
		synfig::warning("axes info: %s", axes_str.c_str());*/
		//for(...) axesstr += etl::strprintf(" %f", event->motion.axes[i])

		double x = 0.0, y = 0.0, p = 0.0;
		int ox = 0, oy = 0;
		#ifndef _WIN32
		Gtk::Container *toplevel = drawing_frame->get_toplevel();
		if (toplevel) drawing_frame->translate_coordinates(*toplevel, 0, 0, ox, oy);
		#endif

		if (gdk_device_get_axis(device, event->motion.axes, GDK_AXIS_X, &x))
			x -= ox; else x = event->motion.x;
		if (gdk_device_get_axis(device, event->motion.axes, GDK_AXIS_Y, &y))
			y -= oy; else y = event->motion.y;

		// Make sure we recognize the device
		if(curr_input_device)
		{
			if(curr_input_device!=device)
			{
				assert(device);
				curr_input_device=device;
				//synfig::error("device changed: %s", gdk_device_get_name(device));
				signal_input_device_changed()(curr_input_device);
			}
		}
		else
		if(device)
		{
			curr_input_device=device;
			//synfig::error("device set: %s", gdk_device_get_name(device));
			signal_input_device_changed()(curr_input_device);
		}

		assert(curr_input_device);


		//synfig::warning("coord (%3.f, %3.f) \t motion (%3.f, %3.f) / %s / axes(%d)", x, y, event->motion.x, event->motion.y, gdk_device_get_name(device), gdk_device_get_n_axes(device));
		if (gdk_device_get_axis(device, event->motion.axes, GDK_AXIS_PRESSURE, &p))
			p = std::max(0.0, (p - 0.04)/(1.0 - 0.04)); else p = 1.0;

		if(std::isnan(x) || std::isnan(y) || std::isnan(p))
			return false;

		mouse_pos=synfig::Point(screen_to_comp_coords(synfig::Point(x, y)));
		pressure = (float)p;
	}
	else
	if(	event->any.type==GDK_BUTTON_PRESS  ||
		event->any.type==GDK_2BUTTON_PRESS ||
		event->any.type==GDK_3BUTTON_PRESS ||
		event->any.type==GDK_BUTTON_RELEASE )
	{
		GdkDevice *device = event->button.device;
		modifier = Gdk::ModifierType(event->button.state);
		drawing_area->grab_focus();

		// Calculate the position of the
		// input device in canvas coordinates
		// and the buttons

		double x = 0.0, y = 0.0, p = 0.0;
		int ox = 0, oy = 0;
		#ifndef _WIN32
		Gtk::Container *toplevel = drawing_frame->get_toplevel();
		if (toplevel) drawing_frame->translate_coordinates(*toplevel, 0, 0, ox, oy);
		#endif

		if (gdk_device_get_axis(device, event->button.axes, GDK_AXIS_X, &x))
			x -= ox; else x = event->button.x;
		if (gdk_device_get_axis(device, event->button.axes, GDK_AXIS_Y, &y))
			y -= oy; else y = event->button.y;
		
		//synfig::warning("coord2 (%3.f, %3.f) \t motion (%3.f, %3.f) / %s / axes(%d)", x, y, event->button.x, event->button.y, gdk_device_get_name(device), gdk_device_get_n_axes(device));
		if (gdk_device_get_axis(device, event->button.axes, GDK_AXIS_PRESSURE, &p))
			p = std::max(0.0, (p - 0.04)/(1.0 - 0.04)); else p = 1.0;

		// Make sure we recognize the device
		if(curr_input_device)
		{
			if(curr_input_device!=device)
			{
				assert(device);
				curr_input_device=device;
				signal_input_device_changed()(curr_input_device);
			}
		}
		else
		if(device)
		{
			curr_input_device=device;
			signal_input_device_changed()(curr_input_device);
		}

		assert(curr_input_device);
			

		if(std::isnan(x) || std::isnan(y) || std::isnan(p))
			return false;

		mouse_pos=synfig::Point(screen_to_comp_coords(synfig::Point(x, y)));
		pressure = (float)p;
		button_pressed=event->button.button;
		if(button_pressed==1 && pressure<=0.f && (event->any.type!=GDK_BUTTON_RELEASE && event->any.type!=GDK_BUTTON_PRESS))
			button_pressed=0;
	}
	else
	// GDK mouse scrolling events
	if(event->any.type==GDK_SCROLL)
	{
		// GDK information needed to properly interpret mouse
		// scrolling events are: scroll.state, scroll.x/scroll.y, and
		// scroll.direction. The value of scroll.direction will be
		// obtained later.

		modifier=Gdk::ModifierType(event->scroll.state);
		mouse_pos=synfig::Point(screen_to_comp_coords(synfig::Point(event->scroll.x,event->scroll.y)));
	}

	// Handle the renderables
	{
		std::set<etl::handle<WorkAreaRenderer> >::iterator iter;
		for(iter=renderer_set_.begin();iter!=renderer_set_.end();++iter)
		{
			if((*iter)->get_enabled())
				if((*iter)->event_vfunc(event))
				{
					// Event handled. Return true.
					return true;
				}
		}
	}

	// Event hasn't been handled, pass it down
	switch(event->type) {
	case GDK_2BUTTON_PRESS: {
		if (event->button.button == 1) {
			if (canvas_view->get_smach().process_event(EventMouse(EVENT_WORKAREA_MOUSE_2BUTTON_DOWN,BUTTON_LEFT,mouse_pos,pressure,modifier))==Smach::RESULT_ACCEPT) {
				return true;
			}
		}
		break;
	}
	case GDK_BUTTON_PRESS: {
		switch(button_pressed) {
		case 1:	{ // Attempt to click on a duck
			etl::handle<Duck> duck;
			set_drag_mode(DRAG_NONE);

			if(allow_duck_clicks) {
				duck=find_duck(mouse_pos,radius);

				//!TODO Remove HARDCODE Ui Specification, make it config ready

				// Single click duck selection on WorkArea [Part I] (Part II lower in code)
				if (duck) {
					// make a note of whether the duck we click on was selected or not
					if(duck_is_selected(duck)) {
						clicked_duck=duck;
					} else {
						clicked_duck=0;
						// if CTRL or SHIFT isn't pressed, clicking an unselected duck will unselect all other ducks
						if(!(modifier&(GDK_CONTROL_MASK|GDK_SHIFT_MASK)))
							clear_selected_ducks();
						select_duck(duck);
					}
				}
			}
			//else
			//	clear_selected_ducks();

			if(allow_bezier_clicks)
				selected_bezier=find_bezier(mouse_pos,radius,&bezier_click_pos);
			else
				selected_bezier=0;

			if (duck) {
				if (!duck->get_editable(get_alternative_mode()))
					return true;

				//get_selected_duck()->signal_user_click(0)();
				//if(clicked_duck)clicked_duck->signal_user_click(0)();

				// if the user is holding shift while clicking on a tangent duck, consider splitting the tangent
				if ((event->button.state&GDK_SHIFT_MASK) && duck->get_type() == Duck::TYPE_TANGENT) {
					synfigapp::ValueDesc value_desc = duck->get_value_desc();

					// we have the tangent, but need the vertex - that's the parent
					if (value_desc.is_value_node()) {
						if (ValueNode_Composite::Handle value_node = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node())) {
							BLinePoint bp((*value_node)(get_time()).get(BLinePoint()));
							// if the tangent isn't split, then split it
							if (!bp.get_split_tangent_both()) {
								if (get_canvas_view()->canvas_interface()->change_value(synfigapp::ValueDesc(
										value_node,
										value_node->get_link_index_from_name("split_radius")),
										true)
								 && get_canvas_view()->canvas_interface()->change_value(synfigapp::ValueDesc(
										value_node,
										value_node->get_link_index_from_name("split_angle")),
										true ) )
								{
									// rebuild the ducks from scratch, so the tangents ducks aren't connected
									get_canvas_view()->rebuild_ducks();

									// reprocess the mouse click
									return on_drawing_area_event(event);
								}
								return true;
							}
						} else synfig::info("parent isn't composite value node?");
					} else {
						// I don't know how to access the vertex from the tangent duck when originally drawing the bline in the bline tool
						// synfig::ValueNode::Handle vn = value_desc.get_value_node();
						synfig::info("parent isn't value node?  shift-drag-tangent doesn't work in bline tool yet...");
					}
				}

				set_drag_mode(DRAG_DUCK);
				drag_point=mouse_pos;
				//drawing_area->queue_draw();
				start_duck_drag(mouse_pos);
				get_canvas_view()->reset_cancel_status();
				return true;
			} else
			if (canvas_view->get_smach().process_event(EventMouse(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,BUTTON_LEFT,mouse_pos,pressure,modifier))==Smach::RESULT_OK) {
				if (selected_bezier) {
					synfig::Point distance_1 = selected_bezier->p1->get_trans_point() - mouse_pos;
					synfig::Point distance_2 = selected_bezier->p2->get_trans_point() - mouse_pos;
					if ( distance_1.mag() > radius*2
					  && distance_2.mag() > radius*2 )
					{
						// If we click a selected bezier
						// not too close to the endpoints
						// We give the states first priority to process the
						// event so as not to interfere with the bline tool
						set_drag_mode(DRAG_BEZIER);
						drag_point=mouse_pos;
						start_bezier_drag(mouse_pos, bezier_click_pos);
						return true;
					}
				}
				// I commented out this section because
				// it was causing issues when rotoscoping.
				// At the moment, we don't need it, so
				// this was the easiest way to fix the problem.
				//else
				//if(selected_bezier)
				//{
				//	selected_duck=0;
				//	selected_bezier->signal_user_click(0)(bezier_click_pos);
				//}

				// Check for a guide click
				if (show_guides) {
					GuideList::iterator iter = find_guide_x(mouse_pos,radius);
					if (iter == get_guide_list_x().end()) {
						curr_guide_is_x = false;
						iter = find_guide_y(mouse_pos,radius);
					} else {
						curr_guide_is_x = true;
					}

					if (iter != get_guide_list_x().end() && iter != get_guide_list_y().end()) {
						set_drag_mode(DRAG_GUIDE);
						curr_guide = iter;
						return true;
					}
				}

				// All else fails, try making a selection box
				set_drag_mode(DRAG_BOX);
				curr_point = drag_point = mouse_pos;
				return true;
			}
			selected_bezier=0;
			break;
		}
		case 2:	{ // Attempt to drag and move the window
			etl::handle<Duck> duck = find_duck(mouse_pos, radius);
			etl::handle<Bezier> bezier = find_bezier(mouse_pos, radius, &bezier_click_pos);
			if (duck)
				duck->signal_user_click(1)();
			else
			if(bezier)
				bezier->signal_user_click(1)(bezier_click_pos);

			if (canvas_view->get_smach().process_event(EventMouse(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,BUTTON_MIDDLE,mouse_pos,pressure,modifier))==Smach::RESULT_OK) {
				set_drag_mode(
					(modifier & GDK_CONTROL_MASK) ? DRAG_ZOOM_WINDOW
				  : (modifier & GDK_SHIFT_MASK)   ? DRAG_ROTATE_WINDOW
				  : DRAG_WINDOW );
				drag_point = (modifier & GDK_CONTROL_MASK) ? synfig::Point(event->motion.x, event->motion.y) :
							mouse_pos;
				signal_user_click(1)(mouse_pos);
			}
			break;
		}
		case 3:	{ // Attempt to either get info on a duck, or open the menu
			if (etl::handle<Duck> duck = find_duck(mouse_pos, radius)) {
				if (get_selected_ducks().size() <= 1)
					duck->signal_user_click(2)();
				else
					canvas_view->get_smach().process_event(EventMouse(EVENT_WORKAREA_MULTIPLE_DUCKS_CLICKED,BUTTON_RIGHT,mouse_pos,pressure,modifier,duck));
				return true;
			}

			if (etl::handle<Bezier> bezier = find_bezier(mouse_pos, radius, &bezier_click_pos)) {
				bezier->signal_user_click(2)(bezier_click_pos);
				return true;
			}

			if (Layer::Handle layer = get_canvas()->find_layer(get_canvas_view()->get_context_params(), mouse_pos)) {
				if (canvas_view->get_smach().process_event(EventLayerClick(layer, BUTTON_RIGHT, mouse_pos)) == Smach::RESULT_OK)
					return false;
				return true;
			}

			canvas_view->get_smach().process_event(EventMouse(
				EVENT_WORKAREA_MOUSE_BUTTON_DOWN, BUTTON_RIGHT, mouse_pos, pressure, modifier ));
			//if(canvas_view->get_smach().process_event(EventMouse(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,BUTTON_RIGHT,mouse_pos,pressure,modifier))==Smach::RESULT_OK) {
			//	//popup_menu();
			//	return true;
			//}
			break;
		}
		case 4:
			signal_user_click(3)(mouse_pos);
			break;
		case 5:
			signal_user_click(4)(mouse_pos);
			break;
		default:
			break;
		}
		break;
	}
	case GDK_MOTION_NOTIFY: {
		if (event->motion.time - last_event_time < 25)
			return true;
		last_event_time = event->motion.time;
		curr_point = mouse_pos;

		signal_cursor_moved_();

		// Guide/Duck highlights on hover
		switch(get_drag_mode()) {
		case DRAG_NONE: {
            GuideList::iterator iter = find_guide_x(mouse_pos,radius);
            if (iter == get_guide_list_x().end())
                iter = find_guide_y(mouse_pos, radius);

            if (iter != curr_guide) {
                curr_guide = iter;
                drawing_area->queue_draw();
            }

            etl::handle<Duck> duck = find_duck(mouse_pos, radius);
            if (duck != hover_duck) {
                hover_duck = duck;
                drawing_area->queue_draw();
            }
    		break;
		}
		case DRAG_DUCK: {
			if (canvas_view->get_cancel_status()) {
				set_drag_mode(DRAG_NONE);
				canvas_view->queue_rebuild_ducks();
				return true;
			}

			//Point point((mouse_pos-selected_duck->get_origin())/selected_duck->get_scalar());
			//if(get_grid_snap()) {
			//	point[0]=floor(point[0]/grid_size[0]+0.5)*grid_size[0];
			//	point[1]=floor(point[1]/grid_size[1]+0.5)*grid_size[1];
			//}
			//selected_duck->set_point(point);

			set_axis_lock(event->motion.state & GDK_SHIFT_MASK);
			translate_selected_ducks(mouse_pos);
			drawing_area->queue_draw();
			break;
		}
		case DRAG_BEZIER: {
			if (canvas_view->get_cancel_status()) {
				set_drag_mode(DRAG_NONE);
				canvas_view->queue_rebuild_ducks();
				return true;
			}
			translate_selected_bezier(mouse_pos);
			drawing_area->queue_draw();
	        break;
		}
		case DRAG_BOX: {
			curr_point=mouse_pos;
			drawing_area->queue_draw();
	        break;
		}
		case DRAG_GUIDE: {
			if(curr_guide_is_x)
				*curr_guide = mouse_pos[0];
			else
				*curr_guide = mouse_pos[1];
			drawing_area->queue_draw();
	        break;
		}
		default: break;
		} // end switch dragging

		if (get_drag_mode() != DRAG_WINDOW) {
			// Update those triangle things on the rulers
			const synfig::Point point(mouse_pos);
			hruler->set_position( Distance(point[0], Distance::SYSTEM_UNITS).get(App::distance_system, get_canvas()->rend_desc()) );
			vruler->set_position( Distance(point[1], Distance::SYSTEM_UNITS).get(App::distance_system, get_canvas()->rend_desc()) );
		}

		if (get_drag_mode() == DRAG_WINDOW) {
			set_focus_point(get_focus_point() + mouse_pos-drag_point);
    	} 
		if (get_drag_mode() == DRAG_ZOOM_WINDOW) {
			set_zoom(get_zoom() * (1.0 + (drag_point[1] - event->motion.y) / 100.0));
			drag_point = synfig::Point(event->motion.x, event->motion.y);
		} else {
			MouseButton button = event->motion.state & GDK_BUTTON1_MASK ? BUTTON_LEFT
					           : event->motion.state & GDK_BUTTON2_MASK ? BUTTON_MIDDLE
					           : event->motion.state & GDK_BUTTON3_MASK ? BUTTON_RIGHT
					           : BUTTON_NONE;
			EventKey event = button == BUTTON_NONE ? EVENT_WORKAREA_MOUSE_MOTION : EVENT_WORKAREA_MOUSE_BUTTON_DRAG;
			Smach::event_result er = canvas_view->get_smach().process_event(
				EventMouse(event, button, mouse_pos, pressure, modifier) );
			if (er == Smach::RESULT_ACCEPT)
				return true;
		}

		break;
	}
	case GDK_BUTTON_RELEASE: {
		bool ret = false;
		switch (get_drag_mode()) {
		case DRAG_GUIDE: {
			double y(event->button.y), x(event->button.x);

			// Erase the guides if dragged into the rulers
			if(curr_guide_is_x && !std::isnan(x) && x<0.0 )
				get_guide_list_x().erase(curr_guide);
			else
			if(!curr_guide_is_x && !std::isnan(y) && y<0.0 )
				get_guide_list_y().erase(curr_guide);

			drawing_area->queue_draw();
			set_drag_mode(DRAG_NONE);
			save_meta_data();
			return true;
		}
		case DRAG_DUCK: {
			synfigapp::Action::PassiveGrouper grouper(instance.get(),_("Move"));

			LockDucks lock(get_canvas_view()); // don't rebuild ducks until end_duck_drag will called
			set_drag_mode(DRAG_NONE);

			//translate_selected_ducks(mouse_pos);
			set_axis_lock(false);

			if (!end_duck_drag()) {
				//!TODO Remove HARDCODED UI SPECIFICATION, make it config ready
				// Single click duck selection on WorkArea [Part II]
				// if we originally clicked on a selected duck ...
				if (clicked_duck) {
					// ... and CTRL is pressed, then just toggle the clicked duck
					//     or not SHIFT is pressed, make the clicked duck the
					//     only selected duck. (Nota : SHIFT just add to the selection)
					if (modifier & GDK_CONTROL_MASK) {
						unselect_duck(clicked_duck);
					} else
					if (!(modifier & GDK_SHIFT_MASK)) {
						clear_selected_ducks();
						select_duck(clicked_duck);
					}
					clicked_duck->signal_user_click(0)();
				}
			}

			//queue_draw();
			clicked_duck=0;
			ret = true;
			break;
		}
		case DRAG_BEZIER: {
			synfigapp::Action::PassiveGrouper grouper(instance.get(),_("Move"));

			LockDucks lock(get_canvas_view()); // don't rebuild ducks until end_duck_drag will called
			set_drag_mode(DRAG_NONE);

			//translate_selected_ducks(mouse_pos);
			set_axis_lock(false);

			if (!end_bezier_drag()) {
				// We didn't move the bezier, just clicked on it
				canvas_view->get_smach().process_event(EventMouse(
					EVENT_WORKAREA_MOUSE_BUTTON_DOWN, BUTTON_LEFT, mouse_pos, pressure, modifier ));
				canvas_view->get_smach().process_event(EventMouse(
					EVENT_WORKAREA_MOUSE_BUTTON_UP, BUTTON_LEFT, mouse_pos, pressure, modifier ));
			}

			//queue_draw();
			clicked_duck = 0;
			ret = true;
			break;
		}
		case DRAG_BOX: {
			set_drag_mode(DRAG_NONE);
			if((drag_point-mouse_pos).mag()>radius/2.0f)
			{
				if(canvas_view->get_smach().process_event(EventBox(drag_point,mouse_pos,MouseButton(event->button.button),modifier))==Smach::RESULT_ACCEPT)
					return true;
                /*
                 * Commented out because now the work is
                 * done in Renderer_Dragbox::event_vfunc
                 *

				// when dragging a box around some ducks:
				// SHIFT selects; CTRL toggles; SHIFT+CTRL unselects; <none> clears all then selects

				if(modifier&GDK_SHIFT_MASK)
					select_ducks_in_box(drag_point,mouse_pos);

				if(modifier&GDK_CONTROL_MASK)
					toggle_select_ducks_in_box(drag_point,mouse_pos);
				else if(!(modifier&GDK_SHIFT_MASK))
				{
					clear_selected_ducks();
					select_ducks_in_box(drag_point,mouse_pos);
				}
				*
				*/
				ret=true;
			}
			else
			{
				if(allow_layer_clicks)
				{
					Layer::Handle layer(get_canvas()->find_layer(get_canvas_view()->get_context_params(),drag_point));
					//if(layer)
					{
						if(canvas_view->get_smach().process_event(EventLayerClick(layer,BUTTON_LEFT,mouse_pos,modifier))==Smach::RESULT_OK)
							signal_layer_selected_(layer);
						ret=true;
					}
				}
				else
				{
					signal_user_click(0)(mouse_pos);
				}
			}
			drawing_area->queue_draw();
			break;
		}
		default: break;
		} //end switch dragging

		set_drag_mode(DRAG_NONE);

		Smach::event_result er = canvas_view->get_smach().process_event(
			EventMouse(
				EVENT_WORKAREA_MOUSE_BUTTON_UP,
				MouseButton(event->button.button),
				mouse_pos,
				pressure,
				modifier ));
		if (er == Smach::RESULT_ACCEPT)
			ret = true;
		return ret;
	}
	case GDK_SCROLL: {
		// Handle a mouse scrolling event like Xara Xtreme and
		// Inkscape:

	    //!TODO Remove HARDCODED UI SPECIFICATION, make it config ready

		// Scroll up/down: scroll up/down
		// Shift + scroll up/down: scroll left/right
		// Control + scroll up/down: zoom in/out

		if (modifier & GDK_CONTROL_MASK) {
			// The zoom is performed while preserving the pointer
			// position as a fixed point (similarly to Xara Xtreme and
			// Inkscape).

			// The strategy used below is to scroll to the updated
			// position, then zoom. This is easy to implement within
			// the present architecture, but has the disadvantage of
			// triggering multiple visible refreshes. Note: 1.25 is
			// the hard wired ratio in zoom_in()/zoom_out(). The
			// variable "drift" compensates additional inaccuracies in
			// the zoom. There is also an additional minus sign for
			// the inverted y coordinates.

			// FIXME: One might want to figure out where in the code
			// this empirical drift is been introduced.

			const synfig::Point scroll_point(get_scrollx_adjustment()->get_value(),get_scrolly_adjustment()->get_value());
			const double drift = 0.052;

			switch(event->scroll.direction) {
			case GDK_SCROLL_UP:
			case GDK_SCROLL_RIGHT:
				get_scrollx_adjustment()->set_value(scroll_point[0]+(mouse_pos[0]-scroll_point[0])*(1.25-(1+drift)));
				get_scrolly_adjustment()->set_value(scroll_point[1]-(mouse_pos[1]+scroll_point[1])*(1.25-(1+drift)));
				zoom_in();
				break;
			case GDK_SCROLL_DOWN:
			case GDK_SCROLL_LEFT:
				get_scrollx_adjustment()->set_value(scroll_point[0]+(mouse_pos[0]-scroll_point[0])*(1/1.25-(1+drift)));
				get_scrolly_adjustment()->set_value(scroll_point[1]-(mouse_pos[1]+scroll_point[1])*(1/1.25-(1+drift)));
				zoom_out();
				break;
			default:
				break;
			}
		} else
		if (modifier & GDK_SHIFT_MASK) {
			// Scroll in either direction by 20 pixels. Ideally, the
			// amount of pixels per scrolling event should be
			// configurable. Xara Xtreme currently uses an (hard
			// wired) amount 20 pixel, Inkscape defaults to 40 pixels.

			const int scroll_pixel = 20;

			switch(event->scroll.direction) {
			case GDK_SCROLL_UP:
				get_scrollx_adjustment()->set_value(get_scrollx_adjustment()->get_value()-scroll_pixel*pw);
				break;
			case GDK_SCROLL_DOWN:
				get_scrollx_adjustment()->set_value(get_scrollx_adjustment()->get_value()+scroll_pixel*pw);
				break;
			case GDK_SCROLL_LEFT:
				get_scrolly_adjustment()->set_value(get_scrolly_adjustment()->get_value()+scroll_pixel*ph);
				break;
			case GDK_SCROLL_RIGHT:
				get_scrolly_adjustment()->set_value(get_scrolly_adjustment()->get_value()-scroll_pixel*ph);
				break;
			default:
				break;
			}
		} else {
			// Scroll in either direction by 20 pixels. Ideally, the
			// amount of pixels per scrolling event should be
			// configurable. Xara Xtreme currently uses an (hard
			// wired) amount 20 pixel, Inkscape defaults to 40 pixels.

			const int scroll_pixel = 20;

			switch(event->scroll.direction) {
			case GDK_SCROLL_UP:
				get_scrolly_adjustment()->set_value(get_scrolly_adjustment()->get_value()+scroll_pixel*ph);
				break;
			case GDK_SCROLL_DOWN:
				get_scrolly_adjustment()->set_value(get_scrolly_adjustment()->get_value()-scroll_pixel*ph);
				break;
			case GDK_SCROLL_LEFT:
				get_scrollx_adjustment()->set_value(get_scrollx_adjustment()->get_value()-scroll_pixel*pw);
				break;
			case GDK_SCROLL_RIGHT:
				get_scrollx_adjustment()->set_value(get_scrollx_adjustment()->get_value()+scroll_pixel*pw);
				break;
			default:
				break;
			}
		}
		break;
	}
	default: break;
	}

	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
WorkArea::on_hruler_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	switch(event->type) {
	case GDK_BUTTON_PRESS:
		if (get_drag_mode() == DRAG_NONE && show_guides) {
			set_drag_mode(DRAG_GUIDE);
			curr_guide = get_guide_list_y().insert(get_guide_list_y().begin(), 0.0);
			curr_guide_is_x = false;
		}
		return true;
	case GDK_MOTION_NOTIFY:
		// Guide movement
		if (get_drag_mode() == DRAG_GUIDE && !curr_guide_is_x) {
			// Event is in the hruler, which has a slightly different
			// coordinate system from the canvas.
			event->motion.y -= hruler->get_height()+2;

			// call the on drawing area event to refresh everything.
			return on_drawing_area_event(event);
		}
		return true;
	case GDK_BUTTON_RELEASE:
		if (get_drag_mode() == DRAG_GUIDE && !curr_guide_is_x) {
			set_drag_mode(DRAG_NONE);
			save_meta_data();
			//get_guide_list_y().erase(curr_guide);
		}
		return true;
	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
WorkArea::on_vruler_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	switch(event->type) {
	case GDK_BUTTON_PRESS:
		if (get_drag_mode() == DRAG_NONE && show_guides) {
			set_drag_mode(DRAG_GUIDE);
			curr_guide=get_guide_list_x().insert(get_guide_list_x().begin(),0.0);
			curr_guide_is_x=true;
		}
		return true;
	case GDK_MOTION_NOTIFY:
		// Guide movement
		if (get_drag_mode() == DRAG_GUIDE && curr_guide_is_x) {
			// Event is in the vruler, which has a slightly different
			// coordinate system from the canvas.
			event->motion.x -= vruler->get_width()+2;

			// call the on drawing area event to refresh everything.
			return on_drawing_area_event(event);
		}
		return true;
	case GDK_BUTTON_RELEASE:
		if (get_drag_mode() == DRAG_GUIDE && curr_guide_is_x) {
			set_drag_mode(DRAG_NONE);
			save_meta_data();
			//get_guide_list_x().erase(curr_guide);
		}
		return true;
	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
WorkArea::on_duck_selection_single(const etl::handle<Duck>& duck)
{
	if (get_drag_mode() == DRAG_NONE) {
		studio::LayerTree* tree_layer(dynamic_cast<studio::LayerTree*>(canvas_view->get_ext_widget("layers_cmp")));
		tree_layer->select_param(duck->get_value_desc());
	}
}

void
WorkArea::refresh_dimension_info()
{
	const RendDesc &desc = get_canvas()->rend_desc();
	w = (int)(desc.get_w()*zoom);
	h = (int)(desc.get_h()*zoom);

	if (drawing_area->get_width()<=0 || drawing_area->get_height()<=0 || w<=0 || h<=0)
		return;

	thumb_w = THUMB_SIZE;
	thumb_h = (thumb_w*desc.get_h() + desc.get_w()/2)/desc.get_w(); // add desc.get_w()/2 for valid rounding

	synfig::RendDesc &rend_desc(get_canvas()->rend_desc());

	canvaswidth=rend_desc.get_br()[0]-rend_desc.get_tl()[0];
	canvasheight=rend_desc.get_br()[1]-rend_desc.get_tl()[1];

	pw=canvaswidth/w;
	ph=canvasheight/h;

	ConfigureAdjustment(scrollx_adjustment)
		.set_lower(-fabs(canvaswidth))
		.set_upper(fabs(canvaswidth))
		.set_step_increment(fabs(pw))
		.set_page_increment(fabs(get_grid_size()[0]))
		.finish();

	ConfigureAdjustment(scrolly_adjustment)
		.set_lower(-fabs(canvasheight))
		.set_upper(fabs(canvasheight))
		.set_step_increment(fabs(ph))
		.set_page_increment(fabs(get_grid_size()[1]))
		.finish();

	const synfig::Point focus_point(get_focus_point());
	const synfig::Real x(focus_point[0]/pw+drawing_area->get_width()/2-w/2);
	const synfig::Real y(focus_point[1]/ph+drawing_area->get_height()/2-h/2);

	window_tl[0]=rend_desc.get_tl()[0]-pw*x;
	window_br[0]=rend_desc.get_br()[0]+pw*(drawing_area->get_width()-x-w);

	window_tl[1]=rend_desc.get_tl()[1]-ph*y;
	window_br[1]=rend_desc.get_br()[1]+ph*(drawing_area->get_height()-y-h);

	hruler->set_min( Distance(window_tl[0],Distance::SYSTEM_UNITS).get(App::distance_system,rend_desc) );
	hruler->set_max( Distance(window_br[0],Distance::SYSTEM_UNITS).get(App::distance_system,rend_desc) );
	vruler->set_min( Distance(window_tl[1],Distance::SYSTEM_UNITS).get(App::distance_system,rend_desc) );
	vruler->set_max( Distance(window_br[1],Distance::SYSTEM_UNITS).get(App::distance_system,rend_desc) );

	view_window_changed();
}


synfig::Point
WorkArea::screen_to_comp_coords(synfig::Point pos)const
{
	synfig::RendDesc &rend_desc(get_canvas()->rend_desc());
	//synfig::Vector::value_type canvaswidth=rend_desc.get_br()[0]-rend_desc.get_tl()[0];
	//synfig::Vector::value_type canvasheight=rend_desc.get_br()[1]-rend_desc.get_tl()[1];
	//synfig::Vector::value_type pw=canvaswidth/w;
	//synfig::Vector::value_type ph=canvasheight/h;
	Vector focus_point=get_focus_point();
	synfig::Vector::value_type x=focus_point[0]/pw+drawing_area->get_width()/2-w/2;
	synfig::Vector::value_type y=focus_point[1]/ph+drawing_area->get_height()/2-h/2;

	return rend_desc.get_tl()-synfig::Point(pw*x,ph*y)+synfig::Point(pw*pos[0],ph*pos[1]);
}

synfig::Point
WorkArea::comp_to_screen_coords(synfig::Point /*pos*/)const
{
	synfig::warning("WorkArea::comp_to_screen_coords: Not yet implemented");
	return synfig::Point();
}

synfig::VectorInt
WorkArea::get_windows_offset() const
{
	const synfig::Vector focus_point(get_focus_point());

	// Calculate the window coordinates of the top-left
	// corner of the canvas.
	const synfig::Vector::value_type
		x(focus_point[0]/pw+drawing_area->get_width()/2-w/2),
		y(focus_point[1]/ph+drawing_area->get_height()/2-h/2);

	return VectorInt(int(x), int(y));
}

synfig::RectInt
WorkArea::get_window_rect() const
{
	VectorInt offset = get_windows_offset();

	RectInt rect(
		-offset[0],
		-offset[1],
		-offset[0] + drawing_area->get_width(),
		-offset[1] + drawing_area->get_height() );

	etl::set_intersect(rect, rect, RectInt(0, 0, w, h));
	return rect;
}

bool
WorkArea::refresh(const Cairo::RefPtr<Cairo::Context> &/*cr*/)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	assert(get_canvas());

	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> draw_area_window = drawing_area->get_window();
	if (!draw_area_window) return false;

	const Vector focus_point(get_focus_point());

	// Update the old focus point
	last_focus_point=focus_point;

	// Draw out the renderables
	{
		std::set<etl::handle<WorkAreaRenderer> >::iterator iter;
		for(iter=renderer_set_.begin();iter!=renderer_set_.end();++iter)
		{
			if((*iter)->get_enabled())
				(*iter)->render_vfunc(
					draw_area_window,
					Gdk::Rectangle(0, 0, draw_area_window->get_width(), draw_area_window->get_height())
				);
		}
	}

	// If we are in animate mode, draw a red border around the screen
	if(canvas_interface->get_mode()&synfigapp::MODE_ANIMATE)
	{
		// So let's do it in a more primitive fashion.
		Cairo::RefPtr<Cairo::Context> cr = draw_area_window->create_cairo_context();
		cr->save();

		cr->set_source_rgb(1,0,0);
		cr->set_line_cap(Cairo::LINE_CAP_BUTT);
		cr->set_line_join(Cairo::LINE_JOIN_MITER);
		cr->set_antialias(Cairo::ANTIALIAS_NONE);
		cr->set_line_width(10);

		cr->rectangle(0, 0, drawing_area->get_width(),drawing_area->get_height());
		cr->stroke();
		cr->restore();
	}

	return true;

	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

String
WorkArea::get_renderer() const
{
	if (get_low_resolution_flag())
	{
		String renderer = etl::strprintf("software-low%d", get_low_res_pixel_size());
		if (synfig::rendering::Renderer::get_renderers().count(renderer))
			return renderer;
	}
	return App::workarea_renderer;
}

void
WorkArea::set_low_res_pixel_size(int x)
{
	if(x==low_res_pixel_size)
		return;
	low_res_pixel_size=x;
	queue_render();
}

void
WorkArea::queue_scroll()
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> draw_area_window = drawing_area->get_window();
	if(!draw_area_window) return;

//	const synfig::RendDesc &rend_desc(get_canvas()->rend_desc());

	const synfig::Point focus_point(get_focus_point());

	const synfig::Real
		new_x(focus_point[0]/pw+drawing_area->get_width()/2-w/2),
		new_y(focus_point[1]/ph+drawing_area->get_height()/2-h/2);

	const synfig::Real
		old_x(last_focus_point[0]/pw+drawing_area->get_width()/2-w/2),
		old_y(last_focus_point[1]/ph+drawing_area->get_height()/2-h/2);

	// If the coordinates didn't change, we shouldn't queue a draw
	if(old_x==new_x && old_y==new_y)
		return;

	const int
		dx(round_to_int(old_x)-round_to_int(new_x)),
		dy(round_to_int(old_y)-round_to_int(new_y));

	draw_area_window->scroll(-dx,-dy);

	if (timecode_width && timecode_height)
	{
		drawing_area->queue_draw_area(timecode_x,    timecode_y,    timecode_x+timecode_width,    timecode_y+timecode_height);
		drawing_area->queue_draw_area(timecode_x-dx, timecode_y-dy, timecode_x-dx+timecode_width, timecode_y-dy+timecode_height);
	}

	if (bonesetup_width && bonesetup_height)
	{
		drawing_area->queue_draw_area(bonesetup_x,    bonesetup_y,    bonesetup_x+bonesetup_width,    bonesetup_y+bonesetup_height);
		drawing_area->queue_draw_area(bonesetup_x-dx, bonesetup_y-dy, bonesetup_x-dx+bonesetup_width, bonesetup_y-dy+bonesetup_height);
	}

	if(canvas_interface->get_mode()&synfigapp::MODE_ANIMATE)
	{
		int maxx = drawing_area->get_width()-1;
		int maxy = drawing_area->get_height()-1;

		if (dx > 0)
		{
			drawing_area->queue_draw_area(      0, 0,       1, maxy);
			drawing_area->queue_draw_area(maxx-dx, 0, maxx-dx, maxy);
		}
		else if (dx < 0)
		{
			drawing_area->queue_draw_area(   maxx, 0,    maxx, maxy);
			drawing_area->queue_draw_area(    -dx, 0,     -dx, maxy);
		}
		if (dy > 0)
		{
			drawing_area->queue_draw_area(0,       0, maxx,       1);
			drawing_area->queue_draw_area(0, maxy-dy, maxx, maxy-dy);
		}
		else if (dy < 0)
		{
			drawing_area->queue_draw_area(0,    maxy, maxx,    maxy);
			drawing_area->queue_draw_area(0,     -dy, maxx,     -dy);
		}
	}

	last_focus_point=focus_point;
}

void
studio::WorkArea::zoom_in()
{
	set_zoom(zoom*1.25);
}

void
studio::WorkArea::zoom_out()
{
	set_zoom(zoom/1.25);
}

void
studio::WorkArea::zoom_fit()
{
	float new_zoom(min(drawing_area->get_width() * zoom / w,
					   drawing_area->get_height() * zoom / h) * 0.995);
	if (zoom / new_zoom > 0.995 && new_zoom / zoom > 0.995)
	{
		set_zoom(prev_zoom);
		return set_focus_point(previous_focus);
	}
	previous_focus = get_focus_point();
	prev_zoom = zoom;
	set_zoom(new_zoom);
	set_focus_point(Point(0,0));
}

void
studio::WorkArea::zoom_norm()
{
	if (zoom == 1.0) return set_zoom(prev_zoom);
	prev_zoom = zoom;
	set_zoom(1.0f);
}

void
studio::WorkArea::zoom_edit()
{
	set_zoom(zoomdial->get_zoom(zoom));
}

void
WorkArea::sync_render(bool refresh)
{
	dirty_trap_queued = 0;
	if (refresh) renderer_canvas->clear_render();
	renderer_canvas->enqueue_render();
	renderer_canvas->wait_render();
}

void
studio::WorkArea::queue_render(bool refresh)
{
	assert(dirty_trap_count >= 0);
	if (dirty_trap_count > 0)
		{ dirty_trap_queued++; return; }
	dirty_trap_queued = 0;
	// avoiding dead-lock : github#1071
	Glib::signal_idle().connect_once([=] () {
		if (refresh) {
			renderer_canvas->clear_render();
			Glib::signal_idle().connect_once(
						sigc::mem_fun(*renderer_canvas, &Renderer_Canvas::enqueue_render),
						Glib::PRIORITY_DEFAULT );
		} else {
			renderer_canvas->enqueue_render();
		}
	});
}

void
studio::WorkArea::set_cursor(const Glib::RefPtr<Gdk::Cursor> &x)
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> draw_area_window = drawing_area->get_window();
	if (!draw_area_window) return;
	draw_area_window->set_cursor(x);
}
void
studio::WorkArea::set_cursor(Gdk::CursorType x)
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> draw_area_window = drawing_area->get_window();
	if(!draw_area_window) return;
	draw_area_window->set_cursor(Gdk::Cursor::create(x));
}

void
studio::WorkArea::refresh_cursor()
{
//	set_cursor(IconController::get_tool_cursor(canvas_view->get_smach().get_state_name(),drawing_area->get_window()));
}

void
studio::WorkArea::reset_cursor()
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> draw_area_window = drawing_area->get_window();
	if(!draw_area_window) return;

	draw_area_window->set_cursor(Gdk::Cursor::create(Gdk::TOP_LEFT_ARROW));
//	set_cursor(Gdk::TOP_LEFT_ARROW);
}

void
studio::WorkArea::set_zoom(float z)
{
	z=max(1.0f/128.0f,min(128.0f,z));
	zoomdial->set_zoom(z);
	if(z==zoom)
		return;
	zoom = z;

	refresh_dimension_info();
	queue_draw();

	// TODO: FIXME: QuickHack
	if (canvas_view->get_smach().get_state_name() != std::string("polygon")
	 && canvas_view->get_smach().get_state_name() != std::string("bline"))
			canvas_view->queue_rebuild_ducks();
}

void
WorkArea::set_selected_value_node(etl::loose_handle<synfig::ValueNode> x)
{
	if(x!=selected_value_node_)
	{
		selected_value_node_=x;
		queue_draw();
	}
}

void
WorkArea::set_active_bone_value_node(etl::loose_handle<synfig::ValueNode> x)
{
	if(x!=active_bone_ && etl::handle<synfig::ValueNode_Bone>::cast_dynamic(x))
	{
		active_bone_=x;
		queue_draw();
	}
}

void
WorkArea::insert_renderer(const etl::handle<WorkAreaRenderer> &x)
{
	renderer_set_.insert(x);
	x->set_work_area(this);
	queue_draw();
}

void
WorkArea::insert_renderer(const etl::handle<WorkAreaRenderer> &x, int priority)
{
	x->set_priority(priority);
	insert_renderer(x);
}

void
WorkArea::erase_renderer(const etl::handle<WorkAreaRenderer> &x)
{
	x->set_work_area(0);
	renderer_set_.erase(x);
	// queue_draw();
	// because erase_renderer is called only form destructor, we do not need to
	// queue draw widget update (GTK error is produced)
}

void
WorkArea::resort_render_set()
{
	std::set<etl::handle<WorkAreaRenderer> > tmp(
		renderer_set_.begin(),
		renderer_set_.end()
	);
	renderer_set_.swap(tmp);
	queue_draw();
}
