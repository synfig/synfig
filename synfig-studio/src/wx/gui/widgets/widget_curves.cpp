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
#define MAX_CHANNELS 15
/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

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
		Type &type(value_desc.get_value_type());
		if (type == type_real)
		{
			channels.push_back(Channel());
			channels.back().name="real";
			channels.back().color=Gdk::Color("#007f7f");
		}
		else
		if (type == type_time)
		{
			channels.push_back(Channel());
			channels.back().name="time";
			channels.back().color=Gdk::Color("#7f7f00");
		}
		else
		if (type == type_integer)
		{
			channels.push_back(Channel());
			channels.back().name="int";
			channels.back().color=Gdk::Color("#7f0000");
		}
		else
		if (type == type_bool)
		{
			channels.push_back(Channel());
			channels.back().name="bool";
			channels.back().color=Gdk::Color("#ff7f00");
		}
		else
		if (type == type_angle)
		{
			channels.push_back(Channel());
			channels.back().name="theta";
			channels.back().color=Gdk::Color("#004f4f");
		}
		else
		if (type == type_color)
		{
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
		}
		else
		if (type == type_vector)
		{
			channels.push_back(Channel());
			channels.back().name="x";
			channels.back().color=Gdk::Color("#7f007f");
			channels.push_back(Channel());
			channels.back().name="y";
			channels.back().color=Gdk::Color("#007f7f");
		}
		else
		if (type == type_bline_point)
		{
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
		}
		else
		if (type == type_width_point)
		{
			channels.push_back(Channel());
			channels.back().name="position";
			channels.back().color=Gdk::Color("#ff0000");
			channels.push_back(Channel());
			channels.back().name="width";
			channels.back().color=Gdk::Color("#00ff00");
		}
		else
		if (type == type_dash_item)
		{
			channels.push_back(Channel());
			channels.back().name="offset";
			channels.back().color=Gdk::Color("#ff0000");
			channels.push_back(Channel());
			channels.back().name="length";
			channels.back().color=Gdk::Color("#00ff00");
		}
		else
		{
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
		Type &type(value.get_type());
		if (type == type_real)
			channels[0].values[time]=value.get(Real());
		else
		if (type == type_time)
			channels[0].values[time]=value.get(Time());
		else
		if (type == type_integer)
			channels[0].values[time]=value.get(int());
		else
		if (type == type_bool)
			channels[0].values[time]=value.get(bool());
		else
		if (type == type_angle)
			channels[0].values[time]=Angle::rad(value.get(Angle())).get();
		else
		if (type == type_color)
		{
			channels[0].values[time]=value.get(Color()).get_r();
			channels[1].values[time]=value.get(Color()).get_g();
			channels[2].values[time]=value.get(Color()).get_b();
			channels[3].values[time]=value.get(Color()).get_a();
		}
		else
		if (type == type_vector)
		{
			channels[0].values[time]=value.get(Vector())[0];
			channels[1].values[time]=value.get(Vector())[1];
		}
		else
		if (type == type_bline_point)
		{
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
		}
		else
		if (type == type_width_point)
		{
			channels[0].values[time]=value.get(WidthPoint()).get_position();
			channels[1].values[time]=value.get(WidthPoint()).get_width();
		}
		else
		{
			return 0;
		}

		return -channels[chan].values[time];
	}

	static bool is_not_supported(const synfigapp::ValueDesc& x)
	{
		return x.get_value_type() == type_string
			|| x.get_value_type() == type_canvas
			|| x.get_value_type() == type_gradient
			|| x.get_value_type() == type_list
			|| x.get_value_type() == type_segment;
	}
};

/* === M E T H O D S ======================================================= */

Widget_Curves::Widget_Curves():
	range_adjustment_(Gtk::Adjustment::create(-1,-2,2,0.1,0.1,2))
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
	//set_vadjustment(range_adjustment_);

	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK);

}

Widget_Curves::~Widget_Curves()
{
}

