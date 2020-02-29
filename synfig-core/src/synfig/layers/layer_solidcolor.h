/* === S Y N F I G ========================================================= */
/*!	\file layer_solidcolor.h
**	\brief Header file for implementation of the "Solid Color" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_LAYER_SOLIDCOLOR_H
#define __SYNFIG_LAYER_SOLIDCOLOR_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"
#include <synfig/color.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Layer_SolidColor : public Layer_Composite, public Layer_NoDeform
{
	SYNFIG_LAYER_MODULE_EXT

private:
	//!Parameter: (Color) color of the solid
	ValueBase param_color;

public:
	Layer_SolidColor();

	virtual bool set_param(const String & param, const synfig::ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual Vocab get_param_vocab()const;

	virtual synfig::Layer::Handle hit_check(synfig::Context context, const synfig::Point &point)const;

protected:
	virtual rendering::Task::Handle build_composite_task_vfunc(ContextParams context_params)const;
}; // END of class Layer_SolidColor

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
