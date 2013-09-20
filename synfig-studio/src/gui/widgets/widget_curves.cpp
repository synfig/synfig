/* === S Y N F I G ========================================================= */
/*!	\file widget_curves.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2008 Gerco Ballintijn
**  Copyright (c) 2011 Carlos López
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

#include "widgets/widget_curves.h"
#include <cmath>
#include "app.h"
#include <gtkmm/drawingarea.h>
#include <map>
#include <vector>
#include <ETL/misc>
#include <sigc++/object.h>

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

/*
void
studio::render_color_to_window(const Glib::RefPtr<Gdk::Drawable>& window,const Gdk::Rectangle& ca,const synfig::Color &color)
{
	const int height(ca.get_height());
	const int width(ca.get_width());

	const int square_size(height/2);

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));

	if(color.get_alpha()!=1.0)
	{
		// In this case we need to render the alpha squares

		const Color bg1(Color::blend(color,Color(0.75, 0.75, 0.75),1.0).clamped());
		const Color bg2(Color::blend(color,Color(0.5, 0.5, 0.5),1.0).clamped());

		Gdk::Color gdk_c1(colorconv_synfig2gdk(bg1));
		Gdk::Color gdk_c2(colorconv_synfig2gdk(bg2));

		bool toggle(false);
		for(int i=0;i<width;i+=square_size)
		{
			const int square_width(min(square_size,width-i));

			if(toggle)
			{
				gc->set_rgb_fg_color(gdk_c1);
				window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y(), square_width, square_size);

				gc->set_rgb_fg_color(gdk_c2);
				window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y()+square_size, square_width, square_size);
				toggle=false;
			}
			else
			{
				gc->set_rgb_fg_color(gdk_c2);
				window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y(), square_width, square_size);

				gc->set_rgb_fg_color(gdk_c1);
				window->draw_rectangle(gc, true, ca.get_x()+i, ca.get_y()+square_size, square_width, square_size);
				toggle=true;
			}
		}
	}
	else
	{
		// In this case we have a solid color to use
		Gdk::Color gdk_c1(colorconv_synfig2gdk(color));

		gc->set_rgb_fg_color(gdk_c1);
		window->draw_rectangle(gc, true, ca.get_x(), ca.get_y(), width-1, height-1);
	}
	gc->set_rgb_fg_color(Gdk::Color("#ffffff"));
	window->draw_rectangle(gc, false, ca.get_x()+1, ca.get_y()+1, width-3, height-3);
	gc->set_rgb_fg_color(Gdk::Color("#000000"));
	window->draw_rectangle(gc, false, ca.get_x(), ca.get_y(), width-1, height-1);
}
*/

/* === C L A S S E S ======================================================= */



struct studio::Widget_Curves::Channel
{
	synfig::String name;
	Gdk::Color color;
	std::map<synfig::Real,synfig::Real> values;
};

struct studio::Widget_Curves::CurveStruct : sigc::trackable
{
	synfigapp::ValueDesc value_desc;
	std::vector<Channel> channels;

