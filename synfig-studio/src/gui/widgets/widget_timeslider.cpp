/* === S Y N F I G ========================================================= */
/*!	\file widget_timeslider.cpp
**	\brief Time Slider Widget Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2012, Carlos López
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

#include "widgets/widget_timeslider.h"

#include <ETL/misc>

#include <cmath>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

using studio::Widget_Timeslider;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
const double zoominfactor = 0.75;
const double zoomoutfactor = 1/zoominfactor;

/* === P R O C E D U R E S ================================================= */

Gdk::Color get_interp_color(synfig::Interpolation x)
{
	switch(x)
	{
	case INTERPOLATION_TCB:
		return Gdk::Color("#73d216");

		break;

	case INTERPOLATION_LINEAR:
		return Gdk::Color("#edd400");
		break;

	case INTERPOLATION_CONSTANT:
		return Gdk::Color("#cc0000");
		break;

	case INTERPOLATION_HALT:
		return Gdk::Color("#3465a4");
		break;

	case INTERPOLATION_MANUAL:
		return Gdk::Color("#75507b");
		break;

	case INTERPOLATION_CLAMPED:
		return Gdk::Color("#c17d11");
		break;

	case INTERPOLATION_UNDEFINED: default:
		return Gdk::Color("#555753");
		break;
	}
}

static Gdk::Color
color_darken(Gdk::Color x, float amount)
{
	double   red = x.get_red_p()   * amount;
	double green = x.get_green_p() * amount;
	double  blue = x.get_blue_p()  * amount;

	x.set_rgb_p(  red > 1 ? 1 : red,
				green > 1 ? 1 : green,
				 blue > 1 ? 1 : blue);

	return x;
}

void
studio::render_time_point_to_window(
	const Glib::RefPtr<Gdk::Drawable>& window,
	const Gdk::Rectangle& area,
	const synfig::TimePoint &tp,
	bool selected
)
{
	Glib::RefPtr<Gdk::GC> gc(Gdk::GC::create(window));
	const Gdk::Color black("#2e3436");

	if(selected)
		gc->set_line_attributes(2,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);
	else
		gc->set_line_attributes(1,Gdk::LINE_SOLID,Gdk::CAP_BUTT,Gdk::JOIN_MITER);

	Gdk::Color color;
	std::vector<Gdk::Point> points;

/*-	BEFORE ------------------------------------- */

	color=get_interp_color(tp.get_before());
	color=color_darken(color,1.0f);
	if(selected)color=color_darken(color,1.3f);
	gc->set_rgb_fg_color(color);

	switch(tp.get_before())
	{
	case INTERPOLATION_TCB:
		window->draw_arc(
			gc,
			true,
			area.get_x(),
			area.get_y(),
			area.get_width(),
			area.get_height(),
			64*90,
			64*180
		);
		gc->set_rgb_fg_color(black);
		window->draw_arc(
			gc,
			false,
			area.get_x(),
			area.get_y(),
			area.get_width(),
			area.get_height(),
			64*90,
			64*180
		);
		break;

	case INTERPOLATION_HALT:
		window->draw_arc(
			gc,
			true,
			area.get_x(),
			area.get_y(),
			area.get_width(),
			area.get_height()*2,
			64*90,
			64*90
		);
		gc->set_rgb_fg_color(black);
		window->draw_arc(
			gc,
			false,
			area.get_x(),
			area.get_y(),
			area.get_width(),
			area.get_height()*2,
			64*90,
			64*90
		);

		points.clear();
		points.push_back(Gdk::Point(area.get_x(),area.get_y()+area.get_height()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_lines(gc,points);

		break;

	case INTERPOLATION_LINEAR:
		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x(),area.get_y()+area.get_height()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_polygon(gc,true,points);
		gc->set_rgb_fg_color(black);
		window->draw_lines(gc,points);
		break;

	case INTERPOLATION_CONSTANT:
		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/4,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/4,area.get_y()+area.get_height()/2));
		points.push_back(Gdk::Point(area.get_x(),area.get_y()+area.get_height()/2));
		points.push_back(Gdk::Point(area.get_x(),area.get_y()+area.get_height()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_polygon(gc,true,points);
		gc->set_rgb_fg_color(black);
		window->draw_lines(gc,points);
		break;

	case INTERPOLATION_CLAMPED:
		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x(),area.get_y()+area.get_height()/2));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_polygon(gc,true,points);
		gc->set_rgb_fg_color(black);
		window->draw_lines(gc,points);
		break;

	case INTERPOLATION_UNDEFINED: default:
		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/3,area.get_y()));
		points.push_back(Gdk::Point(area.get_x(),area.get_y()+area.get_height()/3));
		points.push_back(Gdk::Point(area.get_x(),area.get_y()+area.get_height()-area.get_height()/3));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/3,area.get_y()+area.get_height()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_polygon(gc,true,points);
		gc->set_rgb_fg_color(black);
		window->draw_lines(gc,points);
		break;
	}

