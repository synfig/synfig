/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/misc.cpp
**	\brief Misc
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "misc.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

gl::Misc::Misc(Context &context):
	context(context)
{
	Context::Lock lock(context);
	//
}

gl::Misc::~Misc()
{
	Context::Lock lock(context);
	//
}

void
gl::Misc::clear(const Color &color)
{
	Context::Lock lock(context);
	GLclampf prev[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, prev);
	glClearColor(color.get_r(), color.get_g(), color.get_b(), color.get_a());
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(prev[0], prev[1], prev[2], prev[3]);
}


/* === E N T R Y P O I N T ================================================= */
