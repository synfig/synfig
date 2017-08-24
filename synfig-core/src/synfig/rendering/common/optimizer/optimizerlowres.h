/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerlowres.h
**	\brief OptimizerLowRes Header
**
**	$Id$
**
**	\legal
**	......... ... 2017 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_OPTIMIZELOWRES_H
#define __SYNFIG_RENDERING_OPTIMIZELOWRES_H

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

class OptimizerLowRes: public Optimizer
{
private:
	Real scale;

public:
	explicit OptimizerLowRes(Real scale): scale(scale)
	{
		category_id = CATEGORY_ID_PRE_SPECIALIZE;
		depends_from = CATEGORY_COMMON;
		for_root_task = true;
		deep_first = true;
	}

	virtual void run(const RunParams &params) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
