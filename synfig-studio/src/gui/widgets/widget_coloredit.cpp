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

#include "widgets/widget_coloredit.h"
#include <gui/app.h>
#include <pangomm/attributes.h>
#include <pangomm/attrlist.h>
#include <algorithm>

#include <gtkmm/notebook.h>
#include <gtkmm/box.h>
#include <gtkmm/colorselection.h>
#include <gtkmm/separator.h>
#include <gtkmm/stylecontext.h>

#include <gui/localization.h>

#include <gui/exception_guard.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define ARROW_NEGATIVE_THRESHOLD 0.4

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S ======================================================= */

ColorSlider::ColorSlider(const ColorSlider::Type &x):
	type(x)
{
	set_size_request(-1,16);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
}

void
ColorSlider::set_type(Type x) { type=x; queue_draw(); }

void
ColorSlider::set_color(synfig::Color x) { color_=x; queue_draw(); }

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
	Color dark(0, 0, 0);
	Color light(1, 1, 1);

	// Upper black pointing down arrow
	cr->set_source_rgb(dark.get_r(), dark.get_g(), dark.get_b());
	cr->set_line_width(1.0);
	cr->move_to(x, y);
	cr->line_to(x - 0.5*width, y - height);
	cr->line_to(x + 0.5*width, y - height);
	cr->close_path();
	if (fill)
		cr->fill();
	else
		cr->stroke();

	// Bottom light pointing up arrow
	cr->set_source_rgb(light.get_r(), light.get_g(), light.get_b());
	cr->set_line_width(1.0);
	cr->move_to(x, size - height);
	cr->line_to(x - 0.5*width, size);
	cr->line_to(x + 0.5*width, size);
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

	slider_color_func color_func = jump_table[int(type)];

	Gamma gamma = App::get_selected_canvas_gamma().get_inverted();
	
	float amount;
	switch(type)
	{
		case TYPE_R: amount=color.get_r(); break;
		case TYPE_G: amount=color.get_g(); break;
		case TYPE_B: amount=color.get_b(); break;
		case TYPE_Y: amount=color.get_y(); break;
		case TYPE_U: amount=color.get_u()+0.5; break;
		case TYPE_V: amount=color.get_v()+0.5; break;
		case TYPE_HUE: amount=Angle::rot(color.get_uv_angle()).get(); amount-=floor(amount); break;
		case TYPE_SAT: amount=color.get_s()*2.0; break;
		case TYPE_A: amount=color.get_a(); break;
		default: amount=0; break;
	}
	
	const int height(get_height());
	const int width(get_width());

	Gdk::Rectangle ca(0,0,width,height);

	const Color bg1(0.75, 0.75, 0.75);
	const Color bg2(0.5, 0.5, 0.5);
	for(int i = width-1; i >= 0; --i)
	{
		Color c = color;
		color_func(c, i/float(width));
		
		const Color c1 = gamma.apply(
				Color::blend(c,bg1,1.0).clamped() );
		const Color c2 = gamma.apply(
				Color::blend(c,bg2,1.0).clamped() );
		assert(c1.is_valid());
		assert(c2.is_valid());

		if((i*2/height)&1)
		{
	        cr->set_source_rgb(c1.get_r(), c1.get_g(), c1.get_b());
	        cr->rectangle(ca.get_x()+i, ca.get_y(), 1, height/2);
	        cr->fill();

	        cr->set_source_rgb(c2.get_r(), c2.get_g(), c2.get_b());
	        cr->rectangle(ca.get_x()+i, ca.get_y()+height/2, 1, height/2);
	        cr->fill();
		}
		else
		{
	        cr->set_source_rgb(c2.get_r(), c2.get_g(), c2.get_b());
	        cr->rectangle(ca.get_x()+i, ca.get_y(), 1, height/2);
	        cr->fill();

	        cr->set_source_rgb(c1.get_r(), c1.get_g(), c1.get_b());
	        cr->rectangle(ca.get_x()+i, ca.get_y()+height/2, 1, height/2);
	        cr->fill();
		}
	}

    cr->set_source_rgb(1, 1, 1);
    cr->rectangle(ca.get_x()+1, ca.get_y()+1, width-3, height-3);
    cr->stroke();

    cr->set_source_rgb(0, 0, 0);
    cr->rectangle(ca.get_x(), ca.get_y(), width-1, height-1);
    cr->stroke();

    // Draw face to face contrasted arrows
    draw_arrow(cr, int(amount*width), height/2, height/2, height/2, height, 1);

	return true;
}

