/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/environment.cpp
**	\brief Environment
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "headers.h"
#include "environment.h"
#include "synfig/general.h"
#include <chrono>
#include <thread>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

gl::Environment* gl::Environment::instance = nullptr;

gl::Environment::Environment()
{
	valid = false;

	mainThread = std::thread([&]() {
		// HACK: If glfw context is created before GTK application then the GTK application fails to register
		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(2000));
		mainContext = new gl::Context(nullptr);
		valid = true;
	});
}

gl::Environment::~Environment()
{
	delete mainContext;
	for(auto x: contexts) delete x.second;
}

gl::Context& gl::Environment::get_or_create_context(std::thread::id id)
{
	if(contexts.count(id) != 0) return *contexts[id];

	Context* context = new Context(instance->mainContext);
	contexts[id] = context;
	return *context;
}
/* === E N T R Y P O I N T ================================================= */