/*-	AFTER -------------------------------------- */

	color=get_interp_color(tp.get_after());
	color=color_darken(color,0.8f);
	if(selected)color=color_darken(color,1.3f);
	gc->set_rgb_fg_color(color);

	switch(tp.get_after())
	{
	case INTERPOLATION_TCB:
		window->draw_arc(
			gc,
			true,
			area.get_x(),
			area.get_y(),
			area.get_width(),
			area.get_height(),
			64*270,
			64*180
		);
		gc->set_rgb_fg_color(black);
		window->draw_arc(
			gc,
			false,
			area.get_x(),
			area.get_y(),
			area.get_width(),
			area.get_height(),
			64*270,
			64*180
		);
		break;

	case INTERPOLATION_HALT:
		window->draw_arc(
			gc,
			true,
			area.get_x(),
			area.get_y()-area.get_height(),
			area.get_width(),
			area.get_height()*2,
			64*270,
			64*90
		);
		gc->set_rgb_fg_color(black);
		window->draw_arc(
			gc,
			false,
			area.get_x(),
			area.get_y()-area.get_height(),
			area.get_width(),
			area.get_height()*2,
			64*270,
			64*90
		);

		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width(),area.get_y()));
		window->draw_lines(gc,points);

		break;

	case INTERPOLATION_LINEAR:
		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width(),area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_polygon(gc,true,points);
		gc->set_rgb_fg_color(black);
		window->draw_lines(gc,points);
		break;

	case INTERPOLATION_CONSTANT:
		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width(),area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width(),area.get_y()+area.get_height()/2));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()-area.get_width()/4,area.get_y()+area.get_height()/2));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()-area.get_width()/4,area.get_y()+area.get_height()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_polygon(gc,true,points);
		gc->set_rgb_fg_color(black);
		window->draw_lines(gc,points);
		break;

	case INTERPOLATION_CLAMPED:
		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width(),area.get_y()+area.get_height()/2));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_polygon(gc,true,points);
		gc->set_rgb_fg_color(black);
		window->draw_lines(gc,points);
		break;

	case INTERPOLATION_UNDEFINED: default:
		points.clear();
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()-area.get_width()/3,area.get_y()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width(),area.get_y()+area.get_height()/3));
		points.push_back(Gdk::Point(area.get_x()+area.get_width(),area.get_y()+area.get_height()-area.get_height()/3));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()-area.get_width()/3,area.get_y()+area.get_height()));
		points.push_back(Gdk::Point(area.get_x()+area.get_width()/2,area.get_y()+area.get_height()));
		window->draw_polygon(gc,true,points);
		gc->set_rgb_fg_color(black);
		window->draw_lines(gc,points);
		break;
	}

}

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
double defaultfps = 24;
const int fullheight = 20;

Widget_Timeslider::Widget_Timeslider()
:layout(Pango::Layout::create(get_pango_context())),
adj_default(0,0,2,1/defaultfps,10/defaultfps),
adj_timescale(0),
//invalidated(false),
last_event_time(0),
fps(defaultfps),
dragscroll(false)
{
	set_size_request(-1,fullheight);

	//                click                    scroll                     zoom
	add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK
				| Gdk::BUTTON_MOTION_MASK | Gdk::SCROLL_MASK );

	set_time_adjustment(&adj_default);
	//update_times();
}

