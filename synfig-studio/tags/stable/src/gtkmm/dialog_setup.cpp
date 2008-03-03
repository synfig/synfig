/* === S Y N F I G ========================================================= */
/*!	\file dialog_setup.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "dialog_setup.h"
#include "app.h"
#include <gtkmm/scale.h>
#include <gtkmm/table.h>
#include <gtkmm/frame.h>
#include <gtkmm/notebook.h>
#include <gtkmm/spinbutton.h>
#include "widget_enum.h"
#include "autorecover.h"

#include <ETL/stringf>
#include <ETL/misc>
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static void
attach_label(Gtk::Table *table, String str, guint col, guint xpadding, guint ypadding)
{
	Gtk::Label* label(manage(new Gtk::Label((str + ":").c_str())));
	label->set_alignment(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER);
	table->attach(*label, 0, 1, col, col+1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);
}

Dialog_Setup::Dialog_Setup():
	Dialog(_("Synfig Studio Setup"),false,true),
	adj_gamma_r(2.2,0.1,3.0,0.025,0.025,0.025),
	adj_gamma_g(2.2,0.1,3.0,0.025,0.025,0.025),
	adj_gamma_b(2.2,0.1,3.0,0.025,0.025,0.025),
	adj_recent_files(15,1,50,1,1,1),
	adj_undo_depth(100,10,5000,1,1,1),
	toggle_use_colorspace_gamma(_("Visually Linear Color Selection")),
#ifdef SINGLE_THREADED
	toggle_single_threaded(_("Use Only a Single Thread")),
#endif
	toggle_restrict_radius_ducks(_("Restrict Real-Valued Ducks to Top Right Quadrant"))
{
	// Setup the buttons

	Gtk::Button *ok_button(manage(new class Gtk::Button(Gtk::StockID("gtk-ok"))));
	ok_button->show();
	add_action_widget(*ok_button,2);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Setup::on_ok_pressed));

	Gtk::Button *apply_button(manage(new class Gtk::Button(Gtk::StockID("gtk-apply"))));
	apply_button->show();
	add_action_widget(*apply_button,1);
	apply_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Setup::on_apply_pressed));

	Gtk::Button *cancel_button(manage(new class Gtk::Button(Gtk::StockID("gtk-close"))));
	cancel_button->show();
	add_action_widget(*cancel_button,0);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Setup::hide));


	// Notebook
	Gtk::Notebook *notebook=manage(new class Gtk::Notebook());
	get_vbox()->pack_start(*notebook);


	// Gamma
	Gtk::Table *gamma_table=manage(new Gtk::Table(2,2,false));
	notebook->append_page(*gamma_table,_("Gamma"));
	//gamma_frame->add(*gamma_table);

	gamma_table->attach(gamma_pattern, 0, 2, 0, 1, Gtk::EXPAND, Gtk::SHRINK|Gtk::FILL, 0, 0);

	Gtk::HScale* scale_gamma_r(manage(new Gtk::HScale(adj_gamma_r)));
	gamma_table->attach(*manage(new Gtk::Label(_("Red"))), 0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	gamma_table->attach(*scale_gamma_r, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	adj_gamma_r.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_r_change));

	Gtk::HScale* scale_gamma_g(manage(new Gtk::HScale(adj_gamma_g)));
	gamma_table->attach(*manage(new Gtk::Label(_("Green"))), 0, 1, 2, 3, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	gamma_table->attach(*scale_gamma_g, 1, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	adj_gamma_g.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_g_change));

	Gtk::HScale* scale_gamma_b(manage(new Gtk::HScale(adj_gamma_b)));
	gamma_table->attach(*manage(new Gtk::Label(_("Blue"))), 0, 1, 3, 4, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	gamma_table->attach(*scale_gamma_b, 1, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	adj_gamma_b.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_gamma_b_change));

	gamma_table->attach(*manage(new Gtk::Label(_("Black Level"))), 0, 1, 4, 5, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	gamma_table->attach(black_level_selector, 1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	black_level_selector.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_black_level_change));

	//gamma_table->attach(*manage(new Gtk::Label(_("Red-Blue Level"))), 0, 1, 5, 6, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	//gamma_table->attach(red_blue_level_selector, 1, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);
	//red_blue_level_selector.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Dialog_Setup::on_red_blue_level_change));


	// Misc
	Gtk::Table *misc_table=manage(new Gtk::Table(2,2,false));
	notebook->append_page(*misc_table,_("Misc."));

	int xpadding(8), ypadding(8);

	// Misc - Timestamp
	timestamp_menu=manage(new class Gtk::Menu());
	attach_label(misc_table, _("Timestamp"), 0, xpadding, ypadding);
	misc_table->attach(timestamp_optionmenu, 1, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);

#define ADD_TIMESTAMP(desc,x)									\
	timestamp_menu->items().push_back(							\
		Gtk::Menu_Helpers::MenuElem(							\
			desc,												\
			sigc::bind(											\
				sigc::mem_fun(									\
					*this,										\
					&studio::Dialog_Setup::set_time_format),	\
				x)));
	ADD_TIMESTAMP("HH:MM:SS.FF",		Time::FORMAT_VIDEO	);
	ADD_TIMESTAMP("(HHh MMm SSs) FFf",	Time::FORMAT_NORMAL	);
	ADD_TIMESTAMP("(HHhMMmSSs)FFf",		Time::FORMAT_NORMAL	| Time::FORMAT_NOSPACES	);
	ADD_TIMESTAMP("HHh MMm SSs FFf",	Time::FORMAT_NORMAL	| Time::FORMAT_FULL		);
	ADD_TIMESTAMP("HHhMMmSSsFFf",		Time::FORMAT_NORMAL	| Time::FORMAT_NOSPACES	| Time::FORMAT_FULL);
	ADD_TIMESTAMP("FFf",				Time::FORMAT_FRAMES );

	timestamp_optionmenu.set_menu(*timestamp_menu);

#undef ADD_TIMESTAMP

	{
		ParamDesc param_desc;
		param_desc
			.set_hint("enum")
			.add_enum_value(Distance::SYSTEM_UNITS,"u",_("Units"))
			.add_enum_value(Distance::SYSTEM_PIXELS,"px",_("Pixels"))
			.add_enum_value(Distance::SYSTEM_POINTS,"pt",_("Points"))
			.add_enum_value(Distance::SYSTEM_INCHES,"in",_("Inches"))
			.add_enum_value(Distance::SYSTEM_METERS,"m",_("Meters"))
			.add_enum_value(Distance::SYSTEM_CENTIMETERS,"cm",_("Centimeters"))
			.add_enum_value(Distance::SYSTEM_MILLIMETERS,"mm",_("Millimeters"));

		widget_enum=manage(new Widget_Enum());
		widget_enum->set_param_desc(param_desc);

		attach_label(misc_table, _("Unit System"), 1, xpadding, ypadding);
		misc_table->attach(*widget_enum, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);
	}

	// Misc - recent files
	Gtk::SpinButton* recent_files_spinbutton(manage(new Gtk::SpinButton(adj_recent_files,1,0)));
	attach_label(misc_table, _("Recent Files"), 2, xpadding, ypadding);
	misc_table->attach(*recent_files_spinbutton, 1, 2, 2, 3, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);

	// Misc - use_colorspace_gamma
	misc_table->attach(toggle_use_colorspace_gamma, 0, 2, 5, 6, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);

#ifdef SINGLE_THREADED
	// Misc - single_threaded
	misc_table->attach(toggle_single_threaded, 0, 2, 7, 8, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);
#endif

	// Misc - auto backup interval
	attach_label(misc_table, _("Auto Backup Interval (0 to disable)"), 3, xpadding, ypadding);
	misc_table->attach(auto_backup_interval, 1, 2, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);

	// Misc - restrict_radius_ducks
	misc_table->attach(toggle_restrict_radius_ducks, 0, 2, 6, 7, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);

	// Misc - browser_command
	attach_label(misc_table, _("Browser Command"), 4, xpadding, ypadding);
	misc_table->attach(textbox_browser_command, 1, 2, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, xpadding, ypadding);

	show_all_children();
}

Dialog_Setup::~Dialog_Setup()
{
}

void
Dialog_Setup::on_ok_pressed()
{
    on_apply_pressed();
	hide();
}

void
Dialog_Setup::on_apply_pressed()
{
	App::gamma.set_all(1.0/adj_gamma_r.get_value(),1.0/adj_gamma_g.get_value(),1.0/adj_gamma_b.get_value(),black_level_selector.get_value(),red_blue_level_selector.get_value());

	App::set_max_recent_files((int)adj_recent_files.get_value());

	// Set the time format
	App::set_time_format(get_time_format());

	// Set the use_colorspace_gamma flag
	App::use_colorspace_gamma=toggle_use_colorspace_gamma.get_active();

#ifdef SINGLE_THREADED
	// Set the single_threaded flag
	App::single_threaded=toggle_single_threaded.get_active();
#endif

	// Set the auto backup interval
	App::auto_recover->set_timeout(auto_backup_interval.get_value() * 1000);

	App::distance_system=Distance::System(widget_enum->get_value());

	// Set the restrict_radius_ducks flag
	App::restrict_radius_ducks=toggle_restrict_radius_ducks.get_active();

	// Set the browser_command textbox
	App::browser_command=textbox_browser_command.get_text();

	App::save_settings();
}

void
Dialog_Setup::on_gamma_r_change()
{
	gamma_pattern.set_gamma_r(1.0/adj_gamma_r.get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_gamma_g_change()
{
	gamma_pattern.set_gamma_g(1.0/adj_gamma_g.get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_gamma_b_change()
{
	gamma_pattern.set_gamma_b(1.0/adj_gamma_b.get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_black_level_change()
{
	gamma_pattern.set_black_level(black_level_selector.get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}

void
Dialog_Setup::on_red_blue_level_change()
{
	gamma_pattern.set_red_blue_level(red_blue_level_selector.get_value());
	gamma_pattern.refresh();
	gamma_pattern.queue_draw();
}


void
Dialog_Setup::refresh()
{
	// Refresh the temporary gamma; do this before adjusting the sliders,
	// or variables will be used before their initialization.
	gamma_pattern.set_gamma_r(App::gamma.get_gamma_r());
	gamma_pattern.set_gamma_g(App::gamma.get_gamma_g());
	gamma_pattern.set_gamma_b(App::gamma.get_gamma_b());
	gamma_pattern.set_black_level(App::gamma.get_black_level());
	gamma_pattern.set_red_blue_level(App::gamma.get_red_blue_level());

	adj_gamma_r.set_value(1.0/App::gamma.get_gamma_r());
	adj_gamma_g.set_value(1.0/App::gamma.get_gamma_g());
	adj_gamma_b.set_value(1.0/App::gamma.get_gamma_b());
	black_level_selector.set_value(App::gamma.get_black_level());
	red_blue_level_selector.set_value(App::gamma.get_red_blue_level());

	gamma_pattern.refresh();

	adj_recent_files.set_value(App::get_max_recent_files());

	// Refresh the time format
	set_time_format(App::get_time_format());

	widget_enum->set_value(App::distance_system);

	// Refresh the status of the use_colorspace_gamma flag
	toggle_use_colorspace_gamma.set_active(App::use_colorspace_gamma);

#ifdef SINGLE_THREADED
	// Refresh the status of the single_threaded flag
	toggle_single_threaded.set_active(App::single_threaded);
#endif

	// Refresh the value of the auto backup interval
	auto_backup_interval.set_value(App::auto_recover->get_timeout() / 1000);

	// Refresh the status of the restrict_radius_ducks flag
	toggle_restrict_radius_ducks.set_active(App::restrict_radius_ducks);

	// Refresh the browser_command textbox
	textbox_browser_command.set_text(App::browser_command);
}

GammaPattern::GammaPattern():
	tile_w(80),
	tile_h(80)
{
	set_size_request(tile_w*4,tile_h*3);
	signal_expose_event().connect(sigc::mem_fun(*this, &studio::GammaPattern::redraw));
}

GammaPattern::~GammaPattern()
{
}

void
GammaPattern::refresh()
{
	black[0].set_rgb_p(
		r_F32_to_F32(0.0),
		g_F32_to_F32(0.0),
		b_F32_to_F32(0.0)
	);
	white[0].set_rgb_p(
		r_F32_to_F32(1.0),
		g_F32_to_F32(1.0),
		b_F32_to_F32(1.0)
	);
	gray50[0].set_rgb_p(
		r_F32_to_F32(0.5),
		g_F32_to_F32(0.5),
		b_F32_to_F32(0.5)
	);
	gray25[0].set_rgb_p(
		r_F32_to_F32(0.25),
		g_F32_to_F32(0.25),
		b_F32_to_F32(0.25)
	);

	// Reds
	black[1].set_rgb(black[0].get_red(),0,0);
	gray25[1].set_rgb(gray25[0].get_red(),0,0);
	gray50[1].set_rgb(gray50[0].get_red(),0,0);
	white[1].set_rgb(white[0].get_red(),0,0);

	// Greens
	black[2].set_rgb(0,black[0].get_green(),0);
	gray25[2].set_rgb(0,gray25[0].get_green(),0);
	gray50[2].set_rgb(0,gray50[0].get_green(),0);
	white[2].set_rgb(0,white[0].get_green(),0);

	// blues
	black[3].set_rgb(0,0,black[0].get_blue());
	gray25[3].set_rgb(0,0,gray25[0].get_blue());
	gray50[3].set_rgb(0,0,gray50[0].get_blue());
	white[3].set_rgb(0,0,white[0].get_blue());
}

bool
GammaPattern::redraw(GdkEventExpose */*bleh*/)
{
	static const char hlines[] = { 3, 0 };

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(get_window()));

	int i;
	Gdk::Color trueblack("#000000");

	// 50% Pattern
	for(i=0;i<4;i++)
	{
		gc->set_rgb_fg_color(black[i]);
		get_window()->draw_rectangle(gc, true, i*tile_w, 0, tile_w, tile_h);

		gc->set_stipple(Gdk::Bitmap::create(hlines,2,2));
		gc->set_fill(Gdk::STIPPLED);
		gc->set_rgb_fg_color(white[i]);
		get_window()->draw_rectangle(gc, true, i*tile_w, 0, tile_w, tile_h);

		gc->set_fill(Gdk::SOLID);
		gc->set_rgb_fg_color(gray50[i]);

		get_window()->draw_rectangle(gc, true, i*tile_w+tile_w/4, tile_h/4, tile_w-tile_w/2, tile_h-tile_h/2);
	}

	// 25% Pattern
	for(i=0;i<4;i++)
	{
		gc->set_rgb_fg_color(black[i]);
		get_window()->draw_rectangle(gc, true, i*tile_w, tile_h, tile_w, tile_h);

		gc->set_stipple(Gdk::Bitmap::create(hlines,2,2));
		gc->set_fill(Gdk::STIPPLED);
		gc->set_rgb_fg_color(gray50[i]);
		get_window()->draw_rectangle(gc, true, i*tile_w, tile_h, tile_w, tile_h);

		gc->set_fill(Gdk::SOLID);
		gc->set_rgb_fg_color(gray25[i]);

		get_window()->draw_rectangle(gc, true, i*tile_w+tile_w/4, tile_h+tile_h/4, tile_w-tile_w/2, tile_h-tile_h/2);
	}

	// Black-level Pattern
	gc->set_rgb_fg_color(trueblack);
	get_window()->draw_rectangle(gc, true, 0, tile_h*2, tile_w*4, tile_h);
	gc->set_fill(Gdk::SOLID);
	for(i=0;i<4;i++)
	{
		gc->set_rgb_fg_color(black[i]);

		get_window()->draw_rectangle(gc, true, i*tile_w+tile_w/4, tile_h*2+tile_h/4, tile_w-tile_w/2, tile_h-tile_h/2);
	}

	return true;
}


