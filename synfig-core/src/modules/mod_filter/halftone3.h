/* === S Y N F I G ========================================================= */
/*!	\file halftone3.h
**	\brief Header file for implementation of the "Halftone 3" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2012-2013 Carlos López
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

#ifndef __SYNFIG_HALFTONE3_H
#define __SYNFIG_HALFTONE3_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/valuenode.h>
#include <synfig/layers/layer_composite_fork.h>
#include <synfig/time.h>
#include <synfig/angle.h>
#include "halftone.h"

/* === M A C R O S ========================================================= */
#define HALFTONE3_IMPORT_VALUE(x)                                             \
	if (#x=="tone[i].param_"+param && x.get_type()==value.get_type())         \
		{                                                                     \
			x=value;                                                          \
			return true;                                                      \
		}                                                                     \

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
using namespace synfig;
using namespace etl;

class Halftone3 : public Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//! Parameter: (Vector)
	ValueBase param_size;
	//! Parameter: (int)
	ValueBase param_type;
	//! Parameter: (class Halftone)
	Halftone tone[3];
	//! Parameter: (Color)
	ValueBase param_color[3];
	//! Parameter: (bool)
	ValueBase param_subtractive;

	float inverse_matrix[3][3];

	Color color_func(const Point &x, float supersample,const Color &under_color)const;

	float calc_supersample(const Point &x, float pw,float ph)const;

	//float halftone_func(Point x)const;

	void sync();

public:
	Halftone3();

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	Layer::Handle hit_check(Context context, const Point &point)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;
}; // END of class Halftone3

/* === E N D =============================================================== */

#endif
