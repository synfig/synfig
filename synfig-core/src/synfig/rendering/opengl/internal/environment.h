/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/enveironment.h
**	\brief Environment Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#ifndef __SYNFIG_RENDERING_GL_ENVIRONMENT_H
#define __SYNFIG_RENDERING_GL_ENVIRONMENT_H

/* === H E A D E R S ======================================================= */

#include <synfig/rendering/opengl/internal/samplers.h>
#include <cassert>

#include "antialiasing.h"
#include "buffers.h"
#include "context.h"
#ifdef WITH_OPENCL
#include "clcontext.h"
#endif
#include "framebuffers.h"
#include "misc.h"
#include "samplers.h"
#include "shaders.h"

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
#ifdef WITH_OPENCL
	ClContext clcontext;
#endif
	Samplers samplers;
	Buffers buffers;
	Shaders shaders;
	Antialiasing antialiasing;
	Framebuffers framebuffers;
	Misc misc;

	Environment():
		context(),
#ifdef WITH_OPENCL
		clcontext(),
#endif
		samplers(context),
		buffers(context),
		shaders(context),
		antialiasing(context),
		framebuffers(context),
		misc(context)
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
