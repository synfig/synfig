/*! ========================================================================
** Sinfg
** Template Header File
** $Id: curvegradient.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_CURVEGRADIENT_H
#define __SINFG_CURVEGRADIENT_H

/* === H E A D E R S ======================================================= */

#include <sinfg/vector.h>
#include <sinfg/layer_composite.h>
#include <sinfg/gradient.h>
#include <sinfg/blinepoint.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace sinfg;
using namespace std;
using namespace etl;

class CurveGradient : public Layer_Composite, public Layer_NoDeform
{
	SINFG_LAYER_MODULE_EXT

private:
	std::vector<sinfg::BLinePoint> bline;

	Point p1,p2;
	Point offset;
	Real width;
	Vector diff;
	Gradient gradient;
	Real	curve_length_;

	bool loop;
	bool zigzag;
	bool bline_loop;
	bool perpendicular;

	void sync();

	sinfg::Color color_func(const sinfg::Point &x, int quality=10, float supersample=0)const;

	float calc_supersample(const sinfg::Point &x, float pw,float ph)const;

public:
	CurveGradient();
	
	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
