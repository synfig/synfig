/* === S Y N F I G ========================================================= */
/*!	\file layer_motionblur.h
**	\brief Header file for implementation of the "Motion Blur" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "layer_composite_fork.h"
#include <synfig/time.h>

/* === S T R U C T S & C L A S S E S ======================================= */

namespace synfig {

class Layer_MotionBlur : public synfig::Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT

	enum SubsamplingType
	{
	    SUBSAMPLING_CONSTANT=0,		//!< weight each subsample equally
	    SUBSAMPLING_LINEAR=1,		//!< fade in subsamples linearly
	    SUBSAMPLING_HYPERBOLIC=2,	//!< fade in subsamples by a hyperbolic curve (default style of 0.62 and previous)

	    SUBSAMPLING_END=2				//!< \internal
	};

private:
	ValueBase param_aperture;
	ValueBase param_subsamples_factor;
	ValueBase param_subsampling_type;
	ValueBase param_subsample_start;
	ValueBase param_subsample_end;

public:
	Layer_MotionBlur();
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;
}; // END of class Layer_MotionBlur

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