BlackLevelSelector::BlackLevelSelector()
{
	set_size_request(-1,24);
	signal_expose_event().connect(sigc::mem_fun(*this, &studio::BlackLevelSelector::redraw));

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
}

BlackLevelSelector::~BlackLevelSelector()
{
}

bool
BlackLevelSelector::redraw(GdkEventExpose */*bleh*/)
{
	const int w(get_width()),h(get_height());

	Gdk::Color color;

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(get_window()));

	int i;

	// Draw the gradient
	for(i=0;i<w;i++)
	{
		color.set_rgb(i*65536/w,i*65536/w,i*65536/w);

		gc->set_rgb_fg_color(color);
		get_window()->draw_rectangle(gc, true, i, 0, 1, h);
	}

	// Draw a frame
	gc->set_rgb_fg_color(Gdk::Color("#000000"));
	get_window()->draw_rectangle(gc, false, 0, 0, w-1, h-1);

	// Draw the position of the current value
	i=(int)(level*w+0.5);
	gc->set_rgb_fg_color(Gdk::Color("#ff0000"));
	get_window()->draw_rectangle(gc, true, i, 1, 1, h-1);

	// Print out the value
	Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
	layout->set_text(etl::strprintf("%0.01f%%",level*100.0f));
	layout->set_alignment(Pango::ALIGN_CENTER);
	gc->set_rgb_fg_color(Gdk::Color("#a00000"));
	get_window()->draw_layout(gc, w/2, 4, layout);

	return true;
}



