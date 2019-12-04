/* === S Y N F I G ========================================================= */
/*!	\file layer_duplicate.h
**	\brief Header file for implementation of the "Duplicate" layer
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

#ifndef __SYNFIG_LAYER_DUPLICATE_H__
#define __SYNFIG_LAYER_DUPLICATE_H__

/* === H E A D E R S ======================================================= */

#include <synfig/valuenodes/valuenode_duplicate.h>
#include "layer_composite_fork.h"
#include <synfig/time.h>

/* === S T R U C T S & C L A S S E S ======================================= */

namespace synfig {

class Layer_Duplicate : public synfig::Layer_CompositeFork
{
	SYNFIG_LAYER_MODULE_EXT

private:
	mutable ValueBase param_index;
	mutable std::mutex mutex_;

public:

	Layer_Duplicate();

	//! Duplicates the Layer
	virtual Layer::Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;
	virtual bool set_param(const String & param, const synfig::ValueBase &value);
	virtual ValueBase get_param(const String & param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual ValueNode_Duplicate::Handle get_duplicate_param()const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual bool accelerated_cairorender(Context context, cairo_t *cr, int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
	virtual Vocab get_param_vocab()const;
	virtual bool reads_context()const { return true; }

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context) const;
}; // END of class Layer_Duplicate

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
