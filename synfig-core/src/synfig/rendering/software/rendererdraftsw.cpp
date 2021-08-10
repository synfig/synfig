/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/rendererdraftsw.cpp
**	\brief RendererDraftSW
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>

#include "rendererdraftsw.h"

#include "task/tasksw.h"

#include "../common/optimizer/optimizerblendassociative.h"
#include "../common/optimizer/optimizerblendmerge.h"
#include "../common/optimizer/optimizerblendtotarget.h"
#include "../common/optimizer/optimizerdraft.h"
#include "../common/optimizer/optimizerlist.h"
#include "../common/optimizer/optimizersplit.h"
#include "../common/optimizer/optimizertransformation.h"
#include "../common/optimizer/optimizerpass.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

RendererDraftSW::RendererDraftSW()
{
	register_mode(TaskSW::mode_token.handle());

	// register optimizers
	register_optimizer(std::make_shared<OptimizerDraftContour>(2.0, true));
	register_optimizer(std::make_shared<OptimizerDraftBlur>());
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("MotionBlur"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("radial_blur"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("curve_warp"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("inside_out"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("noise_distort"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("spherize"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("twirl"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("warp"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("metaballs"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("clamp"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("colorcorrect"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("halftone2"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("halftone3"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("lumakey"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("julia"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("mandelbrot"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("conical_gradient"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("curve_gradient"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("noise"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("spiral_gradient"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("duplicate"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("plant"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("text"));
	register_optimizer(std::make_shared<OptimizerDraftLayerSkip>("xor_pattern"));

	register_optimizer(std::make_shared<OptimizerTransformation>());
	register_optimizer(std::make_shared<OptimizerDraftTransformation>());

	register_optimizer(std::make_shared<OptimizerPass>(false));
	register_optimizer(std::make_shared<OptimizerPass>(true));
	register_optimizer(std::make_shared<OptimizerBlendMerge>());
	register_optimizer(std::make_shared<OptimizerBlendToTarget>());
	register_optimizer(std::make_shared<OptimizerList>());
	register_optimizer(std::make_shared<OptimizerBlendAssociative>());
	//register_optimizer(new OptimizerSplit());
}

String RendererDraftSW::get_name() const
{
	return _("Cobra Draft (software)");
}

/* === E N T R Y P O I N T ================================================= */