bool
BlackLevelSelector::on_event(GdkEvent *event)
{
	int x(round_to_int(event->button.x));
	//int y(round_to_int(event->button.y));

    switch(event->type)
    {
	case GDK_MOTION_NOTIFY:
		level=(float)x/(float)get_width();
		if(level<0.0f)level=0.0f;
		if(level>1.0f)level=1.0f;
		signal_value_changed_();
		queue_draw();
		return true;
		break;
	case GDK_BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		if(event->button.button==1)
		{
			level=(float)x/(float)get_width();
			if(level<0.0f)level=0.0f;
			if(level>1.0f)level=1.0f;
			signal_value_changed_();
			queue_draw();
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}


void
Dialog_Setup::set_time_format(synfig::Time::Format x)
{
	time_format=x;
	if (x <= Time::FORMAT_VIDEO)
		timestamp_optionmenu.set_history(0);
	else if (x == (Time::FORMAT_NORMAL))
		timestamp_optionmenu.set_history(1);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_NOSPACES))
		timestamp_optionmenu.set_history(2);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_FULL))
		timestamp_optionmenu.set_history(3);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_NOSPACES | Time::FORMAT_FULL))
		timestamp_optionmenu.set_history(4);
	else if (x == (Time::FORMAT_FRAMES))
		timestamp_optionmenu.set_history(5);
	else
		timestamp_optionmenu.set_history(1);
}
















