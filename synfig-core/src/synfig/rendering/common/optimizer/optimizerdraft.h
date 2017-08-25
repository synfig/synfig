/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerdraft.h
**	\brief Draftt Optimizers Headers
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

#ifndef __SYNFIG_RENDERING_OPTIMIZERDRAFT_H
#define __SYNFIG_RENDERING_OPTIMIZERDRAFT_H

/* === H E A D E R S ======================================================= */

#include "../../optimizer.h"
#include "../../task.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{


class OptimizerDraft: public Optimizer
{
public:
	OptimizerDraft()
	{
		category_id = CATEGORY_ID_COMMON;
		for_task = true;
	}
};


class OptimizerDraftLowRes: public OptimizerDraft
{
private:
	Real scale;
public:
	explicit OptimizerDraftLowRes(Real scale): scale(scale)
	{
		category_id = CATEGORY_ID_PRE_SPECIALIZE;
		depends_from = CATEGORY_COMMON;
		for_root_task = true;
		for_task = false;
		deep_first = true;
	}
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftResample: public OptimizerDraft
{
public:
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftContour: public OptimizerDraft
{
public:
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftBlur: public OptimizerDraft
{
public:
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftLayerRemove: public OptimizerDraft
{
private:
	String layername;
public:
	OptimizerDraftLayerRemove(const String &layername): layername(layername) { }
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftLayerSkip: public OptimizerDraft
{
private:
	String layername;
public:
	OptimizerDraftLayerSkip(const String &layername): layername(layername) { }
	virtual void run(const RunParams &params) const;
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