Widget_Timeslider::~Widget_Timeslider()
{
}

void Widget_Timeslider::set_time_adjustment(Gtk::Adjustment *x)
{
	//disconnect old connections
	time_value_change.disconnect();
	time_other_change.disconnect();

	//connect update function to new adjustment
	adj_timescale = x;

	if(x)
	{
		time_value_change = x->signal_value_changed().connect(sigc::mem_fun(*this,&Widget_Timeslider::queue_draw));
		time_other_change = x->signal_changed().connect(sigc::mem_fun(*this,&Widget_Timeslider::queue_draw));
		//invalidated = true;
		//refresh();
	}
}

void Widget_Timeslider::set_global_fps(float d)
{
	if(fps != d)
	{
		fps = d;

		//update everything since we need to redraw already
		//invalidated = true;
		//refresh();
		queue_draw();
	}
}

/*void Widget_Timeslider::update_times()
{
	if(adj_timescale)
	{
		start = adj_timescale->get_lower();
		end = adj_timescale->get_upper();
		current = adj_timescale->get_value();
	}
}*/

void Widget_Timeslider::refresh()
{
}
/*
{
	if(invalidated)
	{
		queue_draw();
	}else if(adj_timescale)
	{
		double 	l = adj_timescale->get_lower(),
				u = adj_timescale->get_upper(),
				v = adj_timescale->get_value();

		bool invalid = (l != start) || (u != end) || (v != current);

		start = l;
		end = u;
		current = v;

		if(invalid) queue_draw();
	}
}*/

