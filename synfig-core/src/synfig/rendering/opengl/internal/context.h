/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/context.h
**	\brief Context Header
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**	......... ... 2023 Bharat Sahlot
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

#ifndef __SYNFIG_RENDERING_GL_CONTEXT_H
#define __SYNFIG_RENDERING_GL_CONTEXT_H

/* === H E A D E R S ======================================================= */
#include "headers.h"
#include "shaders.h"

#include <mutex>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{
namespace gl
{

// all internal objects assume a valid bound opengl context
class Context
{
public:
	class Lock {
	public:
		Lock(Context& ctx) : ctx(ctx) {
			ctx.use();
		}
		~Lock() {
			ctx.unuse();
		}
	private:
		Context& ctx;
	};

public:
	Context();
	~Context();

	bool initialize();

	gl::Programs::Program get_program(const std::string& str) const;
	gl::Programs::Program get_blend_program(Color::BlendMethod method) const;

	void use();
	void unuse();

private:
	std::recursive_mutex mutex;

	int lock_count = 0;

	bool initialized = false;

	GLFWwindow* glfwWindow = nullptr;

	Programs* programs = nullptr;

	Shaders* shaders = nullptr;
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
