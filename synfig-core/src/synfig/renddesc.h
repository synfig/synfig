/* === S Y N F I G ========================================================= */
/*!	\file synfig/renddesc.h
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
		PX_ASPECT=(1<<0),		// "Pixel Aspect" in Locks and Links
		PX_AREA=(1<<1),			// not used
		PX_W=(1<<2),			// "Pixel Width" in Locks and Links - not used
		PX_H=(1<<3),			// "Pixel Height" in Locks and Links - not used

		IM_ASPECT=(1<<4),		// "Image Aspect" in Locks and Links
		IM_SPAN=(1<<5),			// "Image Span" in Locks and Links
		IM_W=(1<<6),			// "Image Width" in Locks and Links
		IM_H=(1<<7),			// "Image Height" in Locks and Links
		IM_ZOOMIN=(1<<8),		// not used
		IM_ZOOMOUT=(1<<9),		// not used

		LINK_PX_ASPECT=(1<<10),	// not used
		LINK_PX_AREA=(1<<11),	// not used
		LINK_IM_ASPECT=(1<<12),	// not used
		LINK_IM_SPAN=(1<<13),	// not used
		LINK_IM_CENTER=(1<<14)	// not used
	};

private:
	//! Width and height of the compostion in pixels
	int w_,h_;
	//! Horizontal resolution of the composition in pixels per meter
	Real x_res;
	//! Vertical resolution of the composition in pixels per meter
	Real y_res;
	//! The Top Left and the Bottom Right Points of the composition
	Point tl_, br_;
	//! The Focus Point of the compostion. Used when zooming in
	Point focus;
	//! Anti-alias value
	int a;
	//! The background color used when alpha is not supported or avoided
	Color background;
	//! The result of the flags combination.
	//! \see enum Lock
	int flags;
	//! Interlaced flag for targets that suports it
	bool interlaced;
	//! Clamp flag to decide if color must be clamped or not
	bool clamp;
	//! When \c true layers with exclude_from_rendering flag should be rendered
	bool render_excluded_contexts;
	//! Frame rate of the composition to be rendered
	float frame_rate;
	//! Begin time and end time of the Composition to render
	Time time_begin, time_end;

public:
	//! Anti alias filers types. Seems never implemented
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
		render_excluded_contexts(false),
		frame_rate	(24),
		time_begin	(0),
		time_end	(0)
	{ }

	//! Applies the given Render Description \x to the current one
	RendDesc &apply(const RendDesc &x);

	//! Gets the background color
	const Color &get_bg_color()const;

	//! Sets the background color
	RendDesc &set_bg_color(const Color &bg);

	//! Return the width of the composition in pixels
	int get_w()const;

	//! Set the width of the composition in pixels.
	/*! The other parameters are adjusted according to the
	**	constraints placed on the flags.
	* Seems to be incomplete and doesn't use all the possible
	* flags.
	* \todo write the needed code to keep the flags usage
	*/
	RendDesc &set_w(int x);

	//! Return the height of the composition in pixels
	int	get_h()const;

	//! Set the height of the composition in pixels.
	/*! The other parameters are adjusted according to the
	**	constraints placed on the flags.
	* Seems to be incomplete and doesn't use all the possible
	* flags.
	* \todo write the needed code to keep the flags usage
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

	//! Return the status of the render_excluded_contexts flag
	const bool &get_render_excluded_contexts()const;

	//! Set the render_excluded_contexts flag
	RendDesc &set_render_excluded_contexts(bool x);

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

	//! Set the antialias amount
	RendDesc &set_antialias(const int &x);

	//! Return the distance from the bottom-right to the top-left
	Real get_span()const;

	//! Set the span distance
	RendDesc& set_span(const Real &x);

	//! Gets the focus Point
	const Point &get_focus()const;
	//! Sets the focus Point
	RendDesc &set_focus(const Point &x);
	//! Gets the top left point of the compostion
	const Point &get_tl()const;
	//! Sets the top left point of the compostion
	RendDesc &set_tl(const Point &x);
	//! Gets the bottom right point of the compostion
	const Point &get_br()const;
	//! Sets the bottom right point of the compostion
	RendDesc &set_br(const Point &x);
	//! Sets the top left and the bottom right of the composition
	// Use this when the individual set_tl or set_br
	// produce degenerate w or h
	RendDesc &set_tl_br( const Point &x, const Point &y);
	//! Returns the rectangle of the composition
	Rect get_rect()const { return Rect(get_tl(),get_br()); }
	//! Sets the view port by the top left and right bottom corners
	RendDesc &set_viewport(const Point &__tl, const Point &__br);
	//! Sets the view port by the four corners values
	RendDesc &set_viewport(Vector::value_type a,Vector::value_type b,Vector::value_type c,Vector::value_type d);
	//! Returns the width of one pixel
	Real get_pw()const;
	//! Returns the height of one pixel
	Real get_ph()const;
	//! Sets viewport to represent the screen at the given pixel coordinates
	RendDesc &set_subwindow(int x, int y, int w, int h);
	//! Sets the duration of the animation. 
	// Keeps the start time and modifies the end time to match the duration
	RendDesc &set_duration(Time t);
	//! Gets the duration of the animation
	const Time get_duration();
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

}; /* end namespace synfig */

/* === E N D =============================================================== */

#endif
