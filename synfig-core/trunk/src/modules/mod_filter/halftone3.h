/* === S I N F G =========================================================== */
/*!	\file halftone3.h
**	\brief Template Header
**
**	$Id: halftone3.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_HALFTONE3_H
#define __SINFG_HALFTONE3_H

/* === H E A D E R S ======================================================= */

#include <sinfg/vector.h>
#include <sinfg/valuenode.h>
#include <sinfg/layer_composite.h>
#include <sinfg/time.h>
#include <sinfg/angle.h>
#include "halftone.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Halftone3 : public sinfg::Layer_Composite
{
	SINFG_LAYER_MODULE_EXT

private:

	sinfg::Vector size;
	int type;
	Halftone tone[3];
	sinfg::Color	color[3];
	float inverse_matrix[3][3];
	bool subtractive;

	sinfg::Color color_func(const sinfg::Point &x, float supersample,const sinfg::Color &under_color)const;

	float calc_supersample(const sinfg::Point &x, float pw,float ph)const;

	//float halftone_func(sinfg::Point x)const;
	
	void sync();

public:
	Halftone3();
	
	virtual bool set_param(const sinfg::String &param, const sinfg::ValueBase &value);
	virtual sinfg::ValueBase get_param(const sinfg::String &param)const;
	virtual sinfg::Color get_color(sinfg::Context context, const sinfg::Point &pos)const;
	virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;
	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	

	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif
