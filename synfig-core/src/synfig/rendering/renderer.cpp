/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderer.cpp
**	\brief Renderer
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

#include "renderer.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
Renderer::is_optimizer_registered(const Optimizer::Handle &optimizer) const
{
	for(Optimizer::List::const_iterator i = optimizers.begin(); i != optimizers.end(); ++i)
		if (*i == optimizer) return true;
	return false;
}

void
Renderer::register_optimizer(const Optimizer::Handle &optimizer)
{
	if (optimizer) {
		assert(!is_optimizer_registered(optimizer));
		optimizers.push_back(optimizer);
	}
}

void
Renderer::unregister_optimizer(const Optimizer::Handle &optimizer)
{
	for(Optimizer::List::const_iterator i = optimizers.begin(); i != optimizers.end();)
		if (*i == optimizer) i = optimizers.erase(i); else ++i;
}

void
Renderer::optimize(Task::List &list) const
{
	 for(Optimizer::List::const_iterator i = optimizers.begin(); i != optimizers.end();)
		 if ((*i)->run(Optimizer::RunParams(*this, list)))
			 i = optimizers.begin(); else ++i;
}

bool
Renderer::run(const Task::List &list) const
{
	Task::List optimized_list(list);
	optimize(optimized_list);

	bool success = true;

	Task::RunParams params(*this);
	for(Task::List::const_iterator i = optimized_list.begin(); i != optimized_list.end(); ++i)
		if (!(*i)->run(params)) success = false;

	return success;
}

/* === E N T R Y P O I N T ================================================= */