RedBlueLevelSelector::RedBlueLevelSelector()
{
	set_size_request(-1,24);
	signal_expose_event().connect(sigc::mem_fun(*this, &studio::RedBlueLevelSelector::redraw));

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
}

RedBlueLevelSelector::~RedBlueLevelSelector()
{
}

bool
RedBlueLevelSelector::redraw(GdkEventExpose */*bleh*/)
{
	const int w(get_width()),h(get_height());

	Gdk::Color color;

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(get_window()));

	int i;

	// Draw the gradient
	for(i=0;i<w;i++)
	{
		float red_blue(((float(i)/float(w)+0.5f)-1.0f)/2.0f+1.0f);
		float blue_red(2.0f-(red_blue));
		if(red_blue>1.0f)red_blue=1.0f;
		if(blue_red>1.0f)blue_red=1.0f;

		color.set_rgb(
			round_to_int(min(red_blue,1.0f)*65535),
			round_to_int(sqrt(min(red_blue,blue_red))*65535),
			round_to_int(min(blue_red,1.0f)*65535)
		);

		gc->set_rgb_fg_color(color);
		get_window()->draw_rectangle(gc, true, i, 0, 1, h);
	}

	// Draw a frame
	gc->set_rgb_fg_color(Gdk::Color("#000000"));
	get_window()->draw_rectangle(gc, false, 0, 0, w-1, h-1);

	// Draw the position of the current value
	i=(int)(((level-1.0f)*2.0f+1.0f-0.5f)*w+0.5);
	gc->set_rgb_fg_color(Gdk::Color("#00ff00"));
	get_window()->draw_rectangle(gc, true, i, 1, 1, h-1);

	// Print out the value
	Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
	layout->set_text(etl::strprintf("%0.02f",level));
	layout->set_alignment(Pango::ALIGN_CENTER);
	gc->set_rgb_fg_color(Gdk::Color("#a00000"));
	get_window()->draw_layout(gc, w/2, 4, layout);

	return true;
}



bool
RedBlueLevelSelector::on_event(GdkEvent *event)
{
	int x(round_to_int(event->button.x));
	//int y(round_to_int(event->button.y));

    switch(event->type)
    {
	case GDK_MOTION_NOTIFY:
		level=(((float)(x)/(float)get_width()+0.5)-1.0f)/2.0f+1.0f;
		if(level<0.5f)level=0.5f;
		if(level>1.5f)level=1.5f;
		signal_value_changed_();
		queue_draw();
		return true;
		break;
	case GDK_BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		if(event->button.button==1)
		{
			level=(((float)(x)/(float)get_width()+0.5)-1.0f)/2.0f+1.0f;
			if(level<0.5f)level=0.5f;
			if(level>1.5f)level=1.5f;
			signal_value_changed_();
			queue_draw();
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}
