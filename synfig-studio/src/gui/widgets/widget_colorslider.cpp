/*!	\file gui/widgets/widget_colorslider.h
**	\brief Widget for choosing the value of a color component
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

#include "widget_colorslider.h"

#include <gui/app.h>
#include <gui/exception_guard.h>

#endif

using namespace synfig;
using namespace studio;

static const Gdk::RGBA default_color("#F00");
static const int default_min_size = 16;

static synfig::Color
to_synfig_color(const Gdk::RGBA& c)
{
	Color color;
	color.set_r(c.get_red());
	color.set_g(c.get_green());
	color.set_b(c.get_blue());
	color.set_a(c.get_alpha());
	return color;
}

static Gdk::RGBA
to_gdk_rgba(const synfig::Color& c)
{
	Gdk::RGBA color;
	color.set_red(c.get_r());
	color.set_green(c.get_g());
	color.set_blue(c.get_b());
	color.set_alpha(c.get_a());
	return color;
}


void
ColorSlider::init(Type t)
{
	property_type = t;
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);

	property_type.get_proxy().signal_changed().connect([=](){
		queue_draw();
	});

	property_color.get_proxy().signal_changed().connect([=](){
		set_color(to_synfig_color(property_color.get_value()));
	});

	property_orientation.get_proxy().signal_changed().connect([=](){
		queue_draw();
	});
}

ColorSlider::ColorSlider(Type x)
	: Glib::ObjectBase("widget_colorslider"),
	  property_type(*this, "type", x),
	  property_color(*this, "color", default_color),
	  property_orientation(*this, "orientation", Gtk::ORIENTATION_HORIZONTAL)
{
	init(x);
}

ColorSlider::ColorSlider(Gtk::DrawingArea::BaseObjectType* cobject)
	: Glib::ObjectBase("widget_colorslider"),
	  Gtk::DrawingArea(cobject),
	  property_type(*this, "type", TYPE_Y),
	  property_color(*this, "color", default_color),
	  property_orientation(*this, "orientation", Gtk::ORIENTATION_HORIZONTAL)
{
	init(TYPE_Y);
}

void
ColorSlider::set_type(Type x)
{
	if (property_type == x)
		return;
	property_type = x;
}

void
ColorSlider::set_color(const Color& x)
{
	if (color_ == x)
		return;
	color_=x;
	if (color_ != to_synfig_color(property_color.get_value()))
		property_color = to_gdk_rgba(color_);

	queue_draw();
}

void ColorSlider::set_orientation(Gtk::Orientation x)
{
	if (property_orientation == x)
		return;
	property_orientation = x;
}

Gtk::Orientation ColorSlider::get_orientation() const
{
	return property_orientation;
}

float
ColorSlider::get_amount() const
{
	return get_amount(property_type, color_);
}

float
ColorSlider::get_amount(Type type, const synfig::Color& color)
{
	switch(type)
	{
	case TYPE_R: return color.get_r();
	case TYPE_G: return color.get_g();
	case TYPE_B: return color.get_b();
	case TYPE_Y: return color.get_y();
	case TYPE_U: return color.get_u()+0.5f;
	case TYPE_V: return color.get_v()+0.5f;
	case TYPE_HUE: {
		float amount = Angle::rot(color.get_uv_angle()).get();
		return amount - floorf(amount);
	}
	case TYPE_SAT: return color.get_s()*2.0f;
	case TYPE_A: return color.get_a();
	default: return 0;
	}
}

void
ColorSlider::slider_color_TYPE_R(synfig::Color &color, float amount) { color.set_r(amount); }
void
ColorSlider::slider_color_TYPE_G(synfig::Color &color, float amount) { color.set_g(amount); }
void
ColorSlider::slider_color_TYPE_B(synfig::Color &color, float amount) { color.set_b(amount); }
void
ColorSlider::slider_color_TYPE_Y(synfig::Color &color, float amount) { color.set_y(amount); }
void
ColorSlider::slider_color_TYPE_U(synfig::Color &color, float amount) { color.set_u(amount-0.5f); }
void
ColorSlider::slider_color_TYPE_V(synfig::Color &color, float amount) { color.set_v(amount-0.5f); }
void
ColorSlider::slider_color_TYPE_HUE(synfig::Color &color, float amount) { color.set_uv_angle(Angle::rot(amount)); }
void
ColorSlider::slider_color_TYPE_SAT(synfig::Color &color, float amount) { color.set_s(amount*0.5f); }
void
ColorSlider::slider_color_TYPE_A(synfig::Color &color, float amount) { color.set_a(amount); }

void
ColorSlider::adjust_color(Type type, synfig::Color &color, float amount)
{
	static const slider_color_func jump_table[int(TYPE_END)] =
	{
		slider_color_TYPE_R,
		slider_color_TYPE_G,
		slider_color_TYPE_B,
		slider_color_TYPE_Y,
		slider_color_TYPE_U,
		slider_color_TYPE_V,
		slider_color_TYPE_HUE,
		slider_color_TYPE_SAT,
		slider_color_TYPE_A,
	};
	jump_table[int(type)](color,amount);
}

void
ColorSlider::draw_arrow(
	const Cairo::RefPtr<Cairo::Context> &cr,
	double x, double y,
	double width, double height,
	int size,
	bool fill)
{
	// hardcoded colors
	const Color dark(0, 0, 0);
	const Color light(1, 1, 1);

	const bool is_horizontal = property_orientation == Gtk::ORIENTATION_HORIZONTAL;

	// Upper (or left) black pointing down (or right) arrow
	cr->set_source_rgb(dark.get_r(), dark.get_g(), dark.get_b());
	cr->set_line_width(1.0);
	if (is_horizontal) {
		cr->move_to(x, y);
		cr->line_to(x - 0.5*width, y - height);
		cr->line_to(x + 0.5*width, y - height);
	} else {
		cr->move_to(x, y);
		cr->line_to(x - width, y - height/2);
		cr->line_to(x - width, y + height/2);
	}
	cr->close_path();
	if (fill)
		cr->fill();
	else
		cr->stroke();

	// Bottom light (or right) pointing up (or left) arrow
	cr->set_source_rgb(light.get_r(), light.get_g(), light.get_b());
	cr->set_line_width(1.0);
	if (is_horizontal) {
		cr->move_to(x, size - height);
		cr->line_to(x - 0.5*width, size);
		cr->line_to(x + 0.5*width, size);
	} else {
		cr->move_to(size - width, y);
		cr->line_to(size, y - height/2);
		cr->line_to(size, y + height/2);
	}
	cr->close_path();
	if (fill)
		cr->fill();
	else
		cr->stroke();
}

bool
ColorSlider::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	Color color = color_;

	static const slider_color_func jump_table[int(TYPE_END)] =
	{
		slider_color_TYPE_R,
		slider_color_TYPE_G,
		slider_color_TYPE_B,
		slider_color_TYPE_Y,
		slider_color_TYPE_U,
		slider_color_TYPE_V,
		slider_color_TYPE_HUE,
		slider_color_TYPE_SAT,
		slider_color_TYPE_A,
	};

	slider_color_func color_func = jump_table[property_type];

	Gamma gamma = App::get_selected_canvas_gamma().get_inverted();

	float amount = get_amount();

	const bool is_horizontal = property_orientation == Gtk::ORIENTATION_HORIZONTAL;
	const int length = is_horizontal ? get_width() : get_height();
	const int thickness = is_horizontal ? get_height() : get_width();

	const Gdk::Rectangle ca(0, 0, length, thickness);

	const Color bg1(0.75, 0.75, 0.75);
	const Color bg2(0.5, 0.5, 0.5);

	const int bg_size = thickness/2;

	for(int i = length-1; i >= 0; --i)
	{
		Color c = color;
		color_func(c, i/float(length));

		const Color c1 = gamma.apply(
				Color::blend(c,bg1,1.0).clamped() );
		const Color c2 = gamma.apply(
				Color::blend(c,bg2,1.0).clamped() );
		assert(c1.is_valid());
		assert(c2.is_valid());

		int x1 = is_horizontal ? ca.get_x()+i         : ca.get_x();
		int x2 = is_horizontal ? ca.get_x()+i         : ca.get_x() + bg_size;
		int y1 = is_horizontal ? ca.get_y()           : ca.get_y() + length - i;
		int y2 = is_horizontal ? ca.get_y() + bg_size : ca.get_y() + length - i;
		int w  = is_horizontal ? 1       : bg_size;
		int h  = is_horizontal ? bg_size : 1;

		if((i*2/thickness) & 1)
		{
	        cr->set_source_rgb(c1.get_r(), c1.get_g(), c1.get_b());
	        cr->rectangle(x1, y1, w, h);
	        cr->fill();

	        cr->set_source_rgb(c2.get_r(), c2.get_g(), c2.get_b());
	        cr->rectangle(x2, y2, w, h);
	        cr->fill();
		}
		else
		{
	        cr->set_source_rgb(c2.get_r(), c2.get_g(), c2.get_b());
	        cr->rectangle(x1, y1, w, h);
	        cr->fill();

	        cr->set_source_rgb(c1.get_r(), c1.get_g(), c1.get_b());
	        cr->rectangle(x2, y2, w, h);
	        cr->fill();
		}
	}

    cr->set_source_rgb(1, 1, 1);
    cr->rectangle(ca.get_x()+1, ca.get_y()+1, get_width()-3, get_height()-3);
    cr->stroke();

    cr->set_source_rgb(0, 0, 0);
    cr->rectangle(ca.get_x(), ca.get_y(), get_width()-1, get_height()-1);
    cr->stroke();

    // Draw face to face contrasted arrows
	const int arrow_size = std::min(bg_size, 20);
	if (is_horizontal)
		draw_arrow(cr, int(amount*length), thickness/2, arrow_size, bg_size, thickness, true);
	else
		draw_arrow(cr, thickness/2, int((1 - amount)*length), bg_size, arrow_size, thickness, true);

	return true;
}

bool
ColorSlider::on_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	const bool is_horizontal = property_orientation == Gtk::ORIENTATION_HORIZONTAL;
	const int max_value = is_horizontal ? get_width() : get_height();
	float x = 0;
	if( GDK_SCROLL == event->type ){
		Color color(color_);
		float amount;
		switch(property_type)
		{
			case TYPE_R: amount=color.get_r(); break;
			case TYPE_G: amount=color.get_g(); break;
			case TYPE_B: amount=color.get_b(); break;
			case TYPE_Y: amount=color.get_y(); break;
			case TYPE_U: amount=color.get_u()+0.5f; break;
			case TYPE_V: amount=color.get_v()+0.5f; break;
			case TYPE_HUE: amount=Angle::rot(color.get_uv_angle()).get(); amount-=floorf(amount); break;
			case TYPE_SAT: amount=color.get_s()*2.0f; break;
			case TYPE_A: amount=color.get_a(); break;
			default: amount=0; break;
		}
		x = amount * max_value;
		switch(event->scroll.direction){
			case GDK_SCROLL_UP:
			case GDK_SCROLL_RIGHT:
				x+=1.0f;
				break;
			case GDK_SCROLL_DOWN:
			case GDK_SCROLL_LEFT:
				x-=1.0f;
				break;
			default:
				break;
		}
	} else {
		if (is_horizontal)
			x = float(event->button.x);
		else
			x = float(max_value - event->button.y);
	}

	float pos = x/max_value;
	pos = clamp(pos, 0.0f, 1.0f);

	switch(event->type)
	{
	case GDK_SCROLL:
		signal_slider_moved_(property_type,pos);
		queue_draw();
		signal_activated_();
		return true;

	case GDK_BUTTON_RELEASE:
		signal_activated_();
		return true;

	case GDK_BUTTON_PRESS:
	case GDK_MOTION_NOTIFY:
		signal_slider_moved_(property_type,pos);
		queue_draw();
		return true;
		break;
	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
ColorSlider::get_preferred_height_vfunc(int& minimum_height, int& natural_height) const
{
	minimum_height = default_min_size;
	if (property_orientation == Gtk::ORIENTATION_HORIZONTAL) {
		natural_height = default_min_size;
	} else {
		natural_height = 200;
	}
}

void
ColorSlider::get_preferred_width_vfunc(int& minimum_width, int& natural_width) const
{
	minimum_width = default_min_size;
	if (property_orientation == Gtk::ORIENTATION_HORIZONTAL) {
		natural_width = 200;
	} else {
		natural_width = default_min_size;
	}
}

void
ColorSlider::get_preferred_height_for_width_vfunc(int width, int& minimum_height, int& natural_height) const
{
	minimum_height = default_min_size;
	if (property_orientation == Gtk::ORIENTATION_HORIZONTAL) {
		natural_height = default_min_size;
	} else {
		natural_height = std::min(width * 10, 300);
	}
}

void
ColorSlider::get_preferred_width_for_height_vfunc(int height, int& minimum_width, int& natural_width) const
{
	minimum_width = default_min_size;
	if (property_orientation == Gtk::ORIENTATION_HORIZONTAL) {
		natural_width = std::min(height * 10, 300);
	} else {
		natural_width = default_min_size;
	}
}

// Glade & GtkBuilder related

GType ColorSlider::gtype = 0;

Glib::ObjectBase* ColorSlider::wrap_new(GObject* o)
{
	if (gtk_widget_is_toplevel(GTK_WIDGET(o)))
		return new ColorSlider(GTK_DRAWING_AREA(o));
	else
		return Gtk::manage(new ColorSlider(GTK_DRAWING_AREA(o)));
}

void ColorSlider::register_type()
{
	if (gtype)
		return;

	ColorSlider dummy;

	gtype = G_OBJECT_TYPE(dummy.gobj());

	Glib::wrap_register(gtype, ColorSlider::wrap_new);
}

GType
Glib::Value<studio::ColorSlider::Type>::value_type()
{
	static std::atomic<gsize> type_id(0);

	 if (!type_id) {
		 static const GEnumValue color_components_enum[10] = {
			 {studio::ColorSlider::TYPE_R,   "COLORSLIDER_TYPE_R",   "red"},
			 {studio::ColorSlider::TYPE_G,   "COLORSLIDER_TYPE_G",   "green"},
			 {studio::ColorSlider::TYPE_B,   "COLORSLIDER_TYPE_B",   "blue"},
			 {studio::ColorSlider::TYPE_Y,   "COLORSLIDER_TYPE_Y",   "y"},
			 {studio::ColorSlider::TYPE_U,   "COLORSLIDER_TYPE_U",   "u"},
			 {studio::ColorSlider::TYPE_V,   "COLORSLIDER_TYPE_V",   "v"},
			 {studio::ColorSlider::TYPE_HUE, "COLORSLIDER_TYPE_HUE", "hue"},
			 {studio::ColorSlider::TYPE_SAT, "COLORSLIDER_TYPE_SAT", "sat"},
			 {studio::ColorSlider::TYPE_A,   "COLORSLIDER_TYPE_A",   "a"},
			 {0, nullptr, nullptr}
		 };
		 type_id = g_enum_register_static(g_intern_static_string("SynfigColorSliderType"), color_components_enum);
	 }
	 return type_id;
}
