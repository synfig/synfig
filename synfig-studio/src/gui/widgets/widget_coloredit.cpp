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
#include <cmath>
#include "app.h"
#include <gtkmm/drawingarea.h>
#include <pangomm/attributes.h>
#include <pangomm/attrlist.h>
#include <algorithm>
#include <gtkmm/notebook.h>
#include <gtkmm/box.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define SPINBUTTON_WIDTH 100
#define ARROW_NEGATIVE_THRESHOLD 0.4

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S ======================================================= */

ColorSlider::ColorSlider(const ColorSlider::Type &x):
	type(x)
{
	signal_expose_event().connect(sigc::mem_fun(*this, &ColorSlider::redraw));
	set_size_request(-1,12);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	add_events(Gdk::BUTTON1_MOTION_MASK);
}

void
ColorSlider::set_type(Type x) { type=x; queue_draw(); }

void
ColorSlider::set_color(synfig::Color x) { orig_color=x; color_=x; queue_draw(); }

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

bool
ColorSlider::redraw(GdkEventExpose */*bleh*/)
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(!window) return false;

	Color color(color_);

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

	slider_color_func color_func(jump_table[int(type)]);

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
	if(use_colorspace_gamma() && (type<TYPE_U))
		amount=gamma_in(amount);

	const int height(get_height());
	const int width(get_width());

	Gdk::Rectangle ca(0,0,width,height);

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));
	const Color bg1(0.75, 0.75, 0.75);
	const Color bg2(0.5, 0.5, 0.5);
	int i;
	for(i=width-1;i>=0;i--)
	{
		color_func(color,
				   (use_colorspace_gamma() && type<TYPE_U)
				   ? gamma_out(float(i)/float(width))
				   :		  (float(i)/float(width)));
		const Color c1(Color::blend(color,bg1,1.0).clamped());
		const Color c2(Color::blend(color,bg2,1.0).clamped());
		assert(c1.is_valid());
		assert(c2.is_valid());

		if((i*2/height)&1)
		{
			gc->set_rgb_fg_color(colorconv_synfig2gdk(c1));
			window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y(), 1, height/2);

			gc->set_rgb_fg_color(colorconv_synfig2gdk(c2));
			window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y()+height/2, 1, height/2);
		}
		else
		{
			gc->set_rgb_fg_color(colorconv_synfig2gdk(c2));
			window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y(), 1, height/2);

			gc->set_rgb_fg_color(colorconv_synfig2gdk(c1));
			window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y()+height/2, 1, height/2);
		}
	}

	get_style()->paint_arrow(
		window,
		(orig_color.get_y()<ARROW_NEGATIVE_THRESHOLD)?Gtk::STATE_SELECTED:Gtk::STATE_ACTIVE,
		//use light arrow on dark color, and dark arrow on light color , TODO: detect from style which is darkest from SELECTED or ACTIVE
		Gtk::SHADOW_OUT,
		ca,
		*this,
		" ",
		Gtk::ARROW_UP,
		1,
		int(amount*width)-height/2,
		0,
		height,
		height
	);

	gc->set_rgb_fg_color(Gdk::Color("#ffffff"));
	window->draw_rectangle(gc, false, ca.get_x()+1, ca.get_y()+1, width-3, height-3);
	gc->set_rgb_fg_color(Gdk::Color("#000000"));
	window->draw_rectangle(gc, false, ca.get_x(), ca.get_y(), width-1, height-1);
	return true;
}

bool
ColorSlider::on_event(GdkEvent *event)
{
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
		if(use_colorspace_gamma() && (type<TYPE_U))
			amount=gamma_in(amount);
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
		}
	} else {
		x = float(event->button.x);
	}

	float pos(x/width);
	if(pos<0 || x<=0)pos=0;
	if(pos>1)pos=1;

	if(use_colorspace_gamma() && (type<TYPE_U))
		pos=gamma_out(pos);
	if(pos<0 || event->button.x<=0)pos=0;
	if(pos>1)pos=1;

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
//		adjust_color(type,color_,pos);
		signal_slider_moved_(type,pos);
		queue_draw();
		return true;
		break;
	default:
		break;
	}
	return false;
}

/* === M E T H O D S ======================================================= */