	CurveStruct(const synfigapp::ValueDesc& x):
		value_desc(x)
	{
		switch(value_desc.get_value_type())
		{
			case ValueBase::TYPE_REAL:
				channels.push_back(Channel());
				channels.back().name="real";
				channels.back().color=Gdk::Color("#007f7f");
				break;
			case ValueBase::TYPE_TIME:
				channels.push_back(Channel());
				channels.back().name="time";
				channels.back().color=Gdk::Color("#7f7f00");
				break;
			case ValueBase::TYPE_INTEGER:
				channels.push_back(Channel());
				channels.back().name="int";
				channels.back().color=Gdk::Color("#7f0000");
				break;
			case ValueBase::TYPE_BOOL:
				channels.push_back(Channel());
				channels.back().name="bool";
				channels.back().color=Gdk::Color("#ff7f00");
				break;
			case ValueBase::TYPE_ANGLE:
				channels.push_back(Channel());
				channels.back().name="theta";
				channels.back().color=Gdk::Color("#004f4f");
				break;
			case ValueBase::TYPE_COLOR:
				channels.push_back(Channel());
				channels.back().name="red";
				channels.back().color=Gdk::Color("#7f0000");
				channels.push_back(Channel());
				channels.back().name="green";
				channels.back().color=Gdk::Color("#007f00");
				channels.push_back(Channel());
				channels.back().name="blue";
				channels.back().color=Gdk::Color("#00007f");
				channels.push_back(Channel());
				channels.back().name="alpha";
				channels.back().color=Gdk::Color("#000000");
				break;
			case ValueBase::TYPE_VECTOR:
				channels.push_back(Channel());
				channels.back().name="x";
				channels.back().color=Gdk::Color("#7f007f");
				channels.push_back(Channel());
				channels.back().name="y";
				channels.back().color=Gdk::Color("#007f7f");
				break;
			case ValueBase::TYPE_BLINEPOINT:
				channels.push_back(Channel());
				channels.back().name="v.x";
				channels.back().color=Gdk::Color("#ff7f00");
				channels.push_back(Channel());
				channels.back().name="v.y";
				channels.back().color=Gdk::Color("#7f3f00");

				channels.push_back(Channel());
				channels.back().name="width";
				channels.back().color=Gdk::Color("#000000");

				channels.push_back(Channel());
				channels.back().name="origin";
				channels.back().color=Gdk::Color("#ffffff");

				channels.push_back(Channel());
				channels.back().name="tsplit";
				channels.back().color=Gdk::Color("#ff00ff");

				channels.push_back(Channel());
				channels.back().name="t1.x";
				channels.back().color=Gdk::Color("#ff0000");
				channels.push_back(Channel());
				channels.back().name="t1.y";
				channels.back().color=Gdk::Color("#7f0000");

				channels.push_back(Channel());
				channels.back().name="t2.x";
				channels.back().color=Gdk::Color("#ffff00");
				channels.push_back(Channel());
				channels.back().name="t2.y";
				channels.back().color=Gdk::Color("#7f7f00");

				channels.push_back(Channel());
				channels.back().name="rsplit";
				channels.back().color=Gdk::Color("#ff00ff");
				channels.push_back(Channel());
				channels.back().name="asplit";
				channels.back().color=Gdk::Color("#ff00ff");
				break;
			case ValueBase::TYPE_WIDTHPOINT:
				channels.push_back(Channel());
				channels.back().name="position";
				channels.back().color=Gdk::Color("#ff0000");
				channels.push_back(Channel());
				channels.back().name="width";
				channels.back().color=Gdk::Color("#00ff00");
				break;
			case ValueBase::TYPE_DASHITEM:
				channels.push_back(Channel());
				channels.back().name="offset";
				channels.back().color=Gdk::Color("#ff0000");
				channels.push_back(Channel());
				channels.back().name="length";
				channels.back().color=Gdk::Color("#00ff00");
				break;
			default:
				throw synfig::Exception::BadType("Bad type for curves");
		}
	}

	void clear_all_values()
	{
		std::vector<Channel>::iterator iter;
		for(iter=channels.begin();iter!=channels.end();++iter)
			iter->values.clear();
	}

	synfig::Real get_value(int chan, synfig::Real time, synfig::Real tolerance)
	{
		std::map<synfig::Real,synfig::Real>::iterator iter;

		// First check to see if we have a value
		// that is "close enough" to the time
		// we are looking for
		iter=channels[chan].values.lower_bound(time);
		if(iter!=channels[chan].values.end() && iter->first-time<=tolerance)
			return -iter->second;

		// Since that didn't work, we now need
		// to go ahead and figure out what the
		// actual value is at that time.
		ValueBase value(value_desc.get_value(time));
		switch(value.get_type())
		{
			case ValueBase::TYPE_REAL:
				channels[0].values[time]=value.get(Real());
				break;
			case ValueBase::TYPE_TIME:
				channels[0].values[time]=value.get(Time());
				break;
			case ValueBase::TYPE_INTEGER:
				channels[0].values[time]=value.get(int());
				break;
			case ValueBase::TYPE_BOOL:
				channels[0].values[time]=value.get(bool());
				break;
			case ValueBase::TYPE_ANGLE:
				channels[0].values[time]=Angle::rad(value.get(Angle())).get();
				break;
			case ValueBase::TYPE_COLOR:
				channels[0].values[time]=value.get(Color()).get_r();
				channels[1].values[time]=value.get(Color()).get_g();
				channels[2].values[time]=value.get(Color()).get_b();
				channels[3].values[time]=value.get(Color()).get_a();
				break;
			case ValueBase::TYPE_VECTOR:
				channels[0].values[time]=value.get(Vector())[0];
				channels[1].values[time]=value.get(Vector())[1];
				break;
			case ValueBase::TYPE_BLINEPOINT:
				channels[0].values[time]=value.get(BLinePoint()).get_vertex()[0];
				channels[1].values[time]=value.get(BLinePoint()).get_vertex()[1];
				channels[2].values[time]=value.get(BLinePoint()).get_width();
				channels[3].values[time]=value.get(BLinePoint()).get_origin();
				channels[4].values[time]=value.get(BLinePoint()).get_split_tangent_both();
				channels[5].values[time]=value.get(BLinePoint()).get_tangent1()[0];
				channels[6].values[time]=value.get(BLinePoint()).get_tangent1()[1];
				channels[7].values[time]=value.get(BLinePoint()).get_tangent2()[0];
				channels[8].values[time]=value.get(BLinePoint()).get_tangent2()[1];
				channels[9].values[time]=value.get(BLinePoint()).get_split_tangent_radius();
				channels[10].values[time]=value.get(BLinePoint()).get_split_tangent_angle();
				break;
			case ValueBase::TYPE_WIDTHPOINT:
				channels[0].values[time]=value.get(WidthPoint()).get_position();
				channels[1].values[time]=value.get(WidthPoint()).get_width();
				break;
			default:
				return 0;
		}

		return -channels[chan].values[time];
	}

