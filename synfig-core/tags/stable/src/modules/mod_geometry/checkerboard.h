/*! ========================================================================
** Sinfg
** Checkerboard Layer
** $Id: checkerboard.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_CHECKERBOARD_H
#define __SINFG_CHECKERBOARD_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer_composite.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class CheckerBoard : public sinfg::Layer_Composite, public sinfg::Layer_NoDeform
{
	SINFG_LAYER_MODULE_EXT
private:
	sinfg::Color color;
	sinfg::Point pos;
	sinfg::Point size;

	bool point_test(const sinfg::Point& x)const;

public:
	CheckerBoard();
	
	virtual bool set_param(const sinfg::String & param, const sinfg::ValueBase &value);

	virtual sinfg::ValueBase get_param(const sinfg::String & param)const;

	virtual sinfg::Color get_color(sinfg::Context context, const sinfg::Point &pos)const;
	
	virtual Vocab get_param_vocab()const;

	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;

	virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;
};

/* === E N D =============================================================== */

#endif
