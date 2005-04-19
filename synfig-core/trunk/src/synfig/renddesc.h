/* === S Y N F I G ========================================================= */
/*!	\file renddesc.h
**	\brief Template Header
**
**	$Id: renddesc.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERDESC_H
#define __SYNFIG_RENDERDESC_H

/* === H E A D E R S ======================================================= */

#include "vector.h"
#include "color.h"
#include "types.h"
#include <cmath>
#include "rect.h"

/* === M A C R O S ========================================================= */

#ifndef DPM2DPI
#define DPM2DPI(x)	(float(x)/39.3700787402f)
#define DPI2DPM(x)	(float(x)*39.3700787402f)
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class RendDesc
**	\todo writeme
*/
class RendDesc
{
public:
	enum Lock
	{
		PX_ASPECT=(1<<0),
		PX_AREA=(1<<1),
		PX_W=(1<<2),
		PX_H=(1<<3),
		
		IM_ASPECT=(1<<4),
		IM_SPAN=(1<<5),
		IM_W=(1<<6),
		IM_H=(1<<7),
		IM_ZOOMIN=(1<<8),
		IM_ZOOMOUT=(1<<9),
				
		LINK_PX_ASPECT=(1<<10),
		LINK_PX_AREA=(1<<11),
		LINK_IM_ASPECT=(1<<12),
		LINK_IM_SPAN=(1<<13),
		LINK_IM_CENTER=(1<<14)
	};
	
private:
	int w_,h_;
	Real x_res;
	Real y_res;
	Point tl_, br_;
	Point focus;
	int a;
	//Gamma gamma;
	Color background;
	int flags;
	bool interlaced;
	bool clamp;

	float frame_rate;
	Time time_begin, time_end;
		
public:

	enum
	{
		ANTIALIAS_UNIFORM,
		ANTIALIAS_MONTE_CARLO,
		ANTIALIAS_JITTERED,
		ANTIALIAS_ADAPTIVE,
		ANTIALIAS_QUINTCUNX
	} AntialiasFilter;

	//! Default Constructor
	RendDesc():
		w_			(480),
		h_			(270),
		x_res		(DPI2DPM(72.0f)),
		y_res		(DPI2DPM(72.0f)),
		tl_			(-4,2.25),
		br_			(4,-2.25),
		focus		(0,0),
		a			(2),
		background	(Color::gray()),
		flags		(0),
		interlaced	(false),
		clamp		(false),
		frame_rate	(24),
		time_begin	(0),
		time_end	(0)
	{ }
	
	//! \writeme
	RendDesc &apply(const RendDesc &x);
	
	//! \writeme
	const Color &get_bg_color()const;

	//! \writeme
	RendDesc &set_bg_color(const Color &bg);

	//! Return the width of the composition in pixels
	int get_w()const;

	//! Set the width of the composition in pixels.
	/*! The other parameters are adjusted according to the
	**	constraints placed on the flags.
	*/
	RendDesc &set_w(int x);

	//! Return the height of the composition in pixels
	int	get_h()const;

	//! Set the height of the composition in pixels.
	/*! The other parameters are adjusted according to the
	**	constraints placed on the flags.
	*/
	RendDesc &set_h(int y);

	//!	Sets the width and height of the composition in pixels
	RendDesc &set_wh(int x, int y);

    //! Returns the horizontal resolution (in dots per meter)
	Real get_x_res()const;

	//! Sets the horizontal resolution (in dots per meter)
	RendDesc &set_x_res(Real x);
	
    //! Returns the vertical resolution (in dots per meter)
	Real get_y_res()const;

	//! Sets the vertical resolution (in dots per meter)
	RendDesc &set_y_res(Real y);


	//! Return the physical width of the composition in meters
	Real get_physical_w()const;

	//! Return the physical height of the composition in meters
	Real get_physical_h()const;

	//! Set the physical width of the composition in meters
	RendDesc &set_physical_w(Real w);

	//! Set the physical height of the composition in meters
	RendDesc &set_physical_h(Real h);


	//!	Return the index of the first frame
	int get_frame_start()const;

	//! Set the index of the first frame
	RendDesc &set_frame_start(int x);
	
