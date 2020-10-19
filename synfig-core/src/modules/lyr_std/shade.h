/* === S Y N F I G ========================================================= */
/*!	\file shade.h
**	\brief Header file for implementation of the "Shade" layer
**
**	$Id$
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

/* === H E A D E R S ======================================================= */

#ifndef __SYNFIG_LAYER_SHADE_H__
#define __SYNFIG_LAYER_SHADE_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layers/layer_composite_fork.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/blur.h>

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class Layer_Shade : public Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//!Parameter: (Vector)
	ValueBase param_size;
	//!Parameter: (int)
	ValueBase param_type;
	//!Parameter: (Color)
	ValueBase param_color;
	//!Parameter: (Vector)
	ValueBase param_origin;
	//!Parameter: (bool)
	ValueBase param_invert;

public:
	Layer_Shade();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;
	virtual Rect get_full_bounding_rect(Context context)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }

protected:
	virtual rendering::Task::Handle build_composite_fork_task_vfunc(ContextParams context_params, rendering::Task::Handle sub_task)const;
}; // END of class Layer_Shade

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* -- E X T E R N S --------------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
