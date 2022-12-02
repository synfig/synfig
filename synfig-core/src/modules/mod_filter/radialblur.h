/* === S Y N F I G ========================================================= */
/*!	\file radialblur.h
**	\brief Header file for implementation of the "Radial Blur" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RADIALBLUR_H
#define __SYNFIG_RADIALBLUR_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/angle.h>
#include <synfig/layers/layer_composite_fork.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;

class RadialBlur : public Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT
	friend class RadialBlur_Trans;
private:
	//! Parameter: (Vector)
	ValueBase param_origin;
	//! Parameter: (Real)
	ValueBase param_size;
	//! Parameter: (bool)
	ValueBase param_fade_out;

public:
	RadialBlur();
	~RadialBlur();

	virtual bool set_param(const synfig::String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const synfig::String & param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;
}; // END of class RadialBlur

/* === E N D =============================================================== */

#endif
