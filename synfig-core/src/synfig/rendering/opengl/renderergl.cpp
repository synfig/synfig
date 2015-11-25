/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/renderergl.cpp
**	\brief RendererGL
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

#include "renderergl.h"

#include "internal/environment.h"

#include "../common/optimizer/optimizercalcbounds.h"
#include "../common/optimizer/optimizerlinear.h"
#include "../common/optimizer/optimizersplit.h"
#include "../common/optimizer/optimizersurface.h"
#include "../common/optimizer/optimizersurfaceconvert.h"
#include "../common/optimizer/optimizersurfacecreate.h"
#include "../common/optimizer/optimizersurfacedestroy.h"
#include "../common/optimizer/optimizertransformation.h"
#include "../common/optimizer/optimizertransformationaffine.h"

#include "optimizer/optimizercontourgl.h"
#include "optimizer/optimizerblendgl.h"
#include "optimizer/optimizersurfaceresamplegl.h"

#include "../software/optimizer/optimizerblendsw.h"
#include "../software/optimizer/optimizercontoursw.h"
#include "../software/optimizer/optimizerlayersw.h"
#include "../software/optimizer/optimizermeshsw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

RendererGL::RendererGL()
{
	// register optimizers
	register_optimizer(new OptimizerTransformationAffine());
	register_optimizer(new OptimizerCalcBounds());

	register_optimizer(new OptimizerBlendGL());
	register_optimizer(new OptimizerContourGL());
	register_optimizer(new OptimizerLayerSW());
	register_optimizer(new OptimizerSurfaceResampleGL());

	register_optimizer(new OptimizerSurfaceConvert());

	register_optimizer(new OptimizerLinear());
	register_optimizer(new OptimizerSurfaceCreate());
	register_optimizer(new OptimizerSplit());
}

RendererGL::~RendererGL() { }

String RendererGL::get_name() const { return _("Cobra (hardware)"); }

void
RendererGL::initialize() {
	gl::Environment::initialize();
}

void
RendererGL::deinitialize() {
	gl::Environment::deinitialize();
}


/* === E N T R Y P O I N T ================================================= */
