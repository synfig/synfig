/* === S Y N F I G ========================================================= */
/*!	\file synfig/blur.h
**	\brief Blur Helper Header file
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_BLUR_HELPER_H
#define __SYNFIG_BLUR_HELPER_H

/* === H E A D E R S ======================================================= */
#include <synfig/surface.h>
#include <synfig/color.h>
#include <synfig/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
using namespace synfig;
using namespace std;
using namespace etl;

class Blur
{
public:
	enum Type
	{
		BOX				=0,
		FASTGAUSSIAN	=1,
		CROSS			=2,
		GAUSSIAN		=3,
		DISC			=4,

		FORCE_DWORD = 0x8fffffff
	};

private:
	Point size;
	int type;
	ProgressCallback *cb;

public:
	void set_size(const Point &x) { size = x; }
	const Point& get_size() const { return size; }

	void set_type(int x) { type = x; }
	int get_type() const { return type; }

	static Real get_surface_extra_size_amplifier(int type) {
		static const Real gauss_max_deviation = 1.0 / 512.0;
		static const Real gauss_size_amplifier = 0.5*sqrt(-2.0*log(gauss_max_deviation));

		switch(type)
		{
			case Blur::DISC:
			case Blur::BOX:
			case Blur::CROSS:
				return 0.5;
			case Blur::FASTGAUSSIAN:
				return 1.0;
			case Blur::GAUSSIAN:
				return gauss_size_amplifier;
			default:
				break;
		}
		return 0.0;
	}

	Real get_surface_extra_size_amplifier() const
		{ return get_surface_extra_size_amplifier(get_type()); }

	Point get_surface_extra_size() const
		{ return get_size() * get_surface_extra_size_amplifier(); }

	void get_surface_extra_size(Real pixels_per_width_unit, Real pixels_per_height_unit, int &out_dx, int &out_dy) const {
		Point es = get_surface_extra_size();
		out_dx = (int)ceil(fabs(es[0]*pixels_per_width_unit));
		out_dy = (int)ceil(fabs(es[1]*pixels_per_height_unit));
	}

	void get_surface_expanded_size(Real pixels_per_width_unit, Real pixels_per_height_unit, int &width, int &height) const {
		int dw, dh;
		width = width > 0 ? width + 2*dw : 2*dw;
		height = height > 0 ? height + 2*dh : 2*dh;
	}

	Blur(): type(), cb() {}
	Blur(const Point &s, int t, ProgressCallback *callb=0):size(s), type(t), cb(callb) {}
	Blur(Real sx, Real sy, int t, ProgressCallback *callb = 0): size(sx,sy), type(t), cb(callb) {}

	//Parametric Blur
	Point operator()(const Point &p) const;
	Point operator()(Real x, Real y) const;

	//Surface based blur
	//	input surface can be the same as output surface,
	//	though both have to be the same size
	bool operator()(const Surface &surface, const Vector &resolution, Surface &out) const;
	bool operator()(cairo_surface_t *surface, const Vector &resolution, cairo_surface_t *out) const;

	bool operator()(const etl::surface<float> &surface, const Vector &resolution, etl::surface<float> &out) const;
	//bool operator()(const etl::surface<unsigned char> &surface, const Vector &resolution, etl::surface<unsigned char> &out) const;
};

/* === E N D =============================================================== */

#endif
