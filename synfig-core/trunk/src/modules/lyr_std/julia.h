/*! ========================================================================
** Sinfg
** Template Header File
** $Id: julia.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_JULIA_H
#define __SINFG_JULIA_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>
#include <sinfg/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

class Julia : public sinfg::Layer
{
	SINFG_LAYER_MODULE_EXT

private:

	sinfg::Color icolor;
	sinfg::Color ocolor;
	sinfg::Angle color_shift;
	Real bailout;
	Real lp;
	int iterations;
	sinfg::Point seed;

	bool distort_inside;
	bool distort_outside;
	bool shade_inside;
	bool shade_outside;
	bool solid_inside;
	bool solid_outside;	
	bool invert_inside;
	bool invert_outside;
	bool color_inside;
	bool color_outside;

	bool color_cycle;
	bool smooth_outside;
	bool broken;

public:
	Julia();
	
	virtual bool set_param(const sinfg::String &param, const sinfg::ValueBase &value);

	virtual ValueBase get_param(const sinfg::String &param)const;

	virtual Color get_color(sinfg::Context context, const sinfg::Point &pos)const;

	virtual Vocab get_param_vocab()const;	
};

/* === E N D =============================================================== */

#endif
