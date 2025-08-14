/* === S Y N F I G ========================================================= */
/*!	\file insideout.h
**	\brief Header file for implementation of the "Inside Out" layer
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_INSIDEOUT_H
#define __SYNFIG_INSIDEOUT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>
#include <synfig/context.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace modules
{
namespace lyr_std
{

class InsideOut_Trans;

class InsideOut : public Layer
{
	SYNFIG_LAYER_MODULE_EXT
	friend class InsideOut_Trans;

private:
	//!Parameter: (Point)
	ValueBase param_origin;
	ValueBase param_cobra;
	ValueBase param_draft;

public:
	InsideOut();

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	Layer::Handle hit_check(Context context, const Point &point)const;
	virtual Vocab get_param_vocab()const;
	virtual etl::handle<Transform> get_transform()const;
	virtual bool reads_context() const { return true; }

protected:
	rendering::Task::Handle build_rendering_task_vfunc(Context context) const override;
};

}; // END of namespace lyr_std
}; // END of namespace modules
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
