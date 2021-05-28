/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerdraft.h
**	\brief Draftt Optimizers Headers
**
**	\legal
**	......... ... 2017-2018 Ivan Mahonin
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
	OptimizerDraft();
};


class OptimizerDraftLowRes: public OptimizerDraft
{
public:
	const Real scale;
	explicit OptimizerDraftLowRes(Real scale);
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftTransformation: public OptimizerDraft
{
public:
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftContour: public OptimizerDraft
{
public:
	const Real detail;
	const bool antialias;
	OptimizerDraftContour(Real detail, bool antialias);
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftBlur: public OptimizerDraft
{
public:
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftLayerRemove: public OptimizerDraft
{
public:
	const String layername;
	explicit OptimizerDraftLayerRemove(const String &layername);
	virtual void run(const RunParams &params) const;
};


class OptimizerDraftLayerSkip: public OptimizerDraft
{
public:
	const String layername;
	explicit OptimizerDraftLayerSkip(const String &layername);
	virtual void run(const RunParams &params) const;
};


} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
