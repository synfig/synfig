/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/renderersw.cpp
**	\brief RendererSW
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/localization.h>

#include "renderersw.h"

#include "../common/optimizer/optimizerblendassociative.h"
#include "../common/optimizer/optimizerblendblend.h"
#include "../common/optimizer/optimizerblendcomposite.h"
#include "../common/optimizer/optimizerblendseparate.h"
#include "../common/optimizer/optimizerblendsplit.h"
#include "../common/optimizer/optimizerblendzero.h"
#include "../common/optimizer/optimizercalcbounds.h"
#include "../common/optimizer/optimizerlinear.h"
#include "../common/optimizer/optimizerlist.h"
#include "../common/optimizer/optimizersplit.h"
#include "../common/optimizer/optimizersurface.h"
#include "../common/optimizer/optimizersurfaceconvert.h"
#include "../common/optimizer/optimizersurfacecreate.h"
#include "../common/optimizer/optimizersurfacedestroy.h"
#include "../common/optimizer/optimizersurfaceresample.h"
#include "../common/optimizer/optimizertransformation.h"
#include "../common/optimizer/optimizertransformationaffine.h"

#include "optimizer/optimizerblendsw.h"
#include "optimizer/optimizerblursw.h"
#include "optimizer/optimizercontoursw.h"
#include "optimizer/optimizerlayersw.h"
#include "optimizer/optimizermeshsw.h"
#include "optimizer/optimizersurfaceresamplesw.h"

#include "function/fft.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

RendererSW::RendererSW()
{
	// register optimizers
	register_optimizer(new OptimizerTransformationAffine());
	register_optimizer(new OptimizerSurfaceResample());
	register_optimizer(new OptimizerCalcBounds());

	register_optimizer(new OptimizerBlendSW());
	register_optimizer(new OptimizerBlurSW());
	register_optimizer(new OptimizerContourSW());
	register_optimizer(new OptimizerLayerSW());
	register_optimizer(new OptimizerSurfaceResampleSW());

	register_optimizer(new OptimizerBlendZero());
	register_optimizer(new OptimizerBlendBlend());
	register_optimizer(new OptimizerBlendComposite());
	register_optimizer(new OptimizerList());
	register_optimizer(new OptimizerBlendAssociative());
	register_optimizer(new OptimizerBlendSeparate());
	register_optimizer(new OptimizerBlendSplit());
	register_optimizer(new OptimizerSurfaceConvert());

	register_optimizer(new OptimizerLinear());
	register_optimizer(new OptimizerSurfaceCreate());
	//register_optimizer(new OptimizerSplit());
}

RendererSW::~RendererSW() { }

String RendererSW::get_name() const { return _("Cobra (software)"); }

void RendererSW::initialize()
{
	software::FFT::initialize();

}

void RendererSW::deinitialize()
{
	software::FFT::deinitialize();
}

/* === E N T R Y P O I N T ================================================= */
