/* === S Y N F I G ========================================================= */
/*!	\file layer_motionblur.h
**	\brief Template Header
**
**	$Id: layer_motionblur.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SYNFIG_LAYER_MOTIONBLUR_H__
#define __SYNFIG_LAYER_MOTIONBLUR_H__

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include "time.h"

/* === S T R U C T S & C L A S S E S ======================================= */

namespace synfig {

class Layer_MotionBlur : public synfig::Layer_Composite
{
	SYNFIG_LAYER_MODULE_EXT

private:

	Time aperture;

	mutable Time time_cur;

public:

	Layer_MotionBlur();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual void set_time(Context context, Time time)const;

	virtual void set_time(Context context, Time time, const Point &pos)const;

	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;

	virtual Vocab get_param_vocab()const;

}; // END of class Layer_MotionBlur

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
