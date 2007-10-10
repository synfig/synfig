/* === S Y N F I G ========================================================= */
/*!	\file synfig/blur.h
**	\brief Blur Helper Header file
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
class synfig::ProgressCallback;

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
	synfig::Point	size;
	int				type;

	synfig::ProgressCallback *cb;

public:
	synfig::Point & set_size(const synfig::Point &v) { return (size = v); }
	const synfig::Point & get_size() const { return size; }
	synfig::Point & get_size() { return size; }

	int & set_type(const int &t) { return (type = t); }
	const int & get_type() const { return type; }
	int & get_type() { return type; }

	Blur() {}
	Blur(const synfig::Point &s, int t, synfig::ProgressCallback *callb=0):size(s), type(t), cb(callb) {}
	Blur(synfig::Real sx, synfig::Real sy, int t, synfig::ProgressCallback *callb = 0): size(sx,sy), type(t), cb(callb) {}

	//Parametric Blur
	synfig::Point operator ()(const synfig::Point &p) const;
	synfig::Point operator ()(synfig::Real x, synfig::Real y) const;

	//Surface based blur
	//	input surface can be the same as output surface,
	//	though both have to be the same size
	bool operator ()(const synfig::Surface &surface, const synfig::Vector &resolution, synfig::Surface &out) const;

	bool operator ()(const etl::surface<float> &surface, const synfig::Vector &resolution, etl::surface<float> &out) const;
	//bool operator ()(const etl::surface<unsigned char> &surface, const synfig::Vector &resolution, etl::surface<unsigned char> &out) const;
};

/* === E N D =============================================================== */

#endif
