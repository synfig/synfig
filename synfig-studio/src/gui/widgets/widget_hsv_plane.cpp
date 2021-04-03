/*!	\file gui/widgets/widget_hsv_plane.cpp
**	\brief Widget that displays a HSV plane for color choosing
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2021 Rodolfo Ribeiro Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/widgets/widget_hsv_plane.h>

#include <gdkmm/general.h>
#include <glibmm/main.h>

#include <gui/exception_guard.h>

#endif

using namespace synfig;
using namespace studio;

#define clamp(value_, min_, max_) std::min(max_, std::max(min_, value_))
static const Gdk::RGBA default_color("#F00");

static synfig::Color to_synfig_color(const Gdk::RGBA c)
{
	Color color;
	color.set_r(c.get_red());
	color.set_g(c.get_green());
	color.set_b(c.get_blue());
	color.set_a(c.get_alpha());
	return color;
}

static Gdk::RGBA to_gdk_rgba(const synfig::Color c)
{
	Gdk::RGBA color;
	color.set_red(c.get_r());
	color.set_green(c.get_g());
	color.set_blue(c.get_b());
	color.set_alpha(c.get_a());
	return color;
}

void Widget_HSV_Plane::init()
{
	hue = 0.0;
	queued_redraw_pixbuf = false;
	pixbuf_invalid = true;
	dragging = false;

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK | Gdk::STRUCTURE_MASK);

	property_color.get_proxy().signal_changed().connect([&]() {
		set_color(to_synfig_color(property_color.get_value()));
	});
}

Widget_HSV_Plane::Widget_HSV_Plane()
	: Glib::ObjectBase("widget_hsv_plane"),
	  property_color(*this, "color", default_color)
{
	init();
}

Widget_HSV_Plane::Widget_HSV_Plane(BaseObjectType* cobject)
	: Glib::ObjectBase("widget_hsv_plane"),
	  Gtk::DrawingArea(cobject),
	  property_color(*this, "color", default_color)
{
	init();
}

void Widget_HSV_Plane::set_color(Color new_value)
{
	if (color == new_value)
		return;

	double h;
	gtk_rgb_to_hsv(new_value.get_r(), new_value.get_g(), new_value.get_b(), &h, nullptr, nullptr);

	synfig::Real new_hue = h*360;
	if (synfig::approximate_not_equal(hue, new_hue)) {
		hue = new_hue;
		queue_redraw_pixbuf();
	}

	color = new_value;
	if (color != to_synfig_color(property_color.get_value()))
		property_color = to_gdk_rgba(color);
	queue_draw();
}

Color Widget_HSV_Plane::get_color() const
{
	return color;
}

Widget_HSV_Plane::type_signal_activated Widget_HSV_Plane::signal_activated()
{
	return signal_activated_;
}

Widget_HSV_Plane::type_signal_editing_started Widget_HSV_Plane::signal_editing_started()
{
	return signal_editing_started_;
}

Widget_HSV_Plane::type_signal_editing_done Widget_HSV_Plane::signal_editing_done()
{
	return signal_editing_done_;
}

Widget_HSV_Plane::type_signal_editing_canceled Widget_HSV_Plane::signal_editing_canceled()
{
	return signal_editing_canceled_;
}

Widget_HSV_Plane::type_signal_value_changed Widget_HSV_Plane::signal_value_changed()
{
	return signal_value_changed_;
}

void Widget_HSV_Plane::get_preferred_height_vfunc(int& minimum_height, int& natural_height) const
{
	minimum_height = 128;
	natural_height = 256;
}

void Widget_HSV_Plane::get_preferred_width_vfunc(int& minimum_width, int& natural_width) const
{
	minimum_width = 128;
	natural_width = 256;
}

void Widget_HSV_Plane::get_preferred_height_for_width_vfunc(int width, int& minimum_height, int& natural_height) const
{
	minimum_height = 128;
	natural_height = width;
}

void Widget_HSV_Plane::get_preferred_width_for_height_vfunc(int height, int& minimum_width, int& natural_width) const
{
	minimum_width = 128;
	natural_width = height;
}

bool Widget_HSV_Plane::on_button_press_event(GdkEventButton* button_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	if (button_event->button == GDK_BUTTON_SECONDARY) {
		if (dragging) {
			dragging = false;
			color = previous_color;
			signal_editing_canceled().emit();
		}
		return false;
	}

	synfig::Color new_color = get_color_at(button_event->x, button_event->y);
	if (new_color != color) {
		previous_color = color;
		color = new_color;
		queue_draw();
		signal_value_changed().emit();
	}

	dragging = true;

	signal_editing_started().emit();

	SYNFIG_EXCEPTION_GUARD_END_BOOL(false)
}

bool Widget_HSV_Plane::on_button_release_event(GdkEventButton* button_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	if (button_event->button != GDK_BUTTON_PRIMARY)
		return false;

	if (dragging)
		signal_editing_done().emit();
	dragging = false;
	signal_activated().emit();

	SYNFIG_EXCEPTION_GUARD_END_BOOL(false)
}

bool Widget_HSV_Plane::on_configure_event(GdkEventConfigure* configure_event)
{
	std::lock_guard<std::mutex> lock(pixbuf_mutex);
	pixbuf_invalid = true;
	pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, configure_event->width, configure_event->height);
	queue_redraw_pixbuf();

	return false;
}

bool Widget_HSV_Plane::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	const int width = pixbuf->get_width();
	const int height = pixbuf->get_height();

	std::lock_guard<std::mutex> lock(pixbuf_mutex);
	if (!pixbuf || pixbuf_invalid)
		return false;

	Gdk::Cairo::set_source_pixbuf(cr, pixbuf, 0, 0);
	cr->rectangle(0,0,width,height);
	cr->fill();

	gdouble s,v;
	gtk_rgb_to_hsv(color.get_r(), color.get_g(), color.get_b(), nullptr, &s, &v);
	if (v == 0. && dragging) {
		int x, y;
		get_pointer(x, y);
		s = clamp(x/double(width), 0.0, 1.0);
	}

	if (color.get_y() > 0.3f)
		cr->set_source_rgb(0,0,0);
	else
		cr->set_source_rgb(1,1,1);

	cr->arc(s*width, v*height, 4, 0, 6.28);
	cr->stroke();

	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool Widget_HSV_Plane::on_motion_notify_event(GdkEventMotion* motion_event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()

	if (!dragging)
		return false;

	synfig::Color new_color = get_color_at(motion_event->x, motion_event->y);
	if (new_color != color) {
		color = new_color;
		queue_draw();
		signal_value_changed().emit();
	}

	SYNFIG_EXCEPTION_GUARD_END_BOOL(false)
}

void Widget_HSV_Plane::queue_redraw_pixbuf()
{
	if (queued_redraw_pixbuf)
		return;
	queued_redraw_pixbuf = true;
	Glib::signal_idle().connect_once(sigc::mem_fun(*this, &Widget_HSV_Plane::redraw_pixbuf));
}

void Widget_HSV_Plane::redraw_pixbuf()
{
	std::lock_guard<std::mutex> lock(pixbuf_mutex);
	if (!pixbuf) {
		queued_redraw_pixbuf = false;
		return;
	}

	const int width = pixbuf->get_width();
	const int height = pixbuf->get_height();

	queued_redraw_pixbuf = false;

	for (int y = 0; y < height; ++y) {
		const int offset = y * pixbuf->get_rowstride();
		guint8 *pixels = &pixbuf->get_pixels()[offset];

		const Real v = y/Real(height);

		for (int x = 0; x < width; ++x) {
			const Real s = x/Real(width);
			Gdk::RGBA color;
			color.set_hsv(hue, s, v);
			pixels[0] = color.get_red_u() >> 8;
			pixels[1] = color.get_green_u() >> 8;
			pixels[2] = color.get_blue_u() >> 8;
			pixels += 3;
		}
	}

	pixbuf_invalid = false;
	queue_draw();
}

Color Widget_HSV_Plane::get_color_at(double x, double y) const
{
	double s = clamp(x/get_width(), 0.0, 1.0);
	double v = clamp(y/get_height(), 0.0, 1.0);

	Gdk::RGBA c;
	c.set_hsv(hue,s,v);
	c.set_alpha(color.get_alpha());

	return to_synfig_color(c);
}

// Glade & GtkBuilder related

GType Widget_HSV_Plane::gtype = 0;

Glib::ObjectBase* Widget_HSV_Plane::wrap_new(GObject* o)
{
	if (gtk_widget_is_toplevel(GTK_WIDGET(o)))
		return new Widget_HSV_Plane(GTK_DRAWING_AREA(o));
	else
		return Gtk::manage(new Widget_HSV_Plane(GTK_DRAWING_AREA(o)));
}

void Widget_HSV_Plane::register_type()
{
	if (gtype)
		return;

	Widget_HSV_Plane dummy;

	gtype = G_OBJECT_TYPE(dummy.gobj());

	Glib::wrap_register(gtype, Widget_HSV_Plane::wrap_new);
}