bool Widget_Timeslider::redraw(bool /*doublebuffer*/)
{
	Glib::RefPtr<Gdk::Window> window = get_window();

	if(!window) return false;

	Glib::RefPtr<Gdk::GC>	gc = Gdk::GC::create(window);
	if(!gc) return false;

	//synfig::info("Drawing Timeslider");
	//clear	and update to current values
	//invalidated = false;
	//update_times();

	//draw grey rectangle
	Gdk::Color	c("#7f7f7f");
	gc->set_rgb_fg_color(c);
	gc->set_background(c);

	//Get the data for the window and the params to draw it...
	int w = get_width(), h = get_height();

	window->draw_rectangle(gc,true,0,0,w,h);

	const double EPSILON = 1e-6;
	if(!adj_timescale || w == 0) return true;

	//Get the time information since we now know it's valid
	double 	start = adj_timescale->get_lower(),
			end = adj_timescale->get_upper(),
			current = adj_timescale->get_value();

	if(end-start < EPSILON) return true;

	//synfig::info("Drawing Lines");

	//draw all the time stuff
	double dtdp = (end - start)/get_width();
	double dpdt = 1/dtdp;

	//lines

	//Draw the time line...
	double tpx = (current-start)*dpdt;
	gc->set_rgb_fg_color(Gdk::Color("#ffaf00"));
	window->draw_line(gc,round_to_int(tpx),0,round_to_int(tpx),fullheight);

	//normal line/text color
	gc->set_rgb_fg_color(Gdk::Color("#333333"));

	int ifps = round_to_int(fps);
	if (ifps < 1) ifps = 1;

	std::vector<double> ranges;

	unsigned int pos = 0;

	// build a list of all the factors of the frame rate
	for (int i = 1; i*i <= ifps; i++)
		if ((ifps%i) == 0)
		{
			ranges.insert(ranges.begin()+pos, i/fps);
			if (i*i != ifps)
				ranges.insert(ranges.begin()+pos+1, ifps/i/fps);
			pos++;
		}

	// fill in any gaps where one factor is more than 2 times the previous
	std::vector<double>::iterator iter, next;
	pos = 0;
	for (pos = 0; pos < ranges.size()-1; pos++)
	{
		iter = ranges.begin()+pos;
		next = iter+1;
		if (*iter*2 < *next)
			ranges.insert(next, *iter*2);
	}

	double more_ranges[] = {
		2, 3, 5, 10, 20, 30, 60, 90, 120, 180,
		300, 600, 1200, 1800, 2700, 3600, 3600*2,
		3600*4, 3600*8, 3600*16, 3600*32, 3600*64,
		3600*128, 3600*256, 3600*512, 3600*1024 };

	ranges.insert(ranges.end(), more_ranges, more_ranges + sizeof(more_ranges)/sizeof(double));

	double lowerrange = dtdp*140, upperrange = dtdp*280;
	double midrange = (lowerrange + upperrange)/2;

	//find most ideal scale
	double scale;
	next = binary_find(ranges.begin(), ranges.end(), midrange);
	iter = next++;

	if (iter == ranges.end()) iter--;
	if (next == ranges.end()) next--;

	if (abs(*next - midrange) < abs(*iter - midrange))
		iter = next;

	scale = *iter;

	// subdivide into this many tick marks (8 or less)
	int subdiv = round_to_int(scale * ifps);

	if (subdiv > 8)
	{
		const int ideal = subdiv;

		// find a number of tick marks that nicely divides the scale
		// (5 minutes divided by 6 is 50s, but that's not 'nice' -
		//  5 ticks of 1m each is much simpler than 6 ticks of 50s)
		for (subdiv = 8; subdiv > 0; subdiv--)
			if ((ideal <= ifps*2       && (ideal % (subdiv           )) == 0) ||
				(ideal <= ifps*2*60    && (ideal % (subdiv*ifps      )) == 0) ||
				(ideal <= ifps*2*60*60 && (ideal % (subdiv*ifps*60   )) == 0) ||
				(true                  && (ideal % (subdiv*ifps*60*60)) == 0))
				break;

		// if we didn't find anything, use 4 ticks
		if (!subdiv)
			subdiv = 4;
	}

	time_per_tickmark = scale / subdiv;

	//get first valid line and its position in pixel space
	double time = 0;
	double pixel = 0;

	int sdindex = 0;

	double subr = scale / subdiv;

	//get its position inside...
	time = ceil(start/subr)*subr - start;
	pixel = time*dpdt;

	//absolute time of the line to be drawn
	time += start;

	{ //inside the big'n
		double t = (time/scale - floor(time/scale))*subdiv; // the difference from the big mark in 0:1
		//sdindex = (int)floor(t + 0.5); //get how far through the range it is...
		sdindex = round_to_int(t); //get how far through the range it is...
		if (sdindex == subdiv) sdindex = 0;

		//synfig::info("Extracted fr %.2lf -> %d", t, sdindex);
	}

	//synfig::info("Initial values: %.4lf t, %.1lf pixels, %d i", time,pixel,sdindex);

	//loop to draw
	const int heightbig = 12;
	const int heightsmall = 4;

	int width = get_width();
	while( pixel < width )
	{
		int xpx = round_to_int(pixel);

		//draw big
		if(sdindex == 0)
		{
			window->draw_line(gc,xpx,0,xpx,heightbig);
			//round the time to nearest frame and draw the text
			Time tm((double)time);
			if(get_global_fps()) tm.round(get_global_fps());
			Glib::ustring timecode(tm.get_string(get_global_fps(),App::get_time_format()));

			//gc->set_rgb_fg_color(Gdk::Color("#000000"));
			layout->set_text(timecode);
			Pango::AttrList attr_list;
			// Aproximately a font size of 8 pixels.
			// Pango::SCALE = 1024
			// create_attr_size waits a number in 1000th of pixels.
			// Should be user customizable in the future. Now it is fixed to 10
			Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*10));
			pango_size.set_start_index(0);
			pango_size.set_end_index(64);
			attr_list.change(pango_size);
			layout->set_attributes(attr_list);
			window->draw_layout(gc,xpx+2,0,layout);
		}else
		{
			window->draw_line(gc,xpx,0,xpx,heightsmall);
		}

		//increment time and position
		pixel += subr / dtdp;
		time += subr;

		//increment index
		if(++sdindex >= subdiv) sdindex -= subdiv;
	}

	return true;
}

