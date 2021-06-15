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

#include <gui/widgets/widget_defaults.h>

#include <gtkmm/stylecontext.h>
#include <gtkmm/toolitem.h>
#include <gtkmm/toolitemgroup.h>
#include <gtkmm/toolpalette.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/dialogs/dialog_color.h>
#include <gui/dialogs/dialog_gradient.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/widgets/widget_color.h>
#include <gui/widgets/widget_distance.h>
#include <gui/widgets/widget_gradient.h>

#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

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
		add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
		add_events(Gdk::BUTTON1_MOTION_MASK);

		synfigapp::Main::signal_outline_color_changed().connect(sigc::mem_fun(*this,&studio::Widget_Brush::queue_draw));
		synfigapp::Main::signal_fill_color_changed().connect(sigc::mem_fun(*this,&studio::Widget_Brush::queue_draw));
		synfigapp::Main::signal_bline_width_changed().connect(sigc::mem_fun(*this,&studio::Widget_Brush::queue_draw));
		studio::App::signal_instance_selected().connect(sigc::hide(sigc::mem_fun(*this,&studio::Widget_Brush::queue_draw)));
	}

	bool
	on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
	{
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
		render_color_to_window(cr,Gdk::Rectangle(0,0,w,h),synfigapp::Main::get_fill_color());

		// Draw in the circle
		
		Color brush = App::get_selected_canvas_gamma().get_inverted().apply(
			synfigapp::Main::get_outline_color() );
		cr->set_source_rgba(brush.get_r(), brush.get_g(), brush.get_b(), brush.get_a());
		cr->arc(w/2.0, h/2.0, pixelsize, 0.0, 360*M_PI/180.0);
		cr->fill();
		return true;
	}

	bool
	on_event(GdkEvent *event)
	{
		SYNFIG_EXCEPTION_GUARD_BEGIN()
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
						default:
							break;
					}
					synfigapp::Main::set_bline_width(dist);
					return true;
				}
			default:
				break;
		}

		return false;

		SYNFIG_EXCEPTION_GUARD_END_BOOL(false)
	}

};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Defaults::Widget_Defaults():
	Gtk::Box(Gtk::ORIENTATION_VERTICAL)
{
	// Make Brushes button small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	Gtk::IconSize iconsize = Gtk::IconSize::from_name("synfig-tiny_icon");

	// widget colors: outline color and fill color.

	widget_colors = manage(new Gtk::Grid());
	{
		// widget outline color
		widget_outline_color = manage(new Widget_Color());
		widget_outline_color->set_size_request(30, 26);
		widget_outline_color->signal_clicked().connect(sigc::mem_fun(*this,&Widget_Defaults::on_outline_color_clicked));
		widget_outline_color->set_tooltip_text(_("Outline Color"));

		// fixed outline color widget size
		widget_outline_color->set_halign(Gtk::ALIGN_END);
		widget_outline_color->set_valign(Gtk::ALIGN_END);


		// widget fill color
		widget_fill_color = manage(new Widget_Color());
		widget_fill_color->set_size_request(30, 26);
		widget_fill_color->signal_clicked().connect(sigc::mem_fun(*this,&Widget_Defaults::on_fill_color_clicked));
		widget_fill_color->set_tooltip_text(_("Fill Color"));

		// fixed fill color wiget size
		widget_fill_color->set_halign(Gtk::ALIGN_END);
		widget_fill_color->set_valign(Gtk::ALIGN_END);

		Gtk::Image* icon;

		// Swap button
		Gtk::Button* button_swap(manage(new Gtk::Button()));
		button_swap->set_relief(Gtk::RELIEF_NONE);
		button_swap->set_border_width(0);
		icon = manage(new Gtk::Image(Gtk::StockID("synfig-swap_colors"), iconsize));
		button_swap->add(*icon);
		button_swap->signal_clicked().connect(sigc::mem_fun(*this,&Widget_Defaults::on_swap_color_clicked));
		button_swap->set_tooltip_text(_("Swap Fill and\nOutline Colors"));

		// fixed swap button widget size
		button_swap->set_valign(Gtk::ALIGN_CENTER);
		button_swap->set_halign(Gtk::ALIGN_CENTER);

		// Reset button
		Gtk::Button* button_reset(manage(new Gtk::Button()));
		button_reset->set_relief(Gtk::RELIEF_NONE);
		button_reset->set_border_width(0);
		icon = manage(new Gtk::Image(Gtk::StockID("synfig-reset_colors"), iconsize));
		button_reset->add(*icon);
		button_reset->signal_clicked().connect(sigc::mem_fun(*this,&Widget_Defaults::on_reset_color_clicked));
		button_reset->set_tooltip_text(_("Reset Colors to Black and White"));

		// fixed reset button widget size
		button_reset->set_halign(Gtk::ALIGN_CENTER);
		button_reset->set_valign(Gtk::ALIGN_CENTER);

		// ship child widgets together
		widget_colors->attach(*widget_outline_color, 0, 0, 1, 1);
		widget_colors->attach(*widget_fill_color, 1, 1, 1, 1);
		widget_colors->attach(*button_swap,       1, 0, 1, 1);
		widget_colors->attach(*button_reset,      0, 1, 1, 1);

		// fixed colors widget size
		widget_colors->set_valign(Gtk::ALIGN_CENTER);
		widget_colors->set_halign(Gtk::ALIGN_CENTER);

	}

	// widget brush
	widget_brush = manage(new Widget_Brush());
	widget_brush->set_size_request(56, 48);
	widget_brush->set_tooltip_text(_("Brush Preview"));

	brush_increase = Gtk::manage(new class Gtk::Button("+"));
	brush_increase->set_tooltip_text(_("Increase brush size"));
	brush_increase->set_relief(Gtk::RELIEF_NONE);
	brush_increase->set_border_width(0);
	brush_increase->signal_clicked().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_brush_increase_clicked));

	brush_decrease = Gtk::manage(new class Gtk::Button("-"));
	brush_decrease->set_tooltip_text(_("Decrease brush size"));
	brush_decrease->set_relief(Gtk::RELIEF_NONE);
	brush_decrease->set_border_width(0);
	brush_decrease->signal_clicked().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_brush_decrease_clicked));

	brush_entry = Gtk::manage(new class Gtk::Entry());
	brush_entry->set_width_chars(4);
	brush_entry->set_has_frame(false);
	brush_entry->signal_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_brush_entry_changed));
	brush_entry->set_tooltip_text(_("Brush Size"));

	auto brush_layout = Gtk::manage(new class Gtk::Grid());
	brush_layout->attach(*widget_brush,  0, 0, 2, 1);
	brush_layout->attach(*brush_decrease, 0, 1, 1, 1);
	brush_layout->attach(*brush_increase, 1, 1, 1, 1);
	brush_layout->attach(*brush_entry,    0, 2, 2, 1);
	brush_layout->show_all();

	// fixed brush widget size
	widget_brush->set_valign(Gtk::ALIGN_CENTER);
	widget_brush->set_halign(Gtk::ALIGN_CENTER);

	// widget bline width
	widget_bline_width = manage(new Widget_Distance());
	widget_bline_width->set_parent(*this);
	bline_width_refresh();
	widget_bline_width->set_digits(2);
	widget_bline_width->set_range(0,10000000);
	widget_bline_width->set_width_chars(4);
	//widget_bline_width->set_size_request(48, -1); //mini width of bline width widget, this value also affects mini width of whole default_widgets.
	widget_bline_width->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_bline_width_changed));
	widget_bline_width->set_tooltip_text(_("Brush Size"));


	// widget blend method

	/*
	* NOTE1: the blend method widget affects the width of whole default_widgets widget,
	* since it requires mini width by it lengest item, "By Layer Default". If it
	* was removed from toolbox as planned, then toolbutton in toolbox will have
	* proper column spacing by default. I will let it as it is, because the toolbox
	* still need more love.
	*
	* NOTE2: Commented out as of 2014-06-24 -- KD.
	*/

	//widget_blend_method = manage(new Widget_Enum());
	//widget_blend_method->signal_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_blend_method_changed));
	//widget_blend_method->set_param_desc(
	//	ParamDesc((int)Color::BLEND_COMPOSITE,"blend_method")
	//	.add_enum_value(Color::BLEND_BY_LAYER,"bylayer", _("By Layer Default"))
	//);
	//widget_blend_method->set_tooltip_text(_("Default Blend Method"));

	// widget opacity
	//widget_opacity = manage(new Gtk::Scale(0.0f,1.01f,0.01f));
	//widget_opacity->set_digits(2);
	//widget_opacity->set_value_pos(Gtk::POS_LEFT);
	//widget_opacity->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_opacity_changed));
	//widget_opacity->set_tooltip_text(_("Default Opacity"));
	//widget_opacity->set_value_pos(Gtk::POS_LEFT);

	// widget gradient
	widget_gradient = manage(new Widget_Gradient());
	widget_gradient->signal_clicked().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::on_gradient_clicked));
	widget_gradient->set_size_request(56, 24);
	widget_gradient->set_tooltip_text(_("Default Gradient"));

	// fixed gradient widget size
	widget_gradient->set_halign(Gtk::ALIGN_CENTER);
	widget_gradient->set_valign(Gtk::ALIGN_CENTER);

	// pack all widgets
	{
		// pack colors and gradient widgets
		{
			widget_colors_gradient = manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
			widget_colors_gradient->pack_start(*widget_colors);
			widget_colors_gradient->pack_start(*widget_gradient);
		}

		//pack_start(*widget_colors_gradient, Gtk::PACK_EXPAND_PADDING, 4);
		//pack_start(*widget_blend_method, Gtk::PACK_EXPAND_PADDING, 4);
		//pack_start(*widget_opacity, Gtk::PACK_EXPAND_PADDING, 4);
		//pack_start(*widget_brush, Gtk::PACK_EXPAND_PADDING, 6);

		// show all widgets
		widget_colors_gradient->show_all();
		//widget_blend_method->show();
		//widget_opacity->show();
		//widget_brush_bline_width->show_all();

		Gtk::ToolItemGroup *tool_item_group = manage(new class Gtk::ToolItemGroup());
		gtk_tool_item_group_set_label(tool_item_group->gobj(), NULL);

		Gtk::ToolPalette *palette = manage(new Gtk::ToolPalette());
		palette->add(*tool_item_group);
		palette->set_expand(*tool_item_group);
		palette->set_exclusive(*tool_item_group, true);
		palette->set_icon_size(Gtk::IconSize::from_name("synfig-small_icon_16x16"));
		palette->set_size_request(100,100);
		palette->show();

		Gtk::ToolItem *tool_item1 = manage(new class Gtk::ToolItem());
		tool_item1->add(*widget_colors_gradient);
		tool_item_group->insert(*tool_item1);
		tool_item1->show();
		Gtk::ToolItem *tool_item2 = manage(new class Gtk::ToolItem());
		tool_item2->add(*widget_brush);
		tool_item_group->insert(*tool_item2);
		tool_item2->show();

		tool_item_group->show_all();

		//Gtk::ScrolledWindow *scrolled_window = manage(new Gtk::ScrolledWindow());
		//scrolled_window->add(*palette);
		//scrolled_window->show();

		//pack_start(*scrolled_window, Gtk::PACK_EXPAND_PADDING, 4);
		pack_start(*palette, Gtk::PACK_EXPAND_WIDGET|Gtk::PACK_SHRINK, 4);

	}


	// Signals
	//synfigapp::Main::signal_opacity_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::opacity_refresh));
	synfigapp::Main::signal_bline_width_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::bline_width_refresh));
	synfigapp::Main::signal_outline_color_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::outline_color_refresh));
	synfigapp::Main::signal_fill_color_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::fill_color_refresh));
	synfigapp::Main::signal_gradient_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::gradient_refresh));
	//synfigapp::Main::signal_blend_method_changed().connect(sigc::mem_fun(*this,&studio::Widget_Defaults::blend_method_refresh));

	outline_color_refresh();
	fill_color_refresh();
	gradient_refresh();
	//blend_method_refresh();
	//opacity_refresh();
}

