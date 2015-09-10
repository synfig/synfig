/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/renderergl.cpp
**	\brief RendererGL
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include "renderergl.h"

#include "internal/environment.h"

#include "../common/optimizer/optimizercomposite.h"
#include "../common/optimizer/optimizerlinear.h"
#include "../common/optimizer/optimizersurface.h"
#include "../common/optimizer/optimizersurfaceconvert.h"
#include "../common/optimizer/optimizersurfacecreate.h"
#include "../common/optimizer/optimizersurfacedestroy.h"
#include "../common/optimizer/optimizertransformation.h"

#include "optimizer/optimizercontourgl.h"

#include "../software/optimizer/optimizercontoursw.h"
#include "../software/optimizer/optimizerblendsw.h"
#include "../software/optimizer/optimizerblurpreparedsw.h"
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
	register_optimizer(new OptimizerTransformation());
	//register_optimizer(new OptimizerSurface());
	register_optimizer(new OptimizerSurfaceConvert());

	register_optimizer(new OptimizerBlendSW());
	register_optimizer(new OptimizerBlurPreparedSW());
	register_optimizer(new OptimizerContourSW());
	register_optimizer(new OptimizerMeshSW());

	register_optimizer(new OptimizerComposite());

	register_optimizer(new OptimizerLinear());
	register_optimizer(new OptimizerSurfaceCreate());
	//register_optimizer(new OptimizerSurfaceDestroy());
}

RendererGL::~RendererGL() { }

void
RendererGL::initialize() {
	gl::Environment::initialize();
}

void
RendererGL::deinitialize() {
	gl::Environment::deinitialize();
}


/* === E N T R Y P O I N T ================================================= */
