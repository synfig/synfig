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

#include <cstdio>

#include <map>

#include "optimizer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class RenderQueue;

class Renderer: public etl::shared_object
{
public:
	typedef etl::handle<Renderer> Handle;

	struct DebugOptions {
		String task_list_log;
		String task_list_optimized_log;
		String result_image;
	};

private:
	static Handle blank;
	static std::map<String, Handle> *renderers;
	static RenderQueue *queue;
	static DebugOptions debug_options;

	Optimizer::List optimizers[Optimizer::CATEGORY_ID_COUNT];

public:

	virtual ~Renderer();

	virtual String get_name() const = 0;

	const Optimizer::List& get_optimizers(Optimizer::CategoryId category_id) const { return optimizers[category_id]; }
	bool is_optimizer_registered(const Optimizer::Handle &optimizer) const;
	void register_optimizer(const Optimizer::Handle &optimizer);
	void unregister_optimizer(const Optimizer::Handle &optimizer);

private:
	void optimize_recursive(
		const Optimizer::List &optimizers,
		const Optimizer::RunParams& params,
		int &calls_count,
		int &optimizations_count,
		int max_level ) const;

	void log(
		const String &logfile,
		const Task::Handle &task,
		const Optimizer::RunParams* optimization_stack = NULL,
		int level = 0 ) const;
	void log(
		const String &logfile,
		const Task::List &list,
		const String &name = String(),
		const Optimizer::RunParams* optimization_stack = NULL ) const;

	static void initialize_renderers();
	static void deinitialize_renderers();

public:
	int get_max_simultaneous_threads() const;
	void optimize(Task::List &list) const;
	bool run(const Task::List &list) const;

	static void initialize();
	static void deinitialize();
	static void register_renderer(const String &name, const Renderer::Handle &renderer);
	static void unregister_renderer(const String &name);
	static const Renderer::Handle& get_renderer(const String &name);
	static const std::map<String, Handle>& get_renderers();

	static const DebugOptions& get_debug_options()
		{ return debug_options; }

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
