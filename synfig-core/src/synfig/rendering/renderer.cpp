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

#include <typeinfo>

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/debug/measure.h>

#include "renderer.h"

#include "software/renderersw.h"
#include "opengl/renderergl.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer::Handle Renderer::blank;
std::map<String, Renderer::Handle> *Renderer::renderers;



void
Renderer::initialize_renderers()
{
	// initialize renderers
	RendererSW::initialize();
	RendererGL::initialize();

	// register renderers
	register_renderer("software", new RendererSW());
	register_renderer("gl", new RendererGL());
}

void
Renderer::deinitialize_renderers()
{
	RendererGL::deinitialize();
	RendererSW::deinitialize();
}



Renderer::~Renderer() { }

bool
Renderer::is_optimizer_registered(const Optimizer::Handle &optimizer) const
{
	if (!optimizer)
		for(Optimizer::List::const_iterator i = optimizers[optimizer->category_id].begin(); i != optimizers[optimizer->category_id].end(); ++i)
			if (*i == optimizer) return true;
	return false;
}

void
Renderer::register_optimizer(const Optimizer::Handle &optimizer)
{
	if (optimizer) {
		assert(!is_optimizer_registered(optimizer));
		optimizers[optimizer->category_id].push_back(optimizer);
	}
}

void
Renderer::unregister_optimizer(const Optimizer::Handle &optimizer)
{
	for(Optimizer::List::iterator i = optimizers[optimizer->category_id].begin(); i != optimizers[optimizer->category_id].end();)
		if (*i == optimizer) i = optimizers[optimizer->category_id].erase(i); else ++i;
}

void
Renderer::optimize_recursive(const Optimizer::List &optimizers, const Optimizer::RunParams& params, bool first_level_only) const
{
	if (!params.ref_task) return;
	if (params.ref_affects_to & params.depends_from) return;

	for(Optimizer::List::const_iterator i = optimizers.begin(); i != optimizers.end(); ++i)
	{
		if ((*i)->for_task || ((*i)->for_root_task && !params.parent))
		{
			Optimizer::RunParams p(params);
			(*i)->run(p);
			params.ref_affects_to |= p.ref_affects_to;
			params.ref_mode |= p.ref_mode;
			params.ref_task = p.ref_task;
			if (!params.ref_task) return;
			if (params.ref_affects_to & params.depends_from) return;
		}
	}

	if (first_level_only) return;

	bool task_clonned = false;
	for(Task::List::iterator i = params.ref_task->sub_tasks.begin(); i != params.ref_task->sub_tasks.end();)
	{
		if (*i)
		{
			Optimizer::RunParams sub_params = params.sub(*i);
			optimize_recursive(optimizers, sub_params, false);
			if (sub_params.ref_task != *i)
			{
				if (!task_clonned)
				{
					int index = i - params.ref_task->sub_tasks.begin();
					params.ref_task = params.ref_task->clone();
					i = params.ref_task->sub_tasks.begin() + index;
					task_clonned = true;
				}
				*i = sub_params.ref_task;
				if (!(sub_params.ref_mode & Optimizer::MODE_REPEAT)) ++i;
			} else ++i;
			params.ref_affects_to |= sub_params.ref_affects_to;
			if (sub_params.ref_mode & Optimizer::MODE_REPEAT_BRUNCH)
				params.ref_mode |= sub_params.ref_mode;
			if (params.ref_affects_to & params.depends_from) return;
		} else ++i;
	}
}

