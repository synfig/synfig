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
	Point	size;
	int				type;

	ProgressCallback *cb;

public:
	Point & set_size(const Point &v) { return (size = v); }
	const Point & get_size() const { return size; }
	Point & get_size() { return size; }

	int & set_type(const int &t) { return (type = t); }
	const int & get_type() const { return type; }
	int & get_type() { return type; }

	Blur() {}
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
