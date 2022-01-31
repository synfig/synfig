/* === S Y N F I G ========================================================= */
/*!	\file renderer_bonedeformarea.cpp
 **	\brief Renderer for influence area Skeleton Deformation Bones
 **
 **	\legal
 **	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
 **	Copyright (c) 2021 Rodolfo Ribeiro Gomes
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

#include "renderer_bonedeformarea.h"

#include <cassert>

#include <gui/canvasview.h>
#include <gui/workarea.h>

#include <synfig/bone.h>
#include <synfig/pair.h>
#include <synfig/valuenodes/valuenode_bone.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_BoneDeformArea::~Renderer_BoneDeformArea()
{
}

bool
Renderer_BoneDeformArea::get_enabled_vfunc()const
{
	return true;
}

void
Renderer_BoneDeformArea::render_vfunc(const Glib::RefPtr<Gdk::Window>& drawable,
								 const Gdk::Rectangle& /*expose_area*/ )
{
	assert(get_work_area());
	if(!get_work_area())
		return;

	Cairo::RefPtr<Cairo::Context> cr = drawable->create_cairo_context();
	Canvas::Handle canvas(get_work_area()->get_canvas());

	auto layer_selection = get_work_area()->get_canvas_view()->canvas_interface()->get_selection_manager()->get_selected_layers();
	const Gdk::RGBA c("#8f0000");
	const Real pw = get_pw();
	cr->set_source_rgba(c.get_red(), c.get_green(), c.get_blue(), 0.3);

	for (const auto& layer : layer_selection) {
		if (!layer->active() && layer->get_name() == "skeleton_deformation") {
			std::vector<std::pair<Bone,Bone>> bone_pair_list;
			auto it = layer->dynamic_param_list().find("bones");
			if (it != layer->dynamic_param_list().end()) {
				bone_pair_list = (*it->second)(canvas->get_time()).get_list_of(std::pair<Bone,Bone>());
			} else {
				auto bones = layer->get_param("bones");
				bone_pair_list = bones.get_list_of(std::pair<Bone,Bone>());
			}
			for (const auto& bone_pair : bone_pair_list) {
				Bone::Shape shape = bone_pair.first.get_shape();
				Real angle = Angle::rad(bone_pair.first.get_angle()).get();
				ValueNode_Bone::ConstHandle parent = bone_pair.first.get_parent();
				while (!parent->is_root()) {
					Bone b = (*parent)(canvas->get_time()).get(Bone());
					angle += Angle::rad(b.get_angle()).get();
					parent = b.get_parent();
				}
				shape.p0 = comp_to_screen_coords(shape.p0);
				shape.p1 = comp_to_screen_coords(shape.p1);
				shape.r0 /= pw;
				shape.r1 /= pw;

				const Real dr = shape.r1 - shape.r0;
				const Real length = (shape.p1 - shape.p0).mag();
				const Real dist2 = length*length - dr*dr;
				if (dist2 < 0) {
					if (shape.r0 > shape.r1)
						cr->arc(shape.p0[0], shape.p0[1], shape.r0, 0, 2*PI);
					else
						cr->arc(shape.p1[0], shape.p1[1], shape.r1, 0, 2*PI);
				} else {
					const Real da = PI - atan2(sqrt(dist2), dr);
					// cr->move_to(shape.p0[0]+shape.r0*std::cos(-angle+da), shape.p0[1]+shape.r0*std::sin(-angle+da));
					cr->arc(shape.p0[0], shape.p0[1], shape.r0, -angle + da, -angle + 2*PI - da);
					cr->arc(shape.p1[0], shape.p1[1], shape.r1, -angle - da, -angle + da);
					cr->close_path();
				}
				cr->stroke_preserve();
				cr->fill();
			}
		}
	}
}
