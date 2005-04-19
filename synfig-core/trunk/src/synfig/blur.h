/* === S I N F G =========================================================== */
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

#ifndef __SINFG_BLUR_HELPER_H
#define __SINFG_BLUR_HELPER_H

/* === H E A D E R S ======================================================= */
#include <sinfg/surface.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
class sinfg::ProgressCallback;
	
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
	sinfg::Point	size;
	int				type;

	sinfg::ProgressCallback *cb;

public:
	sinfg::Point & set_size(const sinfg::Point &v) { return (size = v); }
	const sinfg::Point & get_size() const { return size; }
	sinfg::Point & get_size() { return size; }
	
	int & set_type(const int &t) { return (type = t); }
	const int & get_type() const { return type; }
	int & get_type() { return type; }
	
	Blur() {}
	Blur(const sinfg::Point &s, int t, sinfg::ProgressCallback *callb=0):size(s), type(t), cb(callb) {}
	Blur(sinfg::Real sx, sinfg::Real sy, int t, sinfg::ProgressCallback *callb = 0): size(sx,sy), type(t), cb(callb) {}
	
	//Parametric Blur
	sinfg::Point operator ()(const sinfg::Point &p) const;
	sinfg::Point operator ()(sinfg::Real x, sinfg::Real y) const;
	
	//Surface based blur
	//	input surface can be the same as output surface,
	//	though both have to be the same size
	bool operator ()(const sinfg::Surface &surface, const sinfg::Vector &resolution, sinfg::Surface &out) const;
		
	bool operator ()(const etl::surface<float> &surface, const sinfg::Vector &resolution, etl::surface<float> &out) const;
	//bool operator ()(const etl::surface<unsigned char> &surface, const sinfg::Vector &resolution, etl::surface<unsigned char> &out) const;
};

/* === E N D =============================================================== */

#endif