	//!	Return the index of the last frame
	int get_frame_end()const;
	
	//! Set the index of the last frame
	RendDesc &set_frame_end(int x);

	//!	Return the starting time of the animation
	const Time get_time_start()const;
	
	//!	Set the time that the animation will start
	RendDesc &set_time_start(Time x);

	//! Return the end time of the animation
	const Time get_time_end()const;
	
	//!	Set the time that the animation will end
	RendDesc &set_time_end(Time x);

	//!	Setup for one frame at the given time
	RendDesc &set_time(Time x);

	//!	Setup for one frame
	RendDesc &set_frame(int x);

	//!	Return the frame rate (frames-per-second)
	const float &get_frame_rate()const;
	
	//! Set the frame rate (frames-per-second)
	RendDesc &set_frame_rate(float x);

	//! Return the status of the interlaced flag
	const bool &get_interlaced()const;
	
	//! Set the interlace flag
	RendDesc &set_interlaced(bool x);

	//! Return the status of the clamp flag
	const bool &get_clamp()const;
	
	//! Set the clamp flag
	RendDesc &set_clamp(bool x);

	//! Set constraint flags
	RendDesc &set_flags(const int &x);
	
	//! Clear constraint flags
	RendDesc &clear_flags();

	//! Get constraint flags
	int get_flags()const;

	//!	Return the aspect ratio of a single pixel
	Point::value_type get_pixel_aspect()const;

	//!	Return the aspect ratio of the entire image
	Point::value_type get_image_aspect()const;
	
	//! Return the antialias amount
	const int &get_antialias()const;

	//! Set the antilaias amount
	RendDesc &set_antialias(const int &x);

	//! Return the distance from the bottom-right to the top-left
	Real get_span()const;

	//! Set the span distance
	RendDesc& set_span(const Real &x);

	//const Gamma &get_gamma()const;

	//RendDesc &set_gamma(const Gamma &x);

	const Point &get_focus()const;

	RendDesc &set_focus(const Point &x);

	const Point &get_tl()const;
	
	RendDesc &set_tl(const Point &x);

	const Point &get_br()const;
	
	RendDesc &set_br(const Point &x);
	
	Rect get_rect()const { return Rect(get_tl(),get_br()); }

	RendDesc &set_viewport(const Point &__tl, const Point &__br);
	
	RendDesc &set_viewport(Vector::value_type a,Vector::value_type b,Vector::value_type c,Vector::value_type d);

	//! Returns the width of one pixel
	Real get_pw()const;

	//! Returns the height of one pixel
	Real get_ph()const;

	//! Sets viewport to represent the screen at the give pixel coordinates
	RendDesc &set_subwindow(int x, int y, int w, int h);
};	// END of class RendDesc

//! This operator allows the combining of RendDesc::Lock flags using the '|' operator
/*!	\see RendDesc::Lock, RendDesc */
inline RendDesc::Lock operator|(RendDesc::Lock lhs, RendDesc::Lock rhs)
{
	return static_cast<RendDesc::Lock>((int)lhs|(int)rhs);
}

//! This operator allows the masking of RendDesc::Lock flags using the '&' operator
/*!	\see RendDesc::Lock, RendDesc */
inline RendDesc::Lock operator&(RendDesc::Lock lhs, RendDesc::Lock rhs)
{
	return static_cast<RendDesc::Lock>((int)lhs&(int)rhs);
}

//! This operator allows the inverting of RendDesc::Lock flags using the '~' operator
/*!	\see RendDesc::Lock, RendDesc */
inline RendDesc::Lock operator~(RendDesc::Lock rhs)
{
	return static_cast<RendDesc::Lock>(~(int)rhs);
}

//! This operator is for checking RendDesc::Lock flags.
/*! Don't think of it as "less then or equal to", but think of it
**	like an arrow. Is \a rhs inside of \a lhs ?
**	\see RendDesc::Lock, RendDesc */
inline bool operator<=(RendDesc::Lock lhs, RendDesc::Lock rhs)
{
	return static_cast<int>(lhs) & static_cast<int>(rhs)==static_cast<int>(rhs);
}


}; /* end namespace synfig */

/* === E N D =============================================================== */

#endif
