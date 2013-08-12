/* === S Y N F I G ========================================================= */
/*!	\file synfig/renddesc.cpp
**	\brief Class that defines the parameters needed by the Renderer to
* render a context to a surface.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "renddesc.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#undef FLAGS
#define FLAGS(x,y)		(((x)&(y))==(y))

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

RendDesc &
RendDesc::apply(const RendDesc &x)
{
	operator=(x);
	return *this;
}

const Color &
RendDesc::get_bg_color()const
{
	return background;
}

RendDesc &
RendDesc::set_bg_color(const Color &bg)
{
	background=bg; return *this;
}

Real
RendDesc::get_physical_w()const
{
	return (Real)get_w()/get_x_res();
}

Real
RendDesc::get_physical_h()const
{
	return (Real)get_h()/get_y_res();
}

RendDesc&
RendDesc::set_physical_w(Real w)
{
	set_w(round_to_int(w*get_x_res()));
	return *this;
}

RendDesc&
RendDesc::set_physical_h(Real h)
{
	set_h(round_to_int(h*get_y_res()));
	return *this;
}

int
RendDesc::get_w()const
{
	return w_;
}

RendDesc &
RendDesc::set_w(int x)
{
	if(FLAGS(flags,LINK_PX_ASPECT)) // never set
	{
		h_=h_*x/w_;
		w_=x;
	}
	else if(FLAGS(flags,LINK_PX_AREA)) // never set
	{
		//! \writeme
		w_=x;
	}
	else if(FLAGS(flags,PX_ASPECT)) // "Pixel Aspect"
	{
		Vector d=br_-tl_;
		float old_span=get_span();

		// If we should preserve image width
		if(		FLAGS(flags,IM_W)							// "Image Width"
			|| (FLAGS(flags,IM_ZOOMIN) && d[1]>d[1]/x*w_)	// never set
			|| (FLAGS(flags,IM_ZOOMOUT) && d[1]<d[1]/x*w_)) // never set
		{
			br_[1]-=focus[1];
			br_[1]=br_[1]/x*w_;
			br_[1]+=focus[1];
			tl_[1]-=focus[1];
			tl_[1]=tl_[1]/x*w_;
			tl_[1]+=focus[1];
		} else
		{
			br_[0]-=focus[0];
			br_[0]=br_[0]/w_*x;
			br_[0]+=focus[0];
			tl_[0]-=focus[0];
			tl_[0]=tl_[0]/w_*x;
			tl_[0]+=focus[0];
		}

		w_=x;

		if(FLAGS(flags,IM_SPAN)) // "Image Span"
			set_span(old_span);
	}
	else if(FLAGS(flags,PX_AREA)) // never set
	{
		//! \writeme
		w_=x;
	}
	else
		w_=x;

	return *this;
}

int
RendDesc::get_h()const
{
	return h_;
}

RendDesc &
RendDesc::set_h(int y)
{
	if(FLAGS(flags,LINK_PX_ASPECT)) // never set
	{
		w_=w_*y/h_;
		h_=y;
	}
	else if(FLAGS(flags,LINK_PX_AREA)) // never set
	{
		//! \writeme
		h_=y;
	}
	else if(FLAGS(flags,PX_ASPECT)) // "Pixel Aspect"
	{
		Vector d=br_-tl_;
		float old_span=get_span();

		// If we should preserve image width
		if(		FLAGS(flags,IM_W)							// "Image Width"
			|| (FLAGS(flags,IM_ZOOMIN) && d[0]>d[0]/y*h_)	// never set
			|| (FLAGS(flags,IM_ZOOMOUT) && d[0]<d[0]/y*h_)) // never set
		{
			br_[0]-=focus[0];
			br_[0]=br_[0]/y*h_;
			br_[0]+=focus[0];
			tl_[0]-=focus[0];
			tl_[0]=tl_[0]/y*h_;
			tl_[0]+=focus[0];
		} else
		{
			br_[1]-=focus[1];
			br_[1]=br_[1]/h_*y;
			br_[1]+=focus[1];
			tl_[1]-=focus[1];
			tl_[1]=tl_[1]/h_*y;
			tl_[1]+=focus[1];
		}

		h_=y;

		if(FLAGS(flags,IM_SPAN)) // "Image Span"
			set_span(old_span);
	}
	else if(FLAGS(flags,PX_AREA)) // never set
	{
		//! \writeme
		h_=y;
	}
	else
		h_=y;

	return *this;
}

RendDesc &
RendDesc::set_wh(int x, int y)
{
	// FIXME: This is a working hack...
	set_w(x);
	set_h(y);

	return *this;
}

Real
RendDesc::get_x_res()const
{
	return x_res;
}

RendDesc &
RendDesc::set_x_res(Real x)
{
	x_res=x; return *this;
}

Real
RendDesc::get_y_res()const
{
	return y_res;
}

RendDesc &
RendDesc::set_y_res(Real y)
{
	y_res=y; return *this;
}

int
RendDesc::get_frame_start()const
{
	return round_to_int(time_begin*frame_rate);
}

RendDesc &
RendDesc::set_frame_start(int x)
{
	return set_time_start(Time(x)/frame_rate);
}

int
RendDesc::get_frame_end()const
{
	return round_to_int(time_end*frame_rate);
}

RendDesc &
RendDesc::set_frame_end(int x)
{
	return set_time_end(Time(x)/frame_rate);
}


const Time
RendDesc::get_time_start()const
{
	return time_begin;
}

RendDesc &
RendDesc::set_time_start(Time x)
{
	if(x>time_end)
		time_begin=time_end=x;
	else
		time_begin=x;
	return *this;
}


const Time
RendDesc::get_time_end()const
{
	return time_end;
}

RendDesc &
RendDesc::set_time_end(Time x)
{
	if(x<time_begin)
		time_end=time_begin=x;
	else
		time_end=x;
	return *this;
}

RendDesc &
RendDesc::set_time(Time x)
{
	time_end=time_begin=x;
	return *this;
}

RendDesc &
RendDesc::set_frame(int x)
{
	return set_time(Time(x)/frame_rate);
}

const float &
RendDesc::get_frame_rate()const
{
	return frame_rate;
}

RendDesc &
RendDesc::set_frame_rate(float x)
{
	frame_rate=x;
	return *this;
}

const bool &
RendDesc::get_interlaced()const
{
	return interlaced;
}

RendDesc &
RendDesc::set_interlaced(bool x)
{ interlaced=x; return *this; }

//! Return the status of the clamp flag
const bool &
RendDesc::get_clamp()const
{ return clamp; }

//! Set the clamp flag
RendDesc &
RendDesc::set_clamp(bool x)
{ clamp=x; return *this; }

//! Return the status of the render_excluded_contexts flag
const bool &
RendDesc::get_render_excluded_contexts()const
{ return render_excluded_contexts; }

//! Set the render_excluded_contexts flag
RendDesc &
RendDesc::set_render_excluded_contexts(bool x)
{ render_excluded_contexts=x; return *this; }

//! Set constraint flags
RendDesc &
RendDesc::set_flags(const int &x)
{ flags=x; return *this; }

//! Clear constraint flags
RendDesc &
RendDesc::clear_flags()
{ flags=0; return *this; }

int
RendDesc::get_flags()const
{ return flags; }


//!	Return the aspect ratio of a single pixel
Real
RendDesc::get_pixel_aspect()const
{
	Vector tmp=br_-tl_;
	tmp[0]/=w_;
	tmp[1]/=h_;
	tmp[0]/=tmp[1];
	if(tmp[0]<0.0)
		return -tmp[0];
	return tmp[0];
}

//!	Return the aspect ratio of the entire image
Real
RendDesc::get_image_aspect()const
{
	Point tmp=br_-tl_;
	tmp[0]/=tmp[1];
	if(tmp[0]<0.0)
		return -tmp[0];
	return tmp[0];
}


//! Return the antialias amount
const int &
RendDesc::get_antialias()const
{ return a; }

//! Set the antialias amount
RendDesc &
RendDesc::set_antialias(const int &x)
{ a=x; return *this; }


//! Return the distance from the bottom-right to the top-left
Real
RendDesc::get_span()const
{
	return (br_-tl_).mag();
}

//! Set the span distance
RendDesc &
RendDesc::set_span(const Real &x)
{
	Vector::value_type ratio=x/get_span();

	//! \todo this looks wrong.  I suspect the intention was to check
	//		  "(not IM_W) AND (not IM_H)", ie "not(IM_W OR IM_H)" but
	//		  this check does "not(IM_W AND IM_H)"
	if(!FLAGS(flags,IM_W|IM_H) || FLAGS(flags,IM_ASPECT)) // (not "Image Width") or (not "Image Height") or "Image Aspect"
	{
		br_-=focus;
		br_=br_*ratio;
		br_+=focus;
		tl_-=focus;
		tl_=tl_*ratio;
		tl_+=focus;
	}
	else if(FLAGS(flags,IM_W))	// "Image Width"
	{
		//! \writeme or fix me
		br_-=focus;
		br_=br_*ratio;
		br_+=focus;
		tl_-=focus;
		tl_=tl_*ratio;
		tl_+=focus;
	}else // IM_H				// "Image Height"
	{
		//! \writeme or fix me
		br_-=focus;
		br_=br_*ratio;
		br_+=focus;
		tl_-=focus;
		tl_=tl_*ratio;
		tl_+=focus;
	}

	return *this;
}


const Point &
RendDesc::get_focus()const
{ return focus; }

RendDesc &
RendDesc::set_focus(const Point &x)
{ focus=x; return *this; }


const Point &
RendDesc::get_tl()const
{ return tl_; }

const Point &
RendDesc::get_br()const
{ return br_; }

RendDesc &
RendDesc::set_tl(const Point &x)
{
	if(FLAGS(flags,PX_ASPECT)) // "Pixel Aspect"
	{
		Vector new_size(x-br_);
		new_size[0]=abs(new_size[0]);
		new_size[1]=abs(new_size[1]);

		Vector old_size(tl_-br_);
		old_size[0]=abs(old_size[0]);
		old_size[1]=abs(old_size[1]);

		if(new_size[0]!=old_size[0])
			w_=round_to_int(new_size[0]*w_/old_size[0]);

		if(new_size[1]!=old_size[1])
			h_=round_to_int(new_size[1]*h_/old_size[1]);
	}

	tl_=x; return *this;
}

RendDesc &
RendDesc::set_br(const Point &x)
{
	if(FLAGS(flags,PX_ASPECT)) // "Pixel Aspect"
	{
		Vector new_size(x-tl_);
		new_size[0]=abs(new_size[0]);
		new_size[1]=abs(new_size[1]);

		Vector old_size(tl_-br_);
		old_size[0]=abs(old_size[0]);
		old_size[1]=abs(old_size[1]);

		if(new_size[0]!=old_size[0])
			w_=round_to_int(new_size[0]*w_/old_size[0]);

		if(new_size[1]!=old_size[1])
			h_=round_to_int(new_size[1]*h_/old_size[1]);
	}
	br_=x; return *this;
}

RendDesc &
RendDesc::set_tl_br(const Point &x, const Point &y)
{
	if(FLAGS(flags, PX_ASPECT))
	{
		Vector new_size(y-x);
		new_size[0]=abs(new_size[0]);
		new_size[1]=abs(new_size[1]);
		
		Vector old_size(tl_-br_);
		old_size[0]=abs(old_size[0]);
		old_size[1]=abs(old_size[1]);
		
		if(new_size[0]!=old_size[0])
			w_=round_to_int(new_size[0]*w_/old_size[0]);
		
		if(new_size[1]!=old_size[1])
			h_=round_to_int(new_size[1]*h_/old_size[1]);
	}
	tl_=x;
	br_=y;
	return *this;
}


RendDesc &
RendDesc::set_viewport(const Point &__tl, const Point &__br)
{ tl_=__tl; br_=__br; return *this; }

RendDesc &
RendDesc::set_viewport(Vector::value_type a, Vector::value_type b, Vector::value_type c, Vector::value_type d)
{ tl_=Point(a,b); br_=Point(c,d); return *this; }

Real
RendDesc::get_pw()const
{
	return (br_[0] - tl_[0]) / w_;
}

Real
RendDesc::get_ph()const
{
	return (br_[1] - tl_[1]) / h_;
}

RendDesc &
RendDesc::set_subwindow(int x, int y, int w, int h)
{
	const Real pw(get_pw());
	const Real ph(get_ph());

	tl_[0]+=pw*x;
	tl_[1]+=ph*y;

	br_[0]-=pw*(w_-(x+w));
	br_[1]-=ph*(h_-(y+h));

	w_=w;
	h_=h;

	return *this;
}

RendDesc &
RendDesc::set_duration(Time duration)
{
	if(get_frame_rate())
	{
		if(duration > Time(0.0))
			set_time_end(get_time_start() + duration  - Time(1/get_frame_rate()));
		else
			set_time_end(get_time_start());
	}
		
	return *this;
}

const Time
RendDesc::get_duration()
{
	if(get_frame_rate())		
		return (get_time_end() - get_time_start() + Time(1/get_frame_rate()));
	return Time(0.0);
}
