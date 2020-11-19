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

#include <synfig/localization.h>

#include "renderergl.h"

#include "task/taskgl.h"

#include "internal/environment.h"

#include "../common/optimizer/optimizertransformation.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

RendererGL::RendererGL()
{
	register_mode(TaskGL::mode_token.handle());

	// register optimizers
	register_optimizer(new OptimizerTransformation());
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