Widget_ColorEdit::Widget_ColorEdit():
	R_adjustment(0,-10000000,10000000,1,10,0),
	G_adjustment(0,-10000000,10000000,1,10,0),
	B_adjustment(0,-10000000,10000000,1,10,0),
	A_adjustment(0,-10000000,10000000,1,10,0)
{
	notebook=manage(new Gtk::Notebook);

	Gtk::Table* rgb_table(manage(new Gtk::Table()));
	Gtk::Table* yuv_table(manage(new Gtk::Table()));
	Gtk::Table* main_table(this);

	{
		Gtk::VBox* rgb_box(manage(new Gtk::VBox()));
		Gtk::VBox* yuv_box(manage(new Gtk::VBox()));
		rgb_box->pack_start(*rgb_table,false,false);
		yuv_box->pack_start(*yuv_table,false,false);
		notebook->append_page(*rgb_box,_("RGB"));
		notebook->append_page(*yuv_box,_("YUV"));
	}

	color=Color(0,0,0,0);

	set_size_request(200,-1);
	hold_signals=true;

	Gtk::Label *label;

	R_adjustment.set_lower(-10000000);
	G_adjustment.set_lower(-10000000);
	B_adjustment.set_lower(-10000000);
	A_adjustment.set_lower(-10000000);

	clamp_=true;

	Pango::AttrList attr_list;
	Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*7));
	pango_size.set_start_index(0);
	pango_size.set_end_index(64);
	attr_list.change(pango_size);

	widget_color.set_size_request(-1,16);
	attach(widget_color, 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);
	attach(*notebook, 0, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

#define SLIDER_ROW(i,n,l) \
	slider_##n=manage(new ColorSlider(ColorSlider::TYPE_##n));	\
	slider_##n->signal_slider_moved().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_slider_moved)); \
	/*slider_##n->signal_activated().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::activated));*/ \
	slider_##n->signal_activated().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed)); \
	label=manage(new class Gtk::Label(l,0.0,0.5)); \
	label->set_use_markup(false); \
	label->set_use_underline(false); \
	label->set_attributes(attr_list); \
	table->attach(*label, 0, 1, 1+2*i, 2+2*i, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	\
	table->attach(*slider_##n, 0, 1, 2+2*i, 3+2*i, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0)

#define ATTACH_SPIN_BUTTON(i,n) \
	spinbutton_##n=manage(new class Gtk::SpinButton(n##_adjustment,1,0)); \
	spinbutton_##n->set_update_policy(Gtk::UPDATE_ALWAYS); \
	spinbutton_##n->set_size_request(SPINBUTTON_WIDTH,-1); \
	spinbutton_##n->show(); \
	table->attach(*spinbutton_##n, 1, 2, 1+2*i, 3+2*i, Gtk::SHRINK, Gtk::EXPAND, 2, 0)

	{
		Gtk::Table* table(rgb_table);
		SLIDER_ROW(0,R,_("Red"));
		ATTACH_SPIN_BUTTON(0,R);
		SLIDER_ROW(1,G,_("Green"));
		ATTACH_SPIN_BUTTON(1,G);
		SLIDER_ROW(2,B,_("Blue"));
		ATTACH_SPIN_BUTTON(2,B);

		hex_color_label = manage(new Gtk::Label(_("HTML code"), 0.0, 0.5));
		hex_color_label->set_use_markup(false);
		hex_color_label->set_use_underline(false);
		hex_color_label->set_attributes(attr_list);
		table->attach(*hex_color_label, 0, 1, 7, 8, Gtk::SHRINK, Gtk::SHRINK, 0, 0);

		hex_color = manage(new Gtk::Entry());
		hex_color->set_width_chars(8);
		hex_color->signal_activate().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_hex_edited));
		table->attach(*hex_color, 0, 1, 8, 9, Gtk::SHRINK, Gtk::SHRINK, 0, 0);
	}
	{
		Gtk::Table* table(yuv_table);
		SLIDER_ROW(0,Y,_("Luma"));
		SLIDER_ROW(1,HUE,_("Hue"));
		SLIDER_ROW(2,SAT,_("Saturation"));
		SLIDER_ROW(3,U,_("U"));
		SLIDER_ROW(4,V,_("V"));
	}
	{
		Gtk::Table* table(main_table);
		SLIDER_ROW(1,A,_("Alpha"));
		ATTACH_SPIN_BUTTON(1,A);
	}

#undef SLIDER_ROW
#undef ATTACH_SPIN_BUTTON

	spinbutton_R->signal_activate().connect(sigc::mem_fun(*spinbutton_G,&Gtk::SpinButton::grab_focus));
	spinbutton_G->signal_activate().connect(sigc::mem_fun(*spinbutton_B,&Gtk::SpinButton::grab_focus));
	spinbutton_B->signal_activate().connect(sigc::mem_fun(*spinbutton_A,&Gtk::SpinButton::grab_focus));
	spinbutton_A->signal_activate().connect(sigc::mem_fun(*spinbutton_R,&Gtk::SpinButton::grab_focus));

	R_adjustment.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));
	G_adjustment.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));
	B_adjustment.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));
	A_adjustment.signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_ColorEdit::on_value_changed));

	show_all_children();

	set_digits(1);
	set_value(color);

	hold_signals=false;
}