bool
ColorSlider::on_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	const int width(get_width());
	float x = 0;
	if( GDK_SCROLL == event->type ){
		Color color(color_);
		float amount;
		switch(type)
		{
			case TYPE_R: amount=color.get_r(); break;
			case TYPE_G: amount=color.get_g(); break;
			case TYPE_B: amount=color.get_b(); break;
			case TYPE_Y: amount=color.get_y(); break;
			case TYPE_U: amount=color.get_u()+0.5; break;
			case TYPE_V: amount=color.get_v()+0.5; break;
			case TYPE_HUE: amount=Angle::rot(color.get_uv_angle()).get(); amount-=floor(amount); break;
			case TYPE_SAT: amount=color.get_s()*2.0; break;
			case TYPE_A: amount=color.get_a(); break;
			default: amount=0; break;
		}
		x = amount*width;
		switch(event->scroll.direction){
			case GDK_SCROLL_UP:
			case GDK_SCROLL_RIGHT:
				x+=1.0;
				break;
			case GDK_SCROLL_DOWN:
			case GDK_SCROLL_LEFT:
				x-=1.0;
				break;
			default:
				break;
		}
	} else {
		x = float(event->button.x);
	}

	float pos(x/width);
	if (pos > 1) pos = 1;
	if (pos < 0 || x <= 0 || event->button.x <= 0) pos=0;

	switch(event->type)
	{
	case GDK_SCROLL:
		signal_slider_moved_(type,pos);
		queue_draw();
		signal_activated_();
		return true;

	case GDK_BUTTON_RELEASE:
		signal_activated_();
		return true;

	case GDK_BUTTON_PRESS:
	case GDK_MOTION_NOTIFY:
		signal_slider_moved_(type,pos);
		queue_draw();
		return true;
		break;
	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

/* === M E T H O D S ======================================================= */
void
Widget_ColorEdit::SliderRow(int left, int top, ColorSlider* color_widget, string l, Gtk::Grid *grid)
{
	auto label = manage(new class Gtk::Label(l));
	label->set_halign(Gtk::ALIGN_START);

	color_widget->set_valign(Gtk::ALIGN_CENTER);
	color_widget->set_hexpand();
	color_widget->set_margin_start(12);
	color_widget->set_margin_bottom(6);
	color_widget->signal_slider_moved().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_slider_moved));
	//color_widget->signal_activated().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::activated));
	color_widget->signal_activated().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));

	grid->attach(*label,        left,   top, 1, 1);
	grid->attach(*color_widget, left+1, top, 1, 1);
}

void
Widget_ColorEdit::AttachSpinButton(int left, int top, Gtk::SpinButton *spin_button, Gtk::Grid *grid)
{
	spin_button->set_margin_start(12);
	spin_button->set_margin_bottom(6);
	spin_button->set_update_policy(Gtk::UPDATE_ALWAYS);
	grid->attach(*spin_button, left, top, 1, 1);
}

