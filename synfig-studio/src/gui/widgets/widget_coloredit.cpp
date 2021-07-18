/* === S Y N F I G ========================================================= */
/*!	\file widget_coloredit.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**  Copyright (c) 2008 Paul Wise
**  Copyright (c) 2015 Denis Zdorovtsov
**  Copyright (c) 2015-2016 Jérôme Blanchi
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

#include <gui/widgets/widget_coloredit.h>

#include <gtkmm/notebook.h>
#include <gtkmm/box.h>
#include <gtkmm/separator.h>
#include <gtkmm/stylecontext.h>

#include <gui/app.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>
#include <gui/resourcehelper.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S ======================================================= */


/* === M E T H O D S ======================================================= */
#include <glibmm/fileutils.h> // Glib::FileError
#include <glibmm/main.h> // Glib::signal_idle()
#include <glibmm/markup.h> // Glib::MarkupError
#include <gtkmm/builder.h>
#include <gtkmm/togglebutton.h>
Widget_ColorEdit::Widget_ColorEdit()
{
	try
	{
		auto builder = Gtk::Builder::create_from_file(ResourceHelper::get_ui_path("widget_coloredit.glade"));
		Gtk::Grid *grid;
		builder->get_widget("Widget_ColorEdit", grid);
		add(*grid);
		builder->get_widget("color_slider_r", slider_R);
		builder->get_widget("color_slider_g", slider_G);
		builder->get_widget("color_slider_b", slider_B);
		builder->get_widget("color_slider_a", slider_A);
		builder->get_widget("vertical_color_slider", slider_vertical);
		builder->get_widget("hsv_plane", hsv_plane);
		builder->get_widget("color", widget_color);
		builder->get_widget("eyedropper", widget_eyedropper);
		Gtk::Button *button_eyedropper;
		builder->get_widget("button_eyedropper", button_eyedropper);
		if (button_eyedropper && widget_eyedropper) {
			button_eyedropper->signal_clicked().connect(sigc::mem_fun(*this, &Widget_ColorEdit::on_button_eyedropper_clicked));
			widget_eyedropper->signal_color_picked().connect(sigc::mem_fun(*this, &Widget_ColorEdit::on_eyedropper_picked));
		}

		color = Color::red();

		auto on_color_changed = [&](Widget_ColorSlider::Type type) {
			Color new_color(color);
			Widget_ColorSlider::adjust_color(type, new_color, .8f);
			if (color != new_color) {
				color = new_color;
				slider_R->set_color(color);
				slider_G->set_color(color);
				slider_B->set_color(color);
				slider_A->set_color(color);
				slider_vertical->set_color(color);
				widget_color->set_value(color);
				hsv_plane->set_color(color);
			}
		};

		auto on_slider_moved = [&](Widget_ColorSlider::Type type, float value) {
			Color new_color(color);
			Widget_ColorSlider::adjust_color(type, new_color, value);
			set_value(new_color);
		};

		slider_R->property_type = Widget_ColorSlider::TYPE_R;
		slider_G->property_type = Widget_ColorSlider::TYPE_G;
		slider_B->property_type = Widget_ColorSlider::TYPE_B;
		slider_A->property_type = Widget_ColorSlider::TYPE_A;
		slider_vertical->property_type = Widget_ColorSlider::TYPE_HUE;
		slider_vertical->property_orientation = Gtk::ORIENTATION_VERTICAL;

		slider_R->signal_slider_moved().connect(on_slider_moved);
		slider_G->signal_slider_moved().connect(on_slider_moved);
		slider_B->signal_slider_moved().connect(on_slider_moved);
		slider_A->signal_slider_moved().connect(on_slider_moved);
		slider_vertical->signal_slider_moved().connect(on_slider_moved);

		hsv_plane->signal_activated().connect([&](){set_value(hsv_plane->get_color());});

		set_value(color);
	}
	catch(const Glib::FileError& ex)
	{
		synfig::error("FileError: " + ex.what());
	}
	catch(const Glib::MarkupError& ex)
	{
		synfig::error("MarkupError: " + ex.what());
	}
	catch(const Gtk::BuilderError& ex)
	{
		synfig::error("BuilderError: " + ex.what());
	}
}

void
Widget_ColorEdit::set_has_frame(bool x)
{
//	spinbutton_R->set_has_frame(x);
//	spinbutton_G->set_has_frame(x);
//	spinbutton_B->set_has_frame(x);
//	spinbutton_A->set_has_frame(x);
}

//void
//Widget_ColorEdit::set_digits(int x)
//{
//	spinbutton_R->set_digits(x);
//	spinbutton_G->set_digits(x);
//	spinbutton_B->set_digits(x);
//	spinbutton_A->set_digits(x);
//}

void Widget_ColorEdit::on_button_eyedropper_clicked()
{
	Glib::signal_timeout().connect_once(sigc::mem_fun(*widget_eyedropper, &Widget_Eyedropper::grab), 250);
}

void Widget_ColorEdit::on_eyedropper_picked(const Gdk::RGBA& rgba)
{
	Color picked_color(rgba.get_red(), rgba.get_green(), rgba.get_blue());
	set_value(picked_color);
}

void
Widget_ColorEdit::set_value(const synfig::Color &new_color)
{
	if (color != new_color) {
		color = new_color;
		Color fixed_color = new_color.clamped();
		slider_R->set_color(fixed_color);
		slider_G->set_color(fixed_color);
		slider_B->set_color(fixed_color);
		slider_A->set_color(fixed_color);
		slider_vertical->set_color(fixed_color);
		widget_color->set_value(fixed_color);
		hsv_plane->set_color(fixed_color);
		signal_value_changed().emit();
	}
	signal_activate().emit();
}



const synfig::Color &
Widget_ColorEdit::get_value()
{

	return color;
}