bool Widget_Timeslider::on_motion_notify_event(GdkEventMotion* event) //for dragging
{
	if(!adj_timescale) return false;

	Gdk::ModifierType mod = Gdk::ModifierType(event->state);

	//scrolling...

	//NOTE: we might want to address the possibility of dragging with both buttons held down

	if(mod & Gdk::BUTTON2_MASK)
	{

		//we need this for scrolling by dragging
		double 	curx = event->x;

		double 	start = adj_timescale->get_lower(),
				end = adj_timescale->get_upper();

		if(dragscroll)
		{
			if(event->time-last_event_time<30)
				return false;
			else
				last_event_time=event->time;

			if(abs(lastx - curx) < 1 && end != start) return true;
			//translate the window and correct it

			//update our stuff so we are operating correctly
			//invalidated = true;
			//update_times();

			//Note: Use inverse of mouse movement because of conceptual space relationship
			double diff = lastx - curx; //curx - lastx;

			//NOTE: This might be incorrect...
			//fraction to move...
			double dpx = (end - start)/get_width();
			lastx = curx;

			diff *= dpx;

			//Adjust...
			start += diff;
			end += diff;

			//But clamp to bounds if they exist...
			//HACK - bounds should not be required for this slider
			if(adj_bounds)
			{
				if(start < adj_bounds->get_lower())
				{
					diff = adj_bounds->get_lower() - start;
					start += diff;
					end += diff;
				}

				if(end > adj_bounds->get_upper())
				{
					diff = adj_bounds->get_upper() - end;
					start += diff;
					end += diff;
				}
			}

			//synfig::info("Scrolling timerange to (%.4f,%.4f)",start,end);

			adj_timescale->set_lower(start);
			adj_timescale->set_upper(end);

			adj_timescale->changed();
		}else
		{
			dragscroll = true;
			lastx = curx;
			//lasty = cury;
		}

		return true;
	}

	if(mod & Gdk::BUTTON1_MASK)
	{
		double curx = event->x;

		//get time from drag...
		double 	start = adj_timescale->get_lower(),
				end = adj_timescale->get_upper(),
				current = adj_timescale->get_value();
		double t = start + curx*(end - start)/get_width();

		//snap it to fps - if they exist...
		if(fps)
		{
			t = floor(t*fps + 0.5)/fps;
		}

		//set time if needed
		if(current != t)
		{
			adj_timescale->set_value(t);

			//Fixed this to actually do what it's supposed to...
			if(event->time-last_event_time>50)
			{
				adj_timescale->value_changed();
				last_event_time = event->time;
			}
		}

		return true;
	}

	return false;
}

bool Widget_Timeslider::on_scroll_event(GdkEventScroll* event) //for zooming
{
	if(!adj_timescale) return false;

	//Update so we are calculating based on current values
	//update_times();

	//figure out if we should center ourselves on the current time
	bool center = false;

	//we want to zoom in on the time value if control is held down
	if(Gdk::ModifierType(event->state) & Gdk::CONTROL_MASK)
		center = true;

	switch(event->direction)
	{
		case GDK_SCROLL_UP: //zoom in
			zoom_in(center);
			return true;

		case GDK_SCROLL_DOWN: //zoom out
			zoom_out(center);
			return true;

		case GDK_SCROLL_RIGHT:
		case GDK_SCROLL_LEFT:
		{
			double t = adj_timescale->get_value();
			double orig_t = t;
			double start = adj_timescale->get_lower();
			double end = adj_timescale->get_upper();
			double lower = adj_bounds->get_lower();
			double upper = adj_bounds->get_upper();
			double adj = time_per_tickmark;

			if( event->direction == GDK_SCROLL_RIGHT )
			{
				// step forward one tick
				t += adj;

				// don't go past the end of time
				if (t > upper)
					t = upper;

				// if we are already in the right half of the slider
				if ((t-start)*2 > (end-start))
				{
					// if we can't scroll the background left one whole tick, scroll it to the end
					if (end > upper - (t-orig_t))
					{
						adj_timescale->set_lower(upper - (end-start));
						adj_timescale->set_upper(upper);
					}
					// else scroll the background left
					else
					{
						adj_timescale->set_lower(start + (t-orig_t));
						adj_timescale->set_upper(start + (t-orig_t) + (end-start));
					}
				}
			}
			else
			{
				// step backwards one tick
				t -= adj;

				// don't go past the start of time
				if (t < lower)
					t = lower;

				// if we are already in the left half of the slider
				if ((t-start)*2 < (end-start))
				{
					// if we can't scroll the background right one whole tick, scroll it to the beginning
					if (start < lower + (orig_t-t))
					{
						adj_timescale->set_lower(lower);
						adj_timescale->set_upper(lower + (end-start));
					}
					// else scroll the background right
					else
					{
						adj_timescale->set_lower(start - (orig_t-t));
						adj_timescale->set_upper(start - (orig_t-t) + (end-start));
					}
				}
			}

			if(adj_timescale)
			{
				adj_timescale->set_value(t);
				adj_timescale->value_changed();
			}
			return true;
		}
		default:
			return false;
	}
}