	static bool is_not_supported(const synfigapp::ValueDesc& x)
	{
		return x.get_value_type() == ValueBase::TYPE_STRING
			|| x.get_value_type() == ValueBase::TYPE_CANVAS
			|| x.get_value_type() == ValueBase::TYPE_GRADIENT
			|| x.get_value_type() == ValueBase::TYPE_LIST
			|| x.get_value_type() == ValueBase::TYPE_SEGMENT;
	}
};

/* === M E T H O D S ======================================================= */

Widget_Curves::Widget_Curves():
	range_adjustment_(new Gtk::Adjustment(-1,-2,2,0.1,0.1,2))
{
	set_size_request(64,64);

	range_adjustment_->signal_changed().connect(
		sigc::mem_fun(
			*this,
			&Widget_Curves::queue_draw
		)
	);
	range_adjustment_->signal_value_changed().connect(
		sigc::mem_fun(
			*this,
			&Widget_Curves::queue_draw
		)
	);
	//set_vadjustment(*range_adjustment_);

	signal_expose_event().connect(sigc::mem_fun(*this, &studio::Widget_Curves::redraw));
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

}

Widget_Curves::~Widget_Curves()
{
}

void
Widget_Curves::set_time_adjustment(Gtk::Adjustment&x)
{
	time_adjustment_=&x;
	time_adjustment_->signal_changed().connect(
		sigc::mem_fun(
			*this,
			&Widget_Curves::queue_draw
		)
	);
	time_adjustment_->signal_value_changed().connect(
		sigc::mem_fun(
			*this,
			&Widget_Curves::queue_draw
		)
	);
	//set_hadjustment(*time_adjustment_);
}

void
Widget_Curves::clear()
{
	curve_list_.clear();
}

void
Widget_Curves::refresh()
{
	std::list<CurveStruct>::iterator curve_iter;
	for(curve_iter=curve_list_.begin();curve_iter!=curve_list_.end();++curve_iter)
	{
		curve_iter->clear_all_values();
	}
	queue_draw();
}

void
Widget_Curves::set_value_descs(std::list<synfigapp::ValueDesc> value_descs)
{
	curve_list_.clear();

	std::list<synfigapp::ValueDesc>::iterator iter;
	for(iter=value_descs.begin();iter!=value_descs.end();++iter)
	{
		if (CurveStruct::is_not_supported(*iter))
	        	continue;

		try {
			curve_list_.push_back(*iter);
			if(iter->is_value_node())
			{
				iter->get_value_node()->signal_changed().connect(
					sigc::mem_fun(
						*this,
						&studio::Widget_Curves::refresh
					)
				);
			}
			if(iter->parent_is_value_node())
			{
				iter->get_parent_value_node()->signal_changed().connect(
					sigc::mem_fun(
						*this,
						&studio::Widget_Curves::refresh
					)
				);
			}
			if(iter->parent_is_layer_param())
			{
				iter->get_layer()->signal_changed().connect(
					sigc::mem_fun(
						*this,
						&studio::Widget_Curves::refresh
					)
				);
			}
		}catch(synfig::Exception::BadType)
		{
			continue;
		}
	}
	queue_draw();
}

bool
Widget_Curves::on_event(GdkEvent *event)
{
	switch(event->type)
	{
	case GDK_SCROLL:
		switch(event->scroll.direction)
		{
			case GDK_SCROLL_UP:
				range_adjustment_->set_page_size(range_adjustment_->get_page_size()/1.25);
				range_adjustment_->changed();
				break;
			case GDK_SCROLL_DOWN:
				range_adjustment_->set_page_size(range_adjustment_->get_page_size()*1.25);
				range_adjustment_->changed();
				break;
			default:
				break;
		}
		break;
	default:
		return Gtk::DrawingArea::on_event(event);
		break;
	}

	return true;

/*	switch(event->type)
	{
	case GDK_BUTTON_PRESS:
		if(event->button.button==1)
		{
			signal_activate_();
			return true;
		}
		if(event->button.button==3)
		{
			signal_secondary_();
			return true;
		}
		break;

	default:
		break;
	}
	return false;
*/
}