Widget_ColorEdit::Widget_ColorEdit():
	R_adjustment(Gtk::Adjustment::create(0,-10000000,10000000,1,10,0)),
	G_adjustment(Gtk::Adjustment::create(0,-10000000,10000000,1,10,0)),
	B_adjustment(Gtk::Adjustment::create(0,-10000000,10000000,1,10,0)),
	A_adjustment(Gtk::Adjustment::create(0,-10000000,10000000,1,10,0)),
	colorHVSChanged(false)
{
	// Set left/right/up/down margin on this widget's content
	auto dialog_context = get_style_context();
	dialog_context->add_class("dialog-main-content");

	notebook=manage(new Gtk::Notebook);
	notebook->set_vexpand();

	auto rgb_grid  (manage(new Gtk::Grid));
	auto yuv_grid  (manage(new Gtk::Grid));
	auto hvs_grid  (manage(new Gtk::Grid));
	auto alpha_grid(manage(new Gtk::Grid));

	auto rgb_context = rgb_grid->get_style_context();
	auto yuv_context = yuv_grid->get_style_context();
	auto hvs_context = hvs_grid->get_style_context();
	rgb_context->add_class("color-grid");
	yuv_context->add_class("color-grid");
	hvs_context->add_class("color-grid");

	{
		auto rgb_box(manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL)));
		auto yuv_box(manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL)));
		auto hvs_box(manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL)));
		rgb_box->add(*rgb_grid);
		yuv_box->add(*yuv_grid);
		hvs_box->add(*hvs_grid);
		notebook->append_page(*rgb_box,_("RGB"));
		notebook->append_page(*yuv_box,_("YUV"));
		notebook->append_page(*hvs_box,_("HSV"));
	}

	color=Color(0,0,0,0);

	set_size_request(200,-1);
	hold_signals=true;
	clamp_=true;

	widget_color.set_size_request(-1, 32);
	attach(widget_color, 0, 0, 1, 1);
	attach(*notebook,    0, 1, 1, 1);
	attach(*alpha_grid,  0, 2, 1, 1);

	//This defines are used for code below simplification.
	#define SLIDER_ROW(left,top,n,l) SliderRow(left, top, slider_##n = manage(new ColorSlider(ColorSlider::TYPE_##n)), l, grid);
	#define ATTACH_SPIN_BUTTON(left,top,n) AttachSpinButton(left, top, spinbutton_##n = manage(new class Gtk::SpinButton(n##_adjustment, 1, 0)),grid);

	{ //RGB frame
		auto grid(rgb_grid);
		SLIDER_ROW(0, 0, R, _("Red"));
		SLIDER_ROW(0, 1, G, _("Green"));
		SLIDER_ROW(0, 2, B, _("Blue"));
		ATTACH_SPIN_BUTTON(2, 0, R);
		ATTACH_SPIN_BUTTON(2, 1, G);
		ATTACH_SPIN_BUTTON(2, 2, B);

		auto separator = manage(new Gtk::Separator);
		grid->attach(*separator, 0, 3, 3, 1);

		hex_color_label = manage(new Gtk::Label("HTML code"));
		hex_color_label->set_halign(Gtk::ALIGN_START);
		grid->attach(*hex_color_label, 0, 4, 1, 1);

		hex_color = manage(new Gtk::Entry());
		hex_color->set_halign(Gtk::ALIGN_START);
		hex_color->set_width_chars(16);
		hex_color->set_margin_start(12);
		hex_color->signal_activate().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_hex_edited));
		hex_color->signal_focus_out_event().connect(sigc::mem_fun(*this, &studio::Widget_ColorEdit::on_hex_focus_out));
		grid->attach(*hex_color, 1, 4, 1, 1);
	}
	{ //YUM frame
		auto grid(yuv_grid);
		grid->set_row_spacing(16);
		SLIDER_ROW(0, 0, Y,   _("Luma"));
		SLIDER_ROW(0, 1, HUE, _("Hue"));
		SLIDER_ROW(0, 2, SAT, _("Saturation"));
		SLIDER_ROW(0, 3, U,   _("U"));
		SLIDER_ROW(0, 4, V,   _("V"));
	}
	{ //HVS frame
		//I use Gtk::ColorSelection widget here.
		hvsColorWidget = manage(new Gtk::ColorSelection());
		setHVSColor(get_value());
		hvsColorWidget->signal_color_changed().connect(sigc::mem_fun(*this, &studio::Widget_ColorEdit::on_color_changed));
		//TODO: Anybody knows how to set min size for this widget? I've tried use set_size_request(..). But it doesn't works.
		hvs_grid->attach(*(hvsColorWidget), 0, 4, 1, 1);
	}
	{
		auto grid(alpha_grid);
		grid->set_margin_top(6);
		SLIDER_ROW(0, 0, A, _("Alpha"));
		ATTACH_SPIN_BUTTON(2, 0, A);
	}

#undef SLIDER_ROW
#undef ATTACH_SPIN_BUTTON

	spinbutton_R->signal_activate().connect(sigc::mem_fun(*spinbutton_G,&Gtk::SpinButton::grab_focus));
	spinbutton_G->signal_activate().connect(sigc::mem_fun(*spinbutton_B,&Gtk::SpinButton::grab_focus));
	spinbutton_B->signal_activate().connect(sigc::mem_fun(*spinbutton_A,&Gtk::SpinButton::grab_focus));
	spinbutton_A->signal_activate().connect(sigc::mem_fun(*spinbutton_R,&Gtk::SpinButton::grab_focus));

	R_adjustment->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));
	G_adjustment->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));
	B_adjustment->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));
	A_adjustment->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));

	show_all_children();

	set_digits(1);
	set_value(color);

	hold_signals=false;
	notebook->set_current_page(2);

}

Widget_ColorEdit::~Widget_ColorEdit()
{
}

#define CLIP_VALUE(value, min, max) (value <= min ? min : (value > max ? max : value))

bool are_close_colors(Gdk::Color const& a, Gdk::Color const& b) {
	static const int eps = 1;
	if (a == b)
		return true;
	return std::abs(a.get_red()-b.get_red()) <= eps
		&& std::abs(a.get_green()-b.get_green()) <= eps
		&& std::abs(a.get_blue()-b.get_blue()) <= eps;
}

void Widget_ColorEdit::setHVSColor(synfig::Color color)
{
	Color c = App::get_selected_canvas_gamma().get_inverted().apply(color).clamped();
	Gdk::Color gtkColor;
	gtkColor.set_rgb_p(c.get_r(), c.get_g(), c.get_b());
	if (!are_close_colors(hvsColorWidget->get_current_color(), gtkColor)) {
		colorHVSChanged = true;
		hvsColorWidget->set_current_color(gtkColor);
	}
	hvsColorWidget->set_previous_color(hvsColorWidget->get_current_color()); //We can't use it there, cause color changes in realtime.
	colorHVSChanged = false;
}

