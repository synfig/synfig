/* === S Y N F I G ========================================================= */
/*!	\file widget_defaults.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Chris Moore
**  Copyright (c) 2008, 2011, 2012 Carlos López
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

#include "widgets/widget_defaults.h"
#include "widgets/widget_color.h"
#include "widgets/widget_gradient.h"
#include "dialogs/dialog_color.h"
#include "dialogs/dialog_gradient.h"
#include "app.h"
#include <gtkmm/menu.h>
#include <gtkmm/scale.h>
#include <synfig/exception.h>
#include <synfigapp/main.h>
#include "canvasview.h"
#include "widgets/widget_enum.h"
#include "widgets/widget_distance.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define DEFAULT_INCREMENT	(0.25)
#define DEFAULT_WIDTH		(synfig::Distance(3,synfig::Distance::SYSTEM_POINTS))

/* === G L O B A L S ======================================================= */

class studio::Widget_Brush : public Gtk::DrawingArea
{
public:
	Widget_Brush()
	{
		signal_expose_event().connect(sigc::mem_fun(*this, &studio::Widget_Brush::redraw));

		set_size_request(22, 22);
		add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
		add_events(Gdk::BUTTON1_MOTION_MASK);

		synfigapp::Main::signal_outline_color_changed().connect(sigc::mem_fun(*this,&studio::Widget_Brush::queue_draw));
		synfigapp::Main::signal_fill_color_changed().connect(sigc::mem_fun(*this,&studio::Widget_Brush::queue_draw));
		synfigapp::Main::signal_bline_width_changed().connect(sigc::mem_fun(*this,&studio::Widget_Brush::queue_draw));
		studio::App::signal_instance_selected().connect(sigc::hide(sigc::mem_fun(*this,&studio::Widget_Brush::queue_draw)));
	}

