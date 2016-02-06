/* === S Y N F I G ========================================================= */
/*!	\file widget_sound.cpp
**	\brief Widget Sound Implementation File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <gtkmm/adjustment.h>

#include <synfig/general.h>
#include <ETL/clock>

#include "widgets/widget_sound.h"
#include "audiocontainer.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
//using namespace synfig;

using studio::AudioProfile;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

studio::Widget_Sound::Widget_Sound()
{
}

studio::Widget_Sound::~Widget_Sound()
{
}

void studio::Widget_Sound::set_position(double t)
{
	//synfig::info("Setting position to %.2lf s", t);
	if(adj_timescale && t != adj_timescale->get_value())
	{
		float upper = adj_timescale->get_upper();
		float lower = adj_timescale->get_lower();
		float framesize =  upper - lower;

		if(t < lower)
		{
			lower -= ceil((lower-t)/framesize)*framesize;
			upper = lower + framesize;
			adj_timescale->set_lower(lower); adj_timescale->set_upper(upper);
			adj_timescale->set_value(t);
			adj_timescale->changed(); adj_timescale->value_changed();
		}else
		if(t > upper)
		{
			lower += ceil((t-upper)/framesize)*framesize;
			upper = lower + framesize;
			adj_timescale->set_lower(lower); adj_timescale->set_upper(upper);
			adj_timescale->set_value(t);
			adj_timescale->changed(); adj_timescale->value_changed();
		}else
		{
			adj_timescale->set_value(t);
			adj_timescale->value_changed();
		}
	}
}

double studio::Widget_Sound::get_position() const
{
	if(adj_timescale)
	{
		return adj_timescale->get_value();
	}
	return 0;
}

bool studio::Widget_Sound::set_profile(etl::handle<AudioProfile>	p)
{
	clear();

	//set the profile
	audioprof = p;

	if(!audioprof)
	{
		clear();
		return false;
	}

	return true;
}

etl::handle<AudioProfile>	studio::Widget_Sound::get_profile() const
{
	return audioprof;
}

void studio::Widget_Sound::clear()
{
	audioprof.detach();
}

void studio::Widget_Sound::draw()
{
	queue_draw();
}

bool studio::Widget_Sound::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	Gdk::RGBA c("#3f3f3f");

	int w = get_width();
	int baseline = get_height()/2;
	cr->set_source_rgb(c.get_red(), c.get_green(), c.get_blue());
	cr->rectangle(0.0, 0.0, w, get_height());
	cr->fill();

	//draw the base line, set up the color to be blue
	cr->set_source_rgb(0.0, 0.5, 1.0);
	cr->move_to(0,baseline);
	cr->line_to(w,baseline);
	cr->stroke();

	//redraw all the samples from begin to end, but only if we have samples to draw (or there is no space to draw)

	//synfig::warning("Ok rendered everything, now must render actual sound wave");
	if(!audioprof || !adj_timescale || !w)
		return true;

	//draw you fool!
	float framesize = adj_timescale->get_upper() - adj_timescale->get_lower();
	if(framesize)
	{
		float delta=0,cum=0;

		//position in sample space
		int begin=0,end=0;
		int	cur=0,maxs=0,mins=0;

		//etl::clock	check; check.reset();

		float position = adj_timescale->get_value();
		float samplerate = audioprof->get_samplerate();
		int		posi = 0;
		//enforce position inside of frame size
		{
			float offset = audioprof->get_offset();

			//clamp begin and end to framesize
			float beginf = adj_timescale->get_lower();
			float endf = adj_timescale->get_upper();

			posi = round_to_int((position-beginf)*w/framesize);
			//posi = (int)((position-beginf)*w/framesize);

			//calculate in sample space from seconds
			begin = round_to_int((beginf - offset)*samplerate);
			end = round_to_int((endf - offset)*samplerate);
			//begin = (int)((beginf - offset)*samplerate);
			//end = (int)((endf - offset)*samplerate);
		}

		delta = (end - begin)/(float)w; //samples per pixel

		/*synfig::warning("Rendering a framesize of %f secs from [%d,%d) samples to %d samples, took %f sec",
						framesize, begin, end, w, check());*/

		cur = begin;
		cum = 0;
		for(int i=0;i<w;++i)
		{
			//get the maximum of the collected samples
			maxs = 0;
			mins = 0;
			for(;cum < delta; ++cum, ++cur)
			{
				maxs = std::max(maxs,(int)(*audioprof)[cur]);
				mins = std::min(mins,(int)(*audioprof)[cur]);
			}
			cum -= delta;

			//draw spike if not needed be
			if(maxs||mins)
			{
				int top = maxs * baseline / 64;
				int bot = mins * baseline / 64;

				cr->set_source_rgb(0.0, 0.5, 1.0);
				cr->move_to(i,baseline+bot);
				cr->line_to(i,baseline+top);
				cr->stroke();
			}
		}

		//synfig::warning("Drawing audio line");
		cr->set_source_rgb(1.0, 0.0, 0.0);
		cr->move_to(posi,0);
		cr->line_to(posi,get_height());
		cr->stroke();
	}

	return true;
}

//--- Handle the single clicking and dragging for scrubbing

bool studio::Widget_Sound::on_motion_notify_event(GdkEventMotion* event)
{
	Gdk::ModifierType	mod = Gdk::ModifierType(event->state);

	//if we are scrubbing
	if(mod & Gdk::BUTTON1_MASK)
	{
		//Can't do this if we don't have a time frame (heheh...)
		if(!adj_timescale) return false;

		double beg = adj_timescale->get_lower(), end = adj_timescale->get_upper();

		//find event position in time
		double t = beg + event->x * (end-beg) / get_width();

		//signal that we are scrubbing to this new value...
		signal_scrub()(t);


		// We should be able to just call
		// Widget_Timeslider::on_motion_notify_event(),
		// but that seems to cause the program to halt
		// for some reason. So for now, let's do the job ourselves
		//adj_timescale->set_value(t);
		//adj_timescale->changed();
		//return true;
	}

	return Widget_Timeslider::on_motion_notify_event(event);
}

bool studio::Widget_Sound::on_button_press_event(GdkEventButton *event)
{
	//Assume button PRESS

	//if we are starting... using left click
	if(event->button == 1)
	{
		if(!adj_timescale) return false;

		double beg = adj_timescale->get_lower(), end = adj_timescale->get_upper();

		//find event position in time
		double t = beg + event->x * (end-beg) / get_width();

		//signal the attached scrubbing devices...
		signal_start_scrubbing()(t);

		return true;
	}

	return Widget_Timeslider::on_button_press_event(event);
}

bool studio::Widget_Sound::on_button_release_event(GdkEventButton *event)
{
	//Assume button RELEASE

	//if we are ending... using left click
	if(event->button == 1)
	{
		//signal the scrubbing device... to stop
		signal_stop_scrubbing()();

		return true;
	}

	return Widget_Timeslider::on_button_release_event(event);
}