void
Widget_ColorEdit::on_color_changed()
{
	//Spike! Gtk::ColorSelection emits this signal when I use
	//set_current_color(...). It calls recursion. Used a flag to fix it.
	if (!colorHVSChanged)
	{
		Gdk::Color newColor = hvsColorWidget->get_current_color();
		Color synfigColor(
			newColor.get_red_p(),
			newColor.get_green_p(),
			newColor.get_blue_p() );
		synfigColor = App::get_selected_canvas_gamma().apply(synfigColor);
		set_value(synfigColor);
		colorHVSChanged = true; //I reset the flag in setHVSColor(..)
		on_value_changed();
	}
}

void
Widget_ColorEdit::on_slider_moved(ColorSlider::Type type, float amount)
{
	Color color(get_value_raw());

	assert(color.is_valid());
	ColorSlider::adjust_color(type,color,amount);
	assert(color.is_valid());

	// If a non-primary colorslider is adjusted,
	// we want to make sure that we clamp
	//if(type>ColorSlider::TYPE_B && (color.get_r()<0 ||color.get_g()<0 ||color.get_b()<0))
	//	clamp_=true;

	/*
	if(type==ColorSlider::TYPE_R && color.get_r()<0)clamp_=false;
	if(type==ColorSlider::TYPE_G && color.get_g()<0)clamp_=false;
	if(type==ColorSlider::TYPE_B && color.get_b()<0)clamp_=false;
	*/
	clamp_=false;

	set_value(color);
	assert(color.is_valid());
}

void
Widget_ColorEdit::on_hex_edited()
{
	Color color(get_value_raw());
	String s = hex_color->get_text();
	color.set_hex(s);
	set_value(color);
	signal_value_changed_();
}

bool
Widget_ColorEdit::on_hex_focus_out(GdkEventFocus* /*event*/)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	on_hex_edited();
	return true;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
Widget_ColorEdit::on_value_changed()
{
	if(hold_signals)
		return;

	const Color color(get_value_raw());
	assert(color.is_valid());
	setHVSColor(color);
	slider_R->set_color(color);
	slider_G->set_color(color);
	slider_B->set_color(color);
	slider_Y->set_color(color);
	slider_U->set_color(color);
	slider_V->set_color(color);
	slider_HUE->set_color(color);
	slider_SAT->set_color(color);
	slider_A->set_color(color);
	hex_color->set_text(color.get_hex());
	widget_color.set_value(color);

	activate();
	signal_value_changed_();
}

void
Widget_ColorEdit::set_has_frame(bool x)
{
	spinbutton_R->set_has_frame(x);
	spinbutton_G->set_has_frame(x);
	spinbutton_B->set_has_frame(x);
	spinbutton_A->set_has_frame(x);
}

void
Widget_ColorEdit::set_digits(int x)
{
	spinbutton_R->set_digits(x);
	spinbutton_G->set_digits(x);
	spinbutton_B->set_digits(x);
	spinbutton_A->set_digits(x);
}

void
Widget_ColorEdit::set_value(const synfig::Color &data)
{
	assert(data.is_valid());
	hold_signals=true;
	clamp_=false;

	color=data;

	R_adjustment->set_value(color.get_r()*100);
	G_adjustment->set_value(color.get_g()*100);
	B_adjustment->set_value(color.get_b()*100);
	A_adjustment->set_value(color.get_a()*100);

	slider_R->set_color(color);
	slider_G->set_color(color);
	slider_B->set_color(color);
	slider_Y->set_color(color);
	slider_U->set_color(color);
	slider_V->set_color(color);
	slider_HUE->set_color(color);
	slider_SAT->set_color(color);
	slider_A->set_color(color);
	hex_color->set_text(color.get_hex());
	widget_color.set_value(color);
	setHVSColor(color);

	hold_signals=false;
}

synfig::Color
Widget_ColorEdit::get_value_raw()
{
	Color color;
	color.set_r(R_adjustment->get_value()/100);
	color.set_g(G_adjustment->get_value()/100);
	color.set_b(B_adjustment->get_value()/100);
	color.set_a(A_adjustment->get_value()/100);
	assert(color.is_valid());

	return color;
}

const synfig::Color &
Widget_ColorEdit::get_value()
{
	color.set_r(R_adjustment->get_value()/100);
	color.set_g(G_adjustment->get_value()/100);
	color.set_b(B_adjustment->get_value()/100);
	color.set_a(A_adjustment->get_value()/100);
	assert(color.is_valid());

	if(notebook->get_current_page()!=0)
		color=color.clamped();

	/*{
		// Clamp out negative values
		color.set_r(std::max(0.0f,(float)color.get_r()));
		color.set_g(std::max(0.0f,(float)color.get_g()));
		color.set_b(std::max(0.0f,(float)color.get_b()));
	}*/

	return color;
}
