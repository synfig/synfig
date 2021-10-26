/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/tasklayer.cpp
**	\brief TaskLayer
**
**	\legal
**	......... ... 2016-2018 Ivan Mahonin
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

#include <synfig/context.h>
#include <synfig/layers/layer_rendering_task.h>

#include "tasklayer.h"
#include "tasktransformation.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Task::Token TaskLayer::token(
	DescAbstract<TaskLayer>("Contour") );


Rect
TaskLayer::calc_bounds() const
{
	if (!layer)
		return Rect::zero();

	etl::handle<Layer_RenderingTask> sub_layer(new Layer_RenderingTask());
	sub_layer->tasks.push_back(sub_task());

	CanvasBase fake_canvas_base;
	fake_canvas_base.push_back(layer);
	fake_canvas_base.push_back(sub_layer);
	fake_canvas_base.push_back(Layer::Handle());

	Context context(fake_canvas_base.begin(), ContextParams());
	return context.get_full_bounding_rect();
}

bool
TaskLayer::renddesc_less(const RendDesc &a, const RendDesc &b)
	{ return fabs(a.get_pw()*a.get_ph()) > fabs(b.get_pw()*b.get_ph()); }

void
TaskLayer::set_coords_sub_tasks()
{
	if (!sub_task())
		return;
	if (!is_valid_coords() || !layer)
		{ sub_task()->set_coords_zero(); return; }

	VectorInt size = target_rect.get_size();

	RendDesc desc;
	desc.set_wh(size[0], size[1]);
	desc.set_tl(source_rect.get_min());
	desc.set_br(source_rect.get_max());

	std::vector<RendDesc> descs;
	layer->get_sub_renddesc(desc, descs);
	sort(descs.begin(), descs.end(), renddesc_less);

	Task::Handle task = sub_task();
	sub_tasks.clear();

	for(std::vector<RendDesc>::const_iterator i = descs.begin(); i != descs.end(); ++i)
	{
		if (i->get_w() <= 0 || i->get_h() <= 0)
			continue;

		Point lt = i->get_tl(), rb = i->get_br();
		Rect rect(lt, rb);
		if (!rect.is_valid())
			continue;

		Matrix matrix;
		if (approximate_less(rb[0], lt[0]))
			{ matrix.m00 = -1.0; matrix.m20 = rb[0] - lt[0]; }
		if (approximate_less(rb[1], lt[1]))
			{ matrix.m11 = -1.0; matrix.m20 = rb[1] - lt[1]; }
		matrix = i->get_transformation_matrix() * matrix;
		if (!matrix.is_invertible())
			continue;

		Task::Handle t = task->clone();
		if (!matrix.is_identity()) {
			TaskTransformationAffine::Handle ta = new TaskTransformationAffine();
			ta->transformation->matrix = matrix;
			ta->sub_task() = t;
			t = ta;
		}

		sub_tasks.push_back(t);
		t->set_coords(rect, VectorInt(i->get_w(), i->get_h()));
	}
}

/* === E N T R Y P O I N T ================================================= */