void
Renderer::optimize(Task::List &list) const
{
	//debug::Measure t("Renderer::optimize");

	int current_category_id = 0;
	int current_optimizer_index = 0;
	Optimizer::Category current_affected = 0;
	Optimizer::Category categories_to_process = Optimizer::CATEGORY_ALL;
	Optimizer::List single(1);

	while(categories_to_process &= Optimizer::CATEGORY_ALL)
	{
		if (current_category_id >= Optimizer::CATEGORY_ID_COUNT)
		{
			current_category_id = 0;
			current_optimizer_index = 0;
			current_affected = 0;
			continue;
		}

		if (!((1 << current_category_id) & categories_to_process))
		{
			++current_category_id;
			current_optimizer_index = 0;
			current_affected = 0;
			continue;
		}

		if (current_optimizer_index >= (int)optimizers[current_category_id].size())
		{
			categories_to_process &= ~(1 << current_category_id);
			categories_to_process |= current_affected;
			++current_category_id;
			current_optimizer_index = 0;
			current_affected = 0;
			continue;
		}

		bool simultaneous_run = Optimizer::categories_info[current_category_id].simultaneous_run;
		const Optimizer::List &current_optimizers = simultaneous_run ? optimizers[current_category_id] : single;
		if (!simultaneous_run) {
			single.front() = optimizers[current_category_id][current_optimizer_index];
			Optimizer::Category depends_from_self = (1 << current_category_id) & single.front()->depends_from;
			if (current_affected & depends_from_self)
			{
				current_category_id = 0;
				current_optimizer_index = 0;
				current_affected = 0;
				continue;
			}
		}

		Optimizer::Category depends_from = 0;
		bool for_list = false;
		bool for_task = false;
		bool for_root_task = false;
		for(Optimizer::List::const_iterator i = current_optimizers.begin(); i != current_optimizers.end(); ++i)
		{
			depends_from |= ((1 << current_category_id) - 1) & (*i)->depends_from;
			if ((*i)->for_list) for_list = true;
			if ((*i)->for_task) for_task = true;
			if ((*i)->for_root_task) for_root_task = true;
		}

		//info("optimize category %d index %d", current_category_id, current_optimizer_index);
		//debug::Measure t(etl::strprintf("optimize category %d index %d", current_category_id, current_optimizer_index));

		if (for_list)
		{
			for(Optimizer::List::const_iterator i = current_optimizers.begin(); !(categories_to_process & depends_from) && i != current_optimizers.end(); ++i)
			{
				if ((*i)->for_list)
				{
					Optimizer::RunParams params(*this, list, depends_from);
					(*i)->run(params);
					categories_to_process |= current_affected |= params.ref_affects_to;
				}
			}
		}

		if (for_task || for_root_task)
		{
			for(Task::List::iterator j = list.begin(); !(categories_to_process & depends_from) && j != list.end();)
			{
				if (*j)
				{
					Optimizer::RunParams params(*this, list, depends_from, *j);
					optimize_recursive(current_optimizers, params, !for_task);
					if (*j != params.ref_task)
					{
						if (params.ref_task)
						{
							*j = params.ref_task;
							if (!(params.ref_mode & Optimizer::MODE_REPEAT)) ++j;
						}
						else
							j = list.erase(j);
					} else ++j;
					categories_to_process |= current_affected |= params.ref_affects_to;
				}
				else
				{
					j = list.erase(j);
				}
			}
		}

		if (categories_to_process & depends_from)
		{
			current_category_id = 0;
			current_optimizer_index = 0;
			current_affected = 0;
			continue;
		}

		current_optimizer_index += current_optimizers.size();
	}

	// remove nulls
	for(Task::List::iterator j = list.begin(); j != list.end();)
		if (*j) ++j; else j = list.erase(j);
}

bool
Renderer::run(const Task::List &list) const
{
	/*
	{
		for(int i = 0; i < Color::BLEND_END; ++i)
		{
			int count = 0;
			int count1 = 0;
			int count2 = 0;
			int count3 = 0;
			for(int j = 0; j < 1000000; ++j)
			{
				Color colors[6];
				Color::value_type *channels = (Color::value_type*)&colors[0];
				for(int k = 0; k < 16; ++k)
					channels[k] = (float)rand()/(float)RAND_MAX*10.f - 5.f;

				Color x, y;
				Color::value_type a0, a1;

				a0 = 1.f;
				a1 = 1.f;
				x = Color::blend(colors[2], Color::blend(colors[1], colors[0], a0, (Color::BlendMethod)i), a0*a1, (Color::BlendMethod)i);
				y = Color::blend(Color::blend(colors[2], colors[1], a1, (Color::BlendMethod)i), colors[0], a0, (Color::BlendMethod)i);
				if ( fabsf(x.get_r() - y.get_r()) > 1e-3
				  || fabsf(x.get_g() - y.get_g()) > 1e-3
				  || fabsf(x.get_b() - y.get_b()) > 1e-3
				  || fabsf(x.get_a() - y.get_a()) > 1e-3 )
					++count;

				a0 = channels[12];
				a1 = channels[13];
				x = Color::blend(colors[2], Color::blend(colors[1], colors[0], a0, (Color::BlendMethod)i), a0*a1, (Color::BlendMethod)i);
				y = Color::blend(Color::blend(colors[2], colors[1], a1, (Color::BlendMethod)i), colors[0], a0, (Color::BlendMethod)i);
				if ( fabsf(x.get_r() - y.get_r()) > 1e-3
				  || fabsf(x.get_g() - y.get_g()) > 1e-3
				  || fabsf(x.get_b() - y.get_b()) > 1e-3
				  || fabsf(x.get_a() - y.get_a()) > 1e-3 )
					++count1;

				a0 = 1.f;
				a1 = channels[13];
				x = Color::blend(colors[2], Color::blend(colors[1], colors[0], a0, (Color::BlendMethod)i), a0*a1, (Color::BlendMethod)i);
				y = Color::blend(Color::blend(colors[2], colors[1], a1, (Color::BlendMethod)i), colors[0], a0, (Color::BlendMethod)i);
				if ( fabsf(x.get_r() - y.get_r()) > 1e-3
				  || fabsf(x.get_g() - y.get_g()) > 1e-3
				  || fabsf(x.get_b() - y.get_b()) > 1e-3
				  || fabsf(x.get_a() - y.get_a()) > 1e-3 )
					++count2;

				a0 = channels[12];
				a1 = 1.f;
				x = Color::blend(colors[2], Color::blend(colors[1], colors[0], a0, (Color::BlendMethod)i), a0*a1, (Color::BlendMethod)i);
				y = Color::blend(Color::blend(colors[2], colors[1], a1, (Color::BlendMethod)i), colors[0], a0, (Color::BlendMethod)i);
				if ( fabsf(x.get_r() - y.get_r()) > 1e-3
				  || fabsf(x.get_g() - y.get_g()) > 1e-3
				  || fabsf(x.get_b() - y.get_b()) > 1e-3
				  || fabsf(x.get_a() - y.get_a()) > 1e-3 )
					++count3;
			}
			info("association %d %d %d %d %d", i, count, count1, count2, count3);
		}
	}
	*/

	//debug::Measure t("Renderer::run");

	log(list, "input list");
	Task::List optimized_list(list);
	{
		//debug::Measure t("optimize");
		optimize(optimized_list);
	}
	log(optimized_list, "optimized list");

	bool success = true;

	{
		//debug::Measure t("run tasks");
		Task::RunParams params;
		for(Task::List::iterator i = optimized_list.begin(); i != optimized_list.end(); ++i)
		{
			//debug::Measure t(typeid(**i).name() + 19);

			// prepare params
			params.used_rect.minx = 0;
			params.used_rect.miny = 0;
			params.used_rect.maxx = (*i)->target_surface ? (*i)->target_surface->get_width() : 0;
			params.used_rect.maxy = (*i)->target_surface ? (*i)->target_surface->get_height() : 0;

			// run
			if (!(*i)->run(params))
				success = false;

			// update used rect for target surface
			if (params.used_rect.valid())
			{
				etl::rect<int> &target_rect = (*i)->target_surface->used_rect;
				if (target_rect.valid())
					etl::set_union(target_rect, target_rect, params.used_rect);
				else
					target_rect = params.used_rect;
			}

			// remove task
			i->reset();
		}
	}
	// optimized_list now contains NULLs, clear it to avoid any mistakes
	optimized_list.clear();

	return success;
}

