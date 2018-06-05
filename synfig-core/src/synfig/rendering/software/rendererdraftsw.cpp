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

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/localization.h>

#include "rendererdraftsw.h"

#include "task/tasksw.h"

#include "../common/optimizer/optimizerblendassociative.h"
#include "../common/optimizer/optimizerblendblend.h"
#include "../common/optimizer/optimizerblendcomposite.h"
#include "../common/optimizer/optimizerblendseparate.h"
#include "../common/optimizer/optimizerblendsplit.h"
#include "../common/optimizer/optimizerblendzero.h"
#include "../common/optimizer/optimizerdraft.h"
#include "../common/optimizer/optimizerlist.h"
#include "../common/optimizer/optimizerpixelprocessorsplit.h"
#include "../common/optimizer/optimizersplit.h"
#include "../common/optimizer/optimizertransformation.h"
#include "../common/optimizer/optimizertransformationaffine.h"

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
	register_optimizer(new OptimizerDraftContour());
	register_optimizer(new OptimizerDraftBlur());
	register_optimizer(new OptimizerDraftLayerSkip("MotionBlur"));
	register_optimizer(new OptimizerDraftLayerSkip("radial_blur"));
	register_optimizer(new OptimizerDraftLayerSkip("curve_warp"));
	register_optimizer(new OptimizerDraftLayerSkip("inside_out"));
	register_optimizer(new OptimizerDraftLayerSkip("noise_distort"));
	register_optimizer(new OptimizerDraftLayerSkip("spherize"));
	register_optimizer(new OptimizerDraftLayerSkip("twirl"));
	register_optimizer(new OptimizerDraftLayerSkip("warp"));
	register_optimizer(new OptimizerDraftLayerSkip("metaballs"));
	register_optimizer(new OptimizerDraftLayerSkip("clamp"));
	register_optimizer(new OptimizerDraftLayerSkip("colorcorrect"));
	register_optimizer(new OptimizerDraftLayerSkip("halftone2"));
	register_optimizer(new OptimizerDraftLayerSkip("halftone3"));
	register_optimizer(new OptimizerDraftLayerSkip("lumakey"));
	register_optimizer(new OptimizerDraftLayerSkip("julia"));
	register_optimizer(new OptimizerDraftLayerSkip("mandelbrot"));
	register_optimizer(new OptimizerDraftLayerSkip("conical_gradient"));
	register_optimizer(new OptimizerDraftLayerSkip("curve_gradient"));
	register_optimizer(new OptimizerDraftLayerSkip("noise"));
	register_optimizer(new OptimizerDraftLayerSkip("spiral_gradient"));
	register_optimizer(new OptimizerDraftLayerSkip("duplicate"));
	register_optimizer(new OptimizerDraftLayerSkip("plant"));
	register_optimizer(new OptimizerDraftLayerSkip("super_sample"));
	register_optimizer(new OptimizerDraftLayerSkip("text"));
	register_optimizer(new OptimizerDraftLayerSkip("xor_pattern"));
	register_optimizer(new OptimizerTransformationAffine());
	register_optimizer(new OptimizerDraftResample());

	register_optimizer(new OptimizerBlendZero());
	register_optimizer(new OptimizerBlendBlend());
	register_optimizer(new OptimizerBlendComposite());
	register_optimizer(new OptimizerList());
	register_optimizer(new OptimizerBlendAssociative());
	register_optimizer(new OptimizerBlendSeparate());
	register_optimizer(new OptimizerBlendSplit());
	register_optimizer(new OptimizerPixelProcessorSplit());

	//register_optimizer(new OptimizerSplit());
}

String RendererDraftSW::get_name() const
{
	return _("Cobra Draft (software)");
}

/* === E N T R Y P O I N T ================================================= */
