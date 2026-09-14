/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/environment.h
**	\brief Environment Header
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

#ifndef __SYNFIG_RENDERING_GL_ENVIRONMENT_H
#define __SYNFIG_RENDERING_GL_ENVIRONMENT_H

/* === H E A D E R S ======================================================= */

#include "context.h"
#include "shaders.h"

#include <cassert>
#include <thread>
#include <mutex>

#include <map>

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
public:
	Environment();
	~Environment();

	static Environment& get_instance()
	{
		assert(is_valid(instance));
		return *instance;
	}

	static void initialize()
	{
		assert(!is_valid(instance));
		instance = new Environment();
	}

	static void deinitialize()
	{
		delete instance;
	}

	static bool is_valid(Environment* instance)
	{
		return instance && instance->valid;
	}

	// Context& get_or_create_context(std::thread::id id);

	Context& get_or_create_context();

private:
	std::mutex mutex;

	bool valid;
	static Environment* instance;

	// std::thread mainThread;
	Context* mainContext = nullptr;

	// Shaders* shaders = nullptr;

	// std::map<std::thread::id, Context*> contexts;
};

}; /* end namespace gl */
}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