void
Renderer::log(const Task::Handle &task, const String &prefix) const
{
	if (task)
	{
		info( prefix
			+ (typeid(*task).name() + 19)
			+ ( task->rect_lt[0] && task->rect_lt[1]
			 && task->rect_rb[0] && task->rect_rb[1]
              ? etl::strprintf(" rect (%f, %f)-(%f, %f)",
				task->rect_lt[0], task->rect_lt[1],
				task->rect_rb[0], task->rect_rb[1] )
		      : "" )
			+ ( task->target_surface
			  ?	etl::strprintf(" target %s (%dx%d) 0x%x",
					(typeid(*task->target_surface).name() + 19),
					task->target_surface->get_width(),
					task->target_surface->get_height(),
					task->target_surface.get() )
			  : "" ));
		for(Task::List::const_iterator i = task->sub_tasks.begin(); i != task->sub_tasks.end(); ++i)
			log(*i, prefix + "  ");
	}
	else
	{
		info(prefix + " NULL");
	}
}

void
Renderer::log(const Task::List &list, const String &name) const
{
	String line = "-------------------------------------------";
	String n = "    " + name;
	n.resize(line.size(), ' ');
	for(int i = 0; i < (int)line.size(); ++i)
		if (n[i] == ' ') n[i] = line[i];
	info(n);
	for(Task::List::const_iterator i = list.begin(); i != list.end(); ++i)
		log(*i);
	info(line);
}

void
Renderer::initialize()
{
	if (renderers != NULL)
		synfig::error("rendering::Renderer already initialized");
	renderers = new std::map<String, Handle>();

	initialize_renderers();
}

void
Renderer::deinitialize()
{
	while(!get_renderers().empty())
		unregister_renderer(get_renderers().begin()->first);

	deinitialize_renderers();

	delete renderers;
}

void
Renderer::register_renderer(const String &name, const Renderer::Handle &renderer)
{
	if (get_renderer(name))
		synfig::error("rendering::Renderer renderer '%s' already registered", name.c_str());
	(*renderers)[name] = renderer;
}

void
Renderer::unregister_renderer(const String &name)
{
	if (!get_renderer(name))
		synfig::error("rendering::Renderer renderer '%s' not registered", name.c_str());
	renderers->erase(name);
}

const Renderer::Handle&
Renderer::get_renderer(const String &name)
{
	return get_renderers().count(name) > 0
		 ? get_renderers().find(name)->second
		 : blank;
}

const std::map<String, Renderer::Handle>&
Renderer::get_renderers()
{
	if (renderers == NULL)
		synfig::error("rendering::Renderer not initialized");
	return *renderers;
}

/* === E N T R Y P O I N T ================================================= */