	bool
	redraw(GdkEventExpose */*bleh*/)
	{
		//!Check if the window we want draw is ready
		Glib::RefPtr<Gdk::Window> window = get_window();
		if(!window) return false;

		Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));

		const int h(get_height());
		const int w(get_width());

		float pixelsize(0);
		if(App::get_selected_canvas_view())
		{
			const RendDesc& rend_desc(App::get_selected_canvas_view()->get_canvas()->rend_desc());
			pixelsize=synfigapp::Main::get_bline_width().get(Distance::SYSTEM_PIXELS,rend_desc);
		}
		else
		{
			RendDesc rend_desc;
			pixelsize=synfigapp::Main::get_bline_width().get(Distance::SYSTEM_PIXELS,rend_desc);
		}
		// Fill in the fill color
		render_color_to_window(window,Gdk::Rectangle(0,0,w,h),synfigapp::Main::get_fill_color());

		// Draw in the circle
		gc->set_rgb_fg_color(colorconv_synfig2gdk(synfigapp::Main::get_outline_color()));
		gc->set_function(Gdk::COPY);
		gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
		window->draw_arc(
			gc,
			true,
			round_to_int(((float)w/2.0f)-pixelsize/2.0f),
			round_to_int(((float)h/2.0f)-pixelsize/2.0f),
			round_to_int(pixelsize+0.6),
			round_to_int(pixelsize+0.6),
			0,
			360*64
		);

		return true;
	}

	bool
	on_event(GdkEvent *event)
	{
		const int y(static_cast<int>(event->button.y));

		const int h(get_height());

		switch(event->type)
		{
			case GDK_MOTION_NOTIFY:
				break;
			case GDK_BUTTON_RELEASE:
				if(event->button.button==1) // Left click
				{
					Distance dist(synfigapp::Main::get_bline_width());

					if(y<h/2) // increase BLine size
					{
						dist+=DEFAULT_INCREMENT;
					}
					else // Decrease BLine size
					{
						dist-=DEFAULT_INCREMENT;
					}
					synfigapp::Main::set_bline_width(dist);
					return true;
				}
				if(event->button.button==3)
				{
					// right click on bline width
					synfigapp::Main::set_bline_width(DEFAULT_WIDTH);
					return true;
				}
				break;
			case GDK_SCROLL:
				{
					Distance dist(synfigapp::Main::get_bline_width());

					switch(event->scroll.direction){
						case GDK_SCROLL_UP:
						case GDK_SCROLL_RIGHT:
							dist+=DEFAULT_INCREMENT;
						break;
						case GDK_SCROLL_DOWN:
						case GDK_SCROLL_LEFT:
							dist-=DEFAULT_INCREMENT;
						break;
					}
					synfigapp::Main::set_bline_width(dist);
					return true;
				}
			default:
				break;
		}

		return false;
	}

};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Defaults::Widget_Defaults()
{
	set_row_spacings(4);
	set_col_spacings(2);
	Gtk::IconSize iconsize = Gtk::IconSize::from_name("synfig-small_icon");

	// widget colors: outline color and fill color
	Gtk::Table* widget_colors(manage(new Gtk::Table()));
	{
		int x = 22;

		// Outline Color
		widget_otln_color=manage(new Widget_Color());
		widget_otln_color->show();
		widget_otln_color->set_size_request(x, x);
		widget_otln_color->signal_clicked().connect(sigc::mem_fun(*this,&Widget_Defaults::on_otln_color_clicked));
		widget_otln_color->set_tooltip_text(_("Outline Color"));

		// Fill Color
		widget_fill_color=manage(new Widget_Color());
		widget_fill_color->show();
		widget_fill_color->set_size_request(x, x);
		widget_fill_color->signal_clicked().connect(sigc::mem_fun(*this,&Widget_Defaults::on_fill_color_clicked));
		widget_fill_color->set_tooltip_text(_("Fill Color"));

		Gtk::Image* icon;

		// Swap button
		Gtk::Button* button_swap(manage(new Gtk::Button()));
		button_swap->set_relief(Gtk::RELIEF_NONE);
		button_swap->set_border_width(0);
		icon=manage(new Gtk::Image(Gtk::StockID("synfig-swap_colors"), iconsize));
		button_swap->add(*icon);
		dynamic_cast<Gtk::Misc*>(button_swap->get_child())->set_padding(0, 0);
		button_swap->signal_clicked().connect(sigc::mem_fun(*this,&Widget_Defaults::on_swap_color_clicked));
		button_swap->set_tooltip_text(_("Swap Fill and\nOutline Colors"));

		// Reset button
		Gtk::Button* button_reset(manage(new Gtk::Button()));
		button_reset->show();
		button_reset->set_relief(Gtk::RELIEF_NONE);
		button_reset->set_border_width(0);
		icon=manage(new Gtk::Image(Gtk::StockID("synfig-reset_colors"), iconsize));
		button_reset->add(*icon);
		dynamic_cast<Gtk::Misc*>(button_reset->get_child())->set_padding(0, 0);
		button_reset->signal_clicked().connect(sigc::mem_fun(*this,&Widget_Defaults::on_reset_color_clicked));
		button_reset->set_tooltip_text(_("Reset Colors to Black and White"));

		// ship child widgets together
		widget_colors->attach(*widget_otln_color, 	0, 4, 0, 4, Gtk::FILL, Gtk::FILL, 0, 0);
		widget_colors->attach(*widget_fill_color, 	3, 7, 3, 7, Gtk::FILL, Gtk::FILL, 0, 0);
		widget_colors->attach(*button_swap, 		4, 7, 0, 3, Gtk::FILL, Gtk::FILL, 0, 0);
		widget_colors->attach(*button_reset, 		0, 3, 4, 7, Gtk::FILL, Gtk::FILL, 0, 0);

		widget_colors->set_homogeneous(true);
		widget_colors->show_all();
	}

	// widget brush
	widget_brush=manage(new Widget_Brush());
	widget_brush->set_size_request(22, 22);
	widget_brush->set_tooltip_text(_("Brush Preview"));

	// widget bline width
	widget_bline_width=manage(new Widget_Distance());
	bline_width_refresh();
	widget_bline_width->set_digits(2);
	widget_bline_width->set_range(0,10000000);
	widget_bline_width->set_size_request(64, 22);
	widget_bline_width->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_bline_width_changed));
	widget_bline_width->set_tooltip_text(_("Brush Size"));

	// widget blend method
	widget_blend_method=manage(new Widget_Enum());
	widget_blend_method->signal_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_blend_method_changed));
	widget_blend_method->set_param_desc(
		ParamDesc(Color::BLEND_COMPOSITE,"blend_method")
		.add_enum_value(Color::BLEND_BY_LAYER,"bylayer", _("By Layer Default"))
	);
	widget_blend_method->set_size_request(-1, 22);
	widget_blend_method->set_tooltip_text(_("Default Blend Method"));

	// widget interpolation
	widget_interpolation=manage(new Widget_Enum());
	widget_interpolation->signal_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_interpolation_changed));
	widget_interpolation->set_param_desc(
		ParamDesc("interpolation")
			.set_hint("enum")
			.add_enum_value(INTERPOLATION_CLAMPED,"clamped",_("Clamped"))
			.add_enum_value(INTERPOLATION_TCB,"auto",_("TCB"))
			.add_enum_value(INTERPOLATION_CONSTANT,"constant",_("Constant"))
			.add_enum_value(INTERPOLATION_HALT,"ease",_("Ease In/Out"))
			.add_enum_value(INTERPOLATION_LINEAR,"linear",_("Linear"))
	);
	widget_interpolation->set_icon(0, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_clamped"), iconsize));
	widget_interpolation->set_icon(1, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_tcb"), iconsize));
	widget_interpolation->set_icon(2, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_const"), iconsize));
	widget_interpolation->set_icon(3, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_ease"), iconsize));
	widget_interpolation->set_icon(4, Gtk::Button().render_icon(Gtk::StockID("synfig-interpolation_type_linear"), iconsize));
	synfigapp::Main::set_interpolation(INTERPOLATION_CLAMPED); // Clamped by default.
	widget_interpolation->set_size_request(-1, 22);
	widget_interpolation->set_tooltip_text(_("Default Interpolation"));

	// widget opacity
	widget_opacity=manage(new Gtk::HScale(0.0f,1.01f,0.01f));
	widget_opacity->set_digits(2);
	widget_opacity->set_value_pos(Gtk::POS_LEFT);
	widget_opacity->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_opacity_changed));
	widget_opacity->set_tooltip_text(_("Default Opacity"));
	widget_opacity->set_value_pos(Gtk::POS_LEFT);

	// widget gradient
	widget_gradient=manage(new Widget_Gradient());
	widget_gradient->signal_clicked().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_gradient_clicked));
	widget_gradient->set_size_request(-1, 22);
	widget_gradient->set_tooltip_text(_("Default Gradient"));

	// ship widgets together
	attach(*widget_colors, 		0, 1, 0, 2, Gtk::FILL, Gtk::FILL, 0, 0);
	attach(*widget_brush, 		1, 2, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL, 0, 0);
	attach(*widget_bline_width, 	1, 2, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL, 0, 0);
	attach(*widget_blend_method, 	0, 2, 2, 3, Gtk::FILL|Gtk::EXPAND, Gtk::FILL, 0, 0);
	attach(*widget_opacity,		0, 2, 3, 4, Gtk::FILL|Gtk::EXPAND, Gtk::FILL, 0, 0);
	attach(*widget_interpolation, 	0, 2, 5, 6, Gtk::FILL|Gtk::EXPAND, Gtk::FILL, 0, 0);
	attach(*widget_gradient, 	0, 2, 4, 5, Gtk::FILL|Gtk::EXPAND, Gtk::FILL, 0, 0);

	// show all widgets
	widget_colors->show();
	widget_brush->show();
	widget_bline_width->show();
	widget_blend_method->show();
	widget_interpolation->show();
	widget_opacity->show();
	widget_gradient->show();

	// Signals
	synfigapp::Main::signal_opacity_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::opacity_refresh));
	synfigapp::Main::signal_bline_width_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::bline_width_refresh));
	synfigapp::Main::signal_outline_color_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::otln_color_refresh));
	synfigapp::Main::signal_fill_color_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::fill_color_refresh));
	synfigapp::Main::signal_gradient_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::gradient_refresh));
	synfigapp::Main::signal_blend_method_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::blend_method_refresh));
	synfigapp::Main::signal_interpolation_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::interpolation_refresh));

	otln_color_refresh();
	fill_color_refresh();
	gradient_refresh();
	blend_method_refresh();
	opacity_refresh();
	interpolation_refresh();
}

