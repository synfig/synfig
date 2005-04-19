/* === S Y N F I G ========================================================= */
/*!	\file blur.h
**	\brief Blur Helper Header file
**
**	$Id: blur.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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
