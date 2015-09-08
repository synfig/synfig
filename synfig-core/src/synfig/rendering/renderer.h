/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderer.h
**	\brief Renderer Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_RENDERER_H
#define __SYNFIG_RENDERING_RENDERER_H

/* === H E A D E R S ======================================================= */

#include <map>

#include "optimizer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Renderer: public etl::shared_object
{
public:
	typedef etl::handle<Renderer> Handle;

private:
	static Handle blank;
	static std::map<String, Handle> *renderers;

	Optimizer::List optimizers;

public:

	virtual ~Renderer();

	const Optimizer::List& get_optimizers() const { return optimizers; }
	bool is_optimizer_registered(const Optimizer::Handle &optimizer) const;
	void register_optimizer(const Optimizer::Handle &optimizer);
	void unregister_optimizer(const Optimizer::Handle &optimizer);

private:
	bool optimize_recursive(const Optimizer &optimizer, const Optimizer::RunParams& params) const;
	bool optimize_recursive(const Optimizer &optimizer, Task::List &list) const;
	void log(const Task::Handle &task, const String &prefix = String()) const;
	void log(const Task::List &list, const String &name = String()) const;

public:
	void optimize(Task::List &list) const;
	bool run(const Task::List &list) const;

	static void initialize();
	static void deinitialize();
	static void register_renderer(const String &name, const Renderer::Handle &renderer);
	static void unregister_renderer(const String &name);
	static const Renderer::Handle& get_renderer(const String &name);
	static const std::map<String, Handle>& get_renderers();

	static bool subsys_init()
	{
		initialize();
		return true;
	}

	static bool subsys_stop()
	{
		deinitialize();
		return false;
	}
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