Widget_Defaults::~Widget_Defaults()
{
}

void
Widget_Defaults::outline_color_refresh()
{
	widget_outline_color->set_value(synfigapp::Main::get_outline_color());
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
	brush_entry->set_text(widget_bline_width->get_value().get_string(widget_bline_width->get_digits()));
}

/*
void
Widget_Defaults::blend_method_refresh()
{
	widget_blend_method->set_value(synfigapp::Main::get_blend_method());
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
*/

void
Widget_Defaults::on_bline_width_changed()
{
	synfigapp::Main::set_bline_width(widget_bline_width->get_value());
}

void
Widget_Defaults::on_brush_entry_changed()
{
	synfig::Distance distance(synfigapp::Main::get_bline_width());
	distance = synfig::String(brush_entry->get_text());
	synfigapp::Main::set_bline_width(distance);
}

void
Widget_Defaults::on_brush_increase_clicked()
{
	synfig::Distance distance(synfigapp::Main::get_bline_width());
	distance+=1;
	synfigapp::Main::set_bline_width(distance);
}

void
Widget_Defaults::on_brush_decrease_clicked()
{
	synfig::Distance distance(synfigapp::Main::get_bline_width());
	distance-=1;
	synfigapp::Main::set_bline_width(distance);
}

void
Widget_Defaults::on_outline_color_clicked()
{
	// Left click on outline color
	App::dialog_color->reset();
	App::dialog_color->set_color(synfigapp::Main::get_outline_color());
	App::dialog_color->signal_edited().connect(sigc::ptr_fun(synfigapp::Main::set_outline_color));
	App::dialog_color->present();
}

void
Widget_Defaults::on_fill_color_clicked()
{
	// Left click on fill color
	App::dialog_color->reset();
	App::dialog_color->set_color(synfigapp::Main::get_fill_color());
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

