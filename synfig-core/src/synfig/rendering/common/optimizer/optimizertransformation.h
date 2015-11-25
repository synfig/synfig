/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizertransformation.h
**	\brief OptimizerTransformation Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_OPTIMIZERTRANSFORMATION_H
#define __SYNFIG_RENDERING_OPTIMIZERTRANSFORMATION_H

/* === H E A D E R S ======================================================= */

#include "../../optimizer.h"
#include "../../task.h"
#include "../task/taskblend.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class OptimizerTransformation: public Optimizer
{
private:
	static bool can_optimize(const Task::Handle &sub_task);
	static void calc_unoptimized_blend_brunches(int &ref_count, const Task::Handle &blend_sub_task);

public:
	OptimizerTransformation()
	{
		category_id = CATEGORY_ID_COMMON;
		// TODO: is MODE_RECURSIVE actually needs?
		mode = MODE_REPEAT_LAST | MODE_RECURSIVE;
		for_task = true;
	}

	virtual void run(const RunParams &params) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