void Widget_Timeslider::zoom_in(bool centerontime)
{
	if(!adj_timescale) return;

	double 	start = adj_timescale->get_lower(),
			end = adj_timescale->get_upper(),
			current = adj_timescale->get_value();

	double focuspoint = centerontime ? current : (start + end)/2;

	//calculate new beginning and end
	end = focuspoint + (end-focuspoint)*zoominfactor;
	start = focuspoint + (start-focuspoint)*zoominfactor;

	//synfig::info("Zooming in timerange to (%.4f,%.4f)",start,end);
	if(adj_bounds)
	{
		if(start < adj_bounds->get_lower())
		{
			start = adj_bounds->get_lower();
		}

		if(end > adj_bounds->get_upper())
		{
			end = adj_bounds->get_upper();
		}
	}

	//reset values
	adj_timescale->set_lower(start);
	adj_timescale->set_upper(end);

	//call changed function
	adj_timescale->changed();
}

void Widget_Timeslider::zoom_out(bool centerontime)
{
	if(!adj_timescale) return;

	double 	start = adj_timescale->get_lower(),
			end = adj_timescale->get_upper(),
			current = adj_timescale->get_value();

	double focuspoint = centerontime ? current : (start + end)/2;

	//calculate new beginning and end
	end = focuspoint + (end-focuspoint)*zoomoutfactor;
	start = focuspoint + (start-focuspoint)*zoomoutfactor;

	//synfig::info("Zooming out timerange to (%.4f,%.4f)",start,end);
	if(adj_bounds)
	{
		if(start < adj_bounds->get_lower())
		{
			start = adj_bounds->get_lower();
		}

		if(end > adj_bounds->get_upper())
		{
			end = adj_bounds->get_upper();
		}
	}

	//reset values
	adj_timescale->set_lower(start);
	adj_timescale->set_upper(end);

	//call changed function
	adj_timescale->changed();
}

bool Widget_Timeslider::on_button_press_event(GdkEventButton *event) //for clicking
{
	switch(event->button)
	{
		//time click...
		case 1:
		{
			double 	start = adj_timescale->get_lower(),
					end = adj_timescale->get_upper(),
					current = adj_timescale->get_value();

			double w = get_width();
			double t = start + (end - start) * event->x / w;

			t = floor(t*fps + 0.5)/fps;

			/*synfig::info("Clicking time from %.3lf to %.3lf [(%.2lf,%.2lf) %.2lf / %.2lf ... %.2lf",
						current, vt, start, end, event->x, w, fps);*/

			if(t != current)
			{
				current = t;

				if(adj_timescale)
				{
					adj_timescale->set_value(current);
					adj_timescale->value_changed();
				}
			}

			break;
		}

		//scroll click
		case 2:
		{
			//start dragging
			dragscroll = true;
			lastx = event->x;
			//lasty = event->y;

			return true;
		}

		default:
		{
			break;
		}
	}

	return false;
}

bool Widget_Timeslider::on_button_release_event(GdkEventButton *event) //end drag
{
	switch(event->button)
	{
		case 2:
		{
			//start dragging
			dragscroll = false;
			return true;
		}

		default:
		{
			break;
		}
	}

	return false;
}
