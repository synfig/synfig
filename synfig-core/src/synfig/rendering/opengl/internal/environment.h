/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/enveironment.h
**	\brief Environment Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_GL_ENVIRONMENT_H
#define __SYNFIG_RENDERING_GL_ENVIRONMENT_H

/* === H E A D E R S ======================================================= */

#include <cassert>

#include "context.h"
#include "clcontext.h"
#include "buffers.h"
#include "shaders.h"
#include "antialiasing.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

class Environment
{
private:
	static Environment *instance;

public:
	Context context;
	ClContext clcontext;
	Buffers buffers;
	Shaders shaders;
	Antialiasing antialiasing;

	Environment():
		context(),
		clcontext(),
		buffers(context),
		shaders(context),
		antialiasing(context)
	{ }

	static Environment& get_instance()
		{ assert(instance); return *instance; }
	static void initialize()
		{ assert(!instance); instance = new Environment(); }
	static void deinitialize()
		{ assert(instance); delete instance; }
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
