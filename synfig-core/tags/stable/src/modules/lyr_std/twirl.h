/* === S I N F G =========================================================== */
/*!	\file twirl.h
**	\brief Template Header
**
**	$Id: twirl.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SINFG_TWIRL_H
#define __SINFG_TWIRL_H

/* === H E A D E R S ======================================================= */

#include <sinfg/layer_composite.h>
#include <sinfg/color.h>
#include <sinfg/vector.h>
#include <sinfg/value.h>
#include <sinfg/gradient.h>
#include <sinfg/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
class Twirl_Trans;

class Twirl : public sinfg::Layer_Composite
{
	SINFG_LAYER_MODULE_EXT
	friend class Twirl_Trans;
	
private:

	sinfg::Point center;

	sinfg::Real radius;

	sinfg::Angle rotations;

	bool distort_inside;

	bool distort_outside;

	sinfg::Point distort(const sinfg::Point &pos, bool reverse=false)const;
public:
	
	Twirl();
	
	virtual bool set_param(const sinfg::String & param, const sinfg::ValueBase &value);

	virtual sinfg::ValueBase get_param(const sinfg::String & param)const;

	virtual sinfg::Color get_color(sinfg::Context context, const sinfg::Point &pos)const;
	
	//virtual bool accelerated_render(sinfg::Context context,sinfg::Surface *surface,int quality, const sinfg::RendDesc &renddesc, sinfg::ProgressCallback *cb)const;

	sinfg::Layer::Handle hit_check(sinfg::Context context, const sinfg::Point &point)const;	
	
	virtual Vocab get_param_vocab()const;
	virtual etl::handle<sinfg::Transform> get_transform()const;
}; // END of class Twirl

/* === E N D =============================================================== */

#endif
