/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/clcontext.h
**	\brief ClContext Header
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

#ifndef __SYNFIG_RENDERING_GL_CLCONTEXT_H
#define __SYNFIG_RENDERING_GL_CLCONTEXT_H

/* === H E A D E R S ======================================================= */

#include <CL/opencl.h>

#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

class ClContext
{
public:
	cl_context context;
	cl_device_id device;
	cl_command_queue queue;

	ClContext();
	~ClContext();

	cl_program load_program(const String &source);

private:
	static void callback(const char *, const void *, size_t, void *);
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