Widget_ColorEdit::~Widget_ColorEdit()
{
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
	if(type>ColorSlider::TYPE_B && (color.get_r()<0 ||color.get_g()<0 ||color.get_b()<0))
		clamp_=true;

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

void
Widget_ColorEdit::on_value_changed()
{
	if(hold_signals)
		return;

	const Color color(get_value_raw());
	assert(color.is_valid());
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
	spinbutton_R->set_size_request(SPINBUTTON_WIDTH,-1);
	spinbutton_G->set_size_request(SPINBUTTON_WIDTH,-1);
	spinbutton_B->set_size_request(SPINBUTTON_WIDTH,-1);
	spinbutton_A->set_size_request(SPINBUTTON_WIDTH,-1);
}

void
Widget_ColorEdit::set_digits(int x)
{
	spinbutton_R->set_digits(x);
	spinbutton_G->set_digits(x);
	spinbutton_B->set_digits(x);
	spinbutton_A->set_digits(x);
	spinbutton_R->set_size_request(SPINBUTTON_WIDTH,-1);
	spinbutton_G->set_size_request(SPINBUTTON_WIDTH,-1);
	spinbutton_B->set_size_request(SPINBUTTON_WIDTH,-1);
	spinbutton_A->set_size_request(SPINBUTTON_WIDTH,-1);
}

void
Widget_ColorEdit::set_value(const synfig::Color &data)
{
	assert(data.is_valid());
	hold_signals=true;
	clamp_=false;

	color=data;

	if(use_colorspace_gamma())
	{
		R_adjustment.set_value(gamma_in(color.get_r())*100);
		G_adjustment.set_value(gamma_in(color.get_g())*100);
		B_adjustment.set_value(gamma_in(color.get_b())*100);
	}
	else
	{
		R_adjustment.set_value(color.get_r()*100);
		G_adjustment.set_value(color.get_g()*100);
		B_adjustment.set_value(color.get_b()*100);
	}
	A_adjustment.set_value(color.get_a()*100);

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

	hold_signals=false;
}

synfig::Color
Widget_ColorEdit::get_value_raw()
{
	Color color;
	if(use_colorspace_gamma())
	{
		color.set_r(gamma_out(R_adjustment.get_value()/100.0f));
		color.set_g(gamma_out(G_adjustment.get_value()/100.0f));
		color.set_b(gamma_out(B_adjustment.get_value()/100.0f));
	}
	else
	{
		color.set_r(R_adjustment.get_value()/100);
		color.set_g(G_adjustment.get_value()/100);
		color.set_b(B_adjustment.get_value()/100);
	}
	color.set_a(A_adjustment.get_value()/100);
	assert(color.is_valid());

	return color;
}

const synfig::Color &
Widget_ColorEdit::get_value()
{
	if(use_colorspace_gamma())
	{
		color.set_r(gamma_out(R_adjustment.get_value()/100.0f));
		color.set_g(gamma_out(G_adjustment.get_value()/100.0f));
		color.set_b(gamma_out(B_adjustment.get_value()/100.0f));
		assert(color.is_valid());
	}
	else
	{
		color.set_r(R_adjustment.get_value()/100);
		color.set_g(G_adjustment.get_value()/100);
		color.set_b(B_adjustment.get_value()/100);
		assert(color.is_valid());
	}
	color.set_a(A_adjustment.get_value()/100);
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