void
Widget_Curves::set_time_adjustment(const Glib::RefPtr<Gtk::Adjustment> &x)
{
	time_adjustment_=x;
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
			if(iter->parent_is_layer())
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
			case GDK_SCROLL_RIGHT:
				if (Gdk::ModifierType(event->scroll.state)&GDK_CONTROL_MASK)
				{
					// Ctrl+scroll , perform zoom in
					range_adjustment_->set_page_size(range_adjustment_->get_page_size()/1.25);
				}
				else
				{
					// Scroll up
					range_adjustment_->set_value(range_adjustment_->get_value()-range_adjustment_->get_step_increment ());
				}
				range_adjustment_->changed();
				break;
			case GDK_SCROLL_DOWN:
			case GDK_SCROLL_LEFT:
				if (Gdk::ModifierType(event->scroll.state)&GDK_CONTROL_MASK)
				{
					// Ctrl+scroll , perform zoom out
					range_adjustment_->set_page_size(range_adjustment_->get_page_size()*1.25);
				}
				else
				{
					// Scroll down
					range_adjustment_->set_value(range_adjustment_->get_value()+range_adjustment_->get_step_increment ());
				}
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
Widget_Curves::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	const int h(get_height());
	const int w(get_width());

	get_style_context()->render_background(cr, 0, 0, w, h);

	if(!time_adjustment_ || !range_adjustment_ || !h || !w)
		return false;

	if(!curve_list_.size())
		return false;

	const Real t_begin(time_adjustment_->get_lower());
	const Real t_end(time_adjustment_->get_upper());
	const Real dt((t_end-t_begin)/w);

	const Real r_bottom(range_adjustment_->get_value());
	const Real r_top(r_bottom+range_adjustment_->get_page_size());
	const Real dr((r_top-r_bottom)/h);
	Real r_max(-100000000);
	Real r_min(100000000);

	std::list<CurveStruct>::iterator curve_iter;
	//Figure out maximum number of channels
	for(curve_iter=curve_list_.begin();curve_iter!=curve_list_.end();++curve_iter)
	{
		int channels(curve_iter->channels.size());
		if(channels>MAX_CHANNELS)
		{
			channels=MAX_CHANNELS;
			synfig::warning("Not allowed more than %d channels! Truncating...", MAX_CHANNELS);
		}
	}
	// and use it when sizing the points
	vector<Gdk::Point> points[MAX_CHANNELS];

	// Draw zero mark
	cr->set_source_rgb(0.31, 0.31, 0.31);
	cr->rectangle(0, round_to_int((0-r_bottom)/dr), w, 0);
	cr->stroke();

	// This try to find a valid canvas to show the keyframes of those
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
				cr->set_source_rgb(0.63, 0.5, 0.5);
				cr->rectangle(x, 0, 1, h);
				cr->fill();
			}
		}
	}

	// Draw current time
	cr->set_source_rgb(0, 0, 1);
	cr->rectangle(round_to_int((time_adjustment_->get_value()-t_begin)/dt), 0, 0, h);
	cr->stroke();

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

		// Draw the graph curves with 0.5 width
		cr->set_line_width(0.5);
		for(int chan=0;chan<channels;chan++)
		{
			// Draw the curve
			std::vector<Gdk::Point> &p = points[chan];
			for(std::vector<Gdk::Point>::iterator i = p.begin(); i != p.end(); ++i)
			{
				if (i == p.begin())
					cr->move_to(i->get_x(), i->get_y());
				else
					cr->line_to(i->get_x(), i->get_y());
			}
			const Gdk::Color &color = curve_iter->channels[chan].color;
			cr->set_source_rgb(color.get_red_p(), color.get_green_p(), color.get_blue_p());
			cr->stroke();

			Glib::RefPtr<Pango::Layout> layout(Pango::Layout::create(get_pango_context()));
			layout->set_text(curve_iter->channels[chan].name);

			cr->move_to(1, points[chan][0].get_y()+1);
			layout->show_in_cairo_context(cr);
		}
	}

	if(!curve_list_.empty())
	{
		range_adjustment_->set_upper(r_max+range_adjustment_->get_page_size()/2);
		range_adjustment_->set_lower(r_min-range_adjustment_->get_page_size()/2);
	}

	return true;
}
