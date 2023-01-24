/* === S Y N F I G ========================================================= */
/*!	\file halftone2.h
**	\brief Header file for implementation of the "Halftone 2" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_HALFTONE2_H
#define __SYNFIG_HALFTONE2_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/valuenode.h>
#include <synfig/layers/layer_composite_fork.h>
#include <synfig/time.h>
#include <synfig/angle.h>
#include "halftone.h"

/* === M A C R O S ========================================================= */
#define HALFTONE2_IMPORT_VALUE(x)                                             \
	if (#x=="halftone.param_"+param && x.get_type()==value.get_type())        \
		{                                                                     \
			x=value;                                                          \
			return true;                                                      \
		}                                                                     \

#define HALFTONE2_EXPORT_VALUE(x)                                             \
	if (#x=="halftone.param_"+param)                                          \
		{                                                                     \
			return x;                                                         \
		}                                                                     \

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
using namespace synfig;

class Halftone2 : public Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (class Halftone)
	Halftone halftone;
	//! Parameter: (Color)
	ValueBase param_color_dark;
	//! Parameter: (Color)
	ValueBase param_color_light;

	Color color_func(const Point &x, float supersample,const Color &under_color)const;

	float calc_supersample(const Point &x, float pw,float ph)const;

	//float halftone_func(Point x)const;

public:
	Halftone2();

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	Layer::Handle hit_check(Context context, const Point &point)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;
}; // END of class Halftone2

/* === E N D =============================================================== */

#endif
