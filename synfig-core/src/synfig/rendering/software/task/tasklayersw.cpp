/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/tasklayersw.cpp
**	\brief TaskLayerSW
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include <synfig/guid.h>
#include <synfig/canvas.h>
#include <synfig/context.h>

#include <synfig/layers/layer_rendering_task.h>

#include "../../common/task/tasklayer.h"
#include "tasksw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskLayerSW: public TaskLayer, public TaskSW
{
public:
	typedef etl::handle<TaskLayerSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams&) const {
		if (!is_valid() || !layer)
			return false;

		Vector upp = get_units_per_pixel();
		Vector lt = source_rect.get_min();
		Vector rb = source_rect.get_max();
		lt[0] -= target_rect.minx*upp[0];
		lt[1] -= target_rect.miny*upp[1];
		rb[0] += (target_surface->get_width() - target_rect.maxx)*upp[0];
		rb[1] += (target_surface->get_height() - target_rect.maxy)*upp[1];

		RendDesc desc;
		desc.set_tl(lt);
		desc.set_br(rb);
		desc.set_wh(target_surface->get_width(), target_surface->get_height());
		desc.set_antialias(1);

		etl::handle<Layer_RenderingTask> sub_layer(new Layer_RenderingTask());
		sub_layer->tasks = sub_tasks;

		CanvasBase fake_canvas_base;
		fake_canvas_base.push_back(layer);
		fake_canvas_base.push_back(sub_layer);
		fake_canvas_base.push_back(Layer::Handle());

		Context context(fake_canvas_base.begin(), ContextParams());

		LockWrite ldst(this);
		if (!ldst)
			return false;

		return context.accelerated_render(&ldst->get_surface(), 4, desc, NULL);
	}
};


Task::Token TaskLayerSW::token(
	DescReal<TaskLayerSW, TaskLayer>("LayerSW") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
