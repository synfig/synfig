/* === S I N F G =========================================================== */
/*!	\file layer_motionblur.h
**	\brief Template Header
**
**	$Id: layer_motionblur.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_LAYER_MOTIONBLUR_H__
#define __SINFG_LAYER_MOTIONBLUR_H__

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include "time.h"

/* === S T R U C T S & C L A S S E S ======================================= */

namespace sinfg {
	
class Layer_MotionBlur : public sinfg::Layer_Composite
{
	SINFG_LAYER_MODULE_EXT

private:

	Time aperture;

	mutable Time time_cur;

public:

	Layer_MotionBlur();
	
	virtual bool set_param(const String & param, const sinfg::ValueBase &value);
	
	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual void set_time(Context context, Time time)const;

	virtual void set_time(Context context, Time time, const Point &pos)const;
	
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;

}; // END of class Layer_MotionBlur

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