Widget_Defaults::~Widget_Defaults()
{
}

void
Widget_Defaults::otln_color_refresh()
{
	widget_otln_color->set_value(synfigapp::Main::get_outline_color());
}

void
Widget_Defaults::fill_color_refresh()
{
	widget_fill_color->set_value(synfigapp::Main::get_fill_color());
}

void
Widget_Defaults::gradient_refresh()
{
	widget_gradient->set_value(synfigapp::Main::get_gradient());
}

void
Widget_Defaults::bline_width_refresh()
{
	widget_bline_width->set_value(synfigapp::Main::get_bline_width());
}

void
Widget_Defaults::blend_method_refresh()
{
	widget_blend_method->set_value(synfigapp::Main::get_blend_method());
}

void
Widget_Defaults::interpolation_refresh()
{
	widget_interpolation->set_value(synfigapp::Main::get_interpolation());
}

void
Widget_Defaults::opacity_refresh()
{
	widget_opacity->set_value(synfigapp::Main::get_opacity());
}

void
Widget_Defaults::on_opacity_changed()
{
	synfigapp::Main::set_opacity(widget_opacity->get_value());
}

void
Widget_Defaults::on_blend_method_changed()
{
	synfigapp::Main::set_blend_method(Color::BlendMethod(widget_blend_method->get_value()));
}

void
Widget_Defaults::on_interpolation_changed()
{
	synfigapp::Main::set_interpolation(Waypoint::Interpolation(widget_interpolation->get_value()));
}

void
Widget_Defaults::on_bline_width_changed()
{
	synfigapp::Main::set_bline_width(widget_bline_width->get_value());
}

void
Widget_Defaults::on_otln_color_clicked()
{
	// Left click on outline color
	App::dialog_color->set_color(synfigapp::Main::get_outline_color());
	App::dialog_color->reset();
	App::dialog_color->signal_edited().connect(sigc::ptr_fun(synfigapp::Main::set_outline_color));
	App::dialog_color->present();
}

void
Widget_Defaults::on_fill_color_clicked()
{
	// Left click on fill color
	App::dialog_color->set_color(synfigapp::Main::get_fill_color());
	App::dialog_color->reset();
	App::dialog_color->signal_edited().connect(sigc::ptr_fun(synfigapp::Main::set_fill_color));
	App::dialog_color->present();
}

void
Widget_Defaults::on_swap_color_clicked()
{
	synfigapp::Main::color_swap();
}

void
Widget_Defaults::on_reset_color_clicked()
{
	synfigapp::Main::set_fill_color(Color::white());
	synfigapp::Main::set_outline_color(Color::black());
}

void
Widget_Defaults::on_gradient_clicked()
{
	App::dialog_gradient->set_gradient(synfigapp::Main::get_gradient());
	App::dialog_gradient->reset();
	App::dialog_gradient->signal_edited().connect(sigc::ptr_fun(synfigapp::Main::set_gradient));
	App::dialog_gradient->set_default_button_set_sensitive(false);
	App::dialog_gradient->present();
}

