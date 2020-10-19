/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/rendererdraftsw.cpp
**	\brief RendererDraftSW
**
**	$Id$
**
**	\legal
**	......... ... 2017 Ivan Mahonin
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
	register_optimizer(new OptimizerDraftContour(2.0, true));
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
	register_optimizer(new OptimizerDraftLayerSkip("text"));
	register_optimizer(new OptimizerDraftLayerSkip("xor_pattern"));

	register_optimizer(new OptimizerTransformation());
	register_optimizer(new OptimizerDraftTransformation());

	register_optimizer(new OptimizerPass(false));
	register_optimizer(new OptimizerPass(true));
	register_optimizer(new OptimizerBlendMerge());
	register_optimizer(new OptimizerBlendToTarget());
	register_optimizer(new OptimizerList());
	register_optimizer(new OptimizerBlendAssociative());
	//register_optimizer(new OptimizerSplit());
}

String RendererDraftSW::get_name() const
{
	return _("Cobra Draft (software)");
}

/* === E N T R Y P O I N T ================================================= */
