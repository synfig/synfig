/*! ========================================================================
** Sinfg
** Template Header File
** $Id: mandelbrot.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_MANDELBROT_H
#define __SINFG_MANDELBROT_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer.h>
#include <sinfg/color.h>
#include <sinfg/angle.h>
#include <sinfg/gradient.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

class Mandelbrot : public Layer
{
	SINFG_LAYER_MODULE_EXT

private:

	Real bailout;
	Real lp;
	int iterations;

	bool smooth_outside;
	bool broken;

	bool distort_inside;
	bool distort_outside;
	bool solid_inside;
	bool solid_outside;	
	bool invert_inside;
	bool invert_outside;
	bool shade_outside;
	bool shade_inside;
	Real gradient_offset_inside;
	Real gradient_offset_outside;
	bool gradient_loop_inside;
	Real gradient_scale_outside;
	Gradient gradient_inside;
	Gradient gradient_outside;

public:
	Mandelbrot();
	
	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual Vocab get_param_vocab()const;	
};

/* === E N D =============================================================== */

#endif
