/* === S Y N F I G ========================================================= */
/*!	\file mod_filter/blur.h
**	\brief Header file for implementation of the "Blur" layer
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

/* === H E A D E R S ======================================================= */

#ifndef __SYNFIG_LAYER_BLUR_H__
#define __SYNFIG_LAYER_BLUR_H__

/* -- H E A D E R S --------------------------------------------------------- */

#include <synfig/layers/layer_composite_fork.h>
#include <synfig/color.h>
#include <synfig/vector.h>
#include <synfig/blur.h>

using namespace synfig;
using namespace std;
using namespace etl;

class Blur_Layer : public Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter: (synfig::Point)
	ValueBase param_size;
	//! Parameter: (int)
	ValueBase param_type;

public:
	Blur_Layer();

	virtual bool set_param(const String & param, const ValueBase &value);

	virtual ValueBase get_param(const String & param)const;

	virtual Color get_color(Context context, const Point &pos)const;

	virtual synfig::Rect get_full_bounding_rect(Context context)const;

	virtual Vocab get_param_vocab()const;

	virtual bool reads_context()const { return true; }

protected:
	virtual rendering::Task::Handle build_composite_fork_task_vfunc(ContextParams context_params, rendering::Task::Handle sub_task)const;
}; // END of class Blur

/* -- E X T E R N S --------------------------------------------------------- */

/* -- E N D ----------------------------------------------------------------- */

#endif