bool
Widget_Curves::redraw(GdkEventExpose */*bleh*/)
{
	//!Check if the window we want draw is ready
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(!window) return true;

	const int h(get_height());
	const int w(get_width());
	window->clear();

	if(!time_adjustment_ || !range_adjustment_ || !h || !w)
		return false;

	if(!curve_list_.size())
		return false;

	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));

	const Real t_begin(time_adjustment_->get_lower());
	const Real t_end(time_adjustment_->get_upper());
	const Real dt((t_end-t_begin)/w);

	const Real r_bottom(range_adjustment_->get_value());
	const Real r_top(r_bottom+range_adjustment_->get_page_size());
	const Real dr((r_top-r_bottom)/h);
	Real r_max(-100000000);
	Real r_min(100000000);

	std::list<CurveStruct>::iterator curve_iter;
	//Figure out maximun number of channels
	int min_channels(100);
	for(curve_iter=curve_list_.begin();curve_iter!=curve_list_.end();++curve_iter)
	{
		int channels(curve_iter->channels.size());
		if(channels<min_channels)
			min_channels=channels;
	}
	// and use it when sizing the points
	vector<Gdk::Point> points[min_channels];

	gc->set_function(Gdk::COPY);
	gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

	// Draw zero mark
	gc->set_rgb_fg_color(Gdk::Color("#4f4f4f"));
	window->draw_rectangle(gc, false, 0, round_to_int((0-r_bottom)/dr), w, 0);

	// This try to find a valid vanvas to show the keyframes of those
	// valuenodes. If not canvas found then no keyframes marks are shown.
	synfig::Canvas::Handle canvas=0;
	for(curve_iter=curve_list_.begin();curve_iter!=curve_list_.end();++curve_iter)
	{
		canvas=curve_iter->value_desc.get_canvas();
		if(canvas)
			break;
	}

	if(canvas)
	{
	// Draw vertical lines for the keyframes marks.
		const synfig::KeyframeList& keyframe_list(canvas->keyframe_list());
		synfig::KeyframeList::const_iterator iter;

		for(iter=keyframe_list.begin();iter!=keyframe_list.end();++iter)
		{
			if(!iter->get_time().is_valid())
				continue;

			const int x((int)((float)w/(t_end-t_begin)*(iter->get_time()-t_begin)));
			if(iter->get_time()>=t_begin && iter->get_time()<t_end)
			{
				gc->set_rgb_fg_color(Gdk::Color("#a07f7f")); // It should be user selectable
				window->draw_rectangle(gc, true, x, 0, 1, h);
			}
		}
	}

	// Draw current time
	gc->set_rgb_fg_color(Gdk::Color("#0000ff")); // It should be user selectable
	window->draw_rectangle(gc, false, round_to_int((time_adjustment_->get_value()-t_begin)/dt), 0, 0, h);

	// Draw curves for the valuenodes stored in the curve list
	for(curve_iter=curve_list_.begin();curve_iter!=curve_list_.end();++curve_iter)
	{
		Real t;
		int i;
		int channels(curve_iter->channels.size());
		for(i=0;i<channels;i++)
			points[i].clear();

		for(i=0,t=t_begin;i<w;i++,t+=dt)
		{
			for(int chan=0;chan<channels;chan++)
			{
				Real x(curve_iter->get_value(chan,t,dt));
				r_max=max(r_max,x);
				r_min=min(r_min,x);
				points[chan].push_back(
					Gdk::Point(
						i,
						round_to_int(
							(
								x-r_bottom
							)/dr
						)
					)
				);
			}
		}

		for(int chan=0;chan<channels;chan++)
		{
			gc->set_rgb_fg_color(curve_iter->channels[chan].color);

			// Draw the curve
			window->draw_lines(gc, Glib::ArrayHandle<Gdk::Point>(points[chan]));

			Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));

			layout->set_text(curve_iter->channels[chan].name);
			window->draw_layout(gc, 1, points[chan][0].get_y()+1, layout);
		}
	}

	if(!curve_list_.empty())
	{
		range_adjustment_->set_upper(r_max+range_adjustment_->get_page_size()/2);
		range_adjustment_->set_lower(r_min-range_adjustment_->get_page_size()/2);
	}
	window->get_update_area();

	return true;
}
