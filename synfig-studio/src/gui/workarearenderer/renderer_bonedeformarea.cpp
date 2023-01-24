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
#include <gui/ducktransform_matrix.h>
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
			TransformStack transform_stack;
			Layer::Handle ref_layer = layer;
			while (ref_layer) {
				TransformStack canvas_transform_stack;
				Layer_PasteCanvas::Handle parent_paste_canvas = Layer_PasteCanvas::Handle::cast_dynamic(ref_layer->get_parent_paste_canvas_layer());
				if (parent_paste_canvas) {
					canvas_transform_stack.push(
						new Transform_Matrix(
							ref_layer->get_guid(),
							parent_paste_canvas->get_summary_transformation().get_matrix()
						)
					);
				}
				Canvas::LooseHandle canvas = ref_layer->get_canvas();
				if (canvas) {
					auto cend = canvas->cend();
					for (auto iter = canvas->cbegin(); iter != cend && *iter != ref_layer; ++iter) {
						if ((*iter)->active()) {
							canvas_transform_stack.push((*iter)->get_transform());
						}
					}
				}

				transform_stack.insert(transform_stack.begin(), canvas_transform_stack.begin(), canvas_transform_stack.end());

				ref_layer = parent_paste_canvas;
			}
			std::vector<std::pair<Bone,Bone>> bone_pair_list;
			auto it = layer->dynamic_param_list().find("bones");
			if (it != layer->dynamic_param_list().end()) {
				bone_pair_list = (*it->second)(canvas->get_time()).get_list_of(std::pair<Bone,Bone>());
			} else {
				auto bones = layer->get_param("bones");
				bone_pair_list = bones.get_list_of(std::pair<Bone,Bone>());
			}

			if (transform_stack.empty()) {
				// Easier to implement
				for (const auto& bone_pair : bone_pair_list) {
					Bone::Shape shape = bone_pair.first.get_shape();

					shape.p0 = comp_to_screen_coords(transform_stack.perform(shape.p0));
					shape.p1 = comp_to_screen_coords(transform_stack.perform(shape.p1));
					shape.r0 /= pw;
					shape.r1 /= pw;

					const Real dr = shape.r1 - shape.r0;
					const Real length_squared = (shape.p1 - shape.p0).mag_squared();
					const Real dist2 = length_squared - dr*dr;
					if (dist2 < 0) {
						// radius is big enough to contain the entire bone

						if (shape.r0 > shape.r1) {
							cr->arc(shape.p0[0], shape.p0[1], shape.r0, 0, 2*PI);
						} else {
							cr->arc(shape.p1[0], shape.p1[1], shape.r1, 0, 2*PI);
						}
					} else {
						// 'Regular' bone shape

						const Real dr = shape.r1 - shape.r0;
						const Real length_squared = (shape.p1 - shape.p0).mag_squared();
						const Real dist2 = length_squared - dr*dr;

						const Real angle = Angle::rad((shape.p1 - shape.p0).angle()).get();
						const Real da = PI - atan2(sqrt(dist2), dr);
						const Real angle1 = angle + da;
						const Real angle2 = angle - da;

						cr->move_to(shape.p0[0]+shape.r0*cos(angle1), shape.p0[1]+shape.r0*sin(angle1));
						cr->arc(shape.p0[0], shape.p0[1], shape.r0, angle1, angle2 + (angle2 < angle1? 2*PI : 0));
						cr->arc(shape.p1[0], shape.p1[1], shape.r1, angle2, angle1 + (angle1 < angle2? 2*PI : 0));
						cr->close_path();
					}
				}
			} else {
				// It has Transformations:
				// let's draw the bone shape by splitting it into small segments

				const int num_segments = 72;
				const auto angle_step = Angle::deg(360./num_segments);

				for (const auto& bone_pair : bone_pair_list) {
					Bone::Shape shape = bone_pair.first.get_shape();

					const Real dr = shape.r1 - shape.r0;
					const Real length_squared = (shape.p1 - shape.p0).mag_squared();
					const Real dist2 = length_squared - dr*dr;
					if (dist2 < 0) {
						// radius is big enough to contain the entire bone

						Point center = shape.r0 > shape.r1 ? shape.p0 : shape.p1;
						Real radius  = shape.r0 > shape.r1 ? shape.r0 : shape.r1;
						Point p = comp_to_screen_coords(transform_stack.perform(center+Vector(radius, 0)));
						cr->move_to(p[0], p[1]);
						for (int i = 1; i < num_segments; ++i) {
							p = comp_to_screen_coords(transform_stack.perform(center+Vector(radius, angle_step*i)));
							cr->line_to(p[0], p[1]);
						}
						cr->close_path();
					} else {
						// 'Regular' bone shape

						const Real angle = Angle::rad((shape.p1 - shape.p0).angle()).get();
						const Real da = PI - atan2(sqrt(dist2), dr);
						const Real angle1 = angle + da;
						const Real angle2 = angle - da;

						// Semicircle of Bone Origin
						Angle::rad a(angle1);
						Angle::rad b(angle2 + (angle2 < angle1? 2*PI : 0));
						Point p = shape.p0 + Vector(shape.r0, a);
						p = comp_to_screen_coords(transform_stack.perform(p));
						cr->move_to(p[0], p[1]);
						for (; a < b; a += angle_step) {
							p = shape.p0 + Vector(shape.r0, a);
							p = comp_to_screen_coords(transform_stack.perform(p));
							cr->line_to(p[0], p[1]);
						}
						p = shape.p0 + Vector(shape.r0, b);
						p = comp_to_screen_coords(transform_stack.perform(p));
						cr->line_to(p[0], p[1]);

						// Line to Bone Tip
						p = shape.p0 + Vector(shape.r0, b);
						a = Angle::rad(angle2);
						b = Angle::rad(angle1 + (angle1 < angle2? 2*PI : 0));
						Point q = shape.p1 + Vector(shape.r1, a);
						Vector dir = (q - p) / num_segments;
						for (int i = 0; i < num_segments; ++i) {
							p += dir;
							Point sp = comp_to_screen_coords(transform_stack.perform(p));
							cr->line_to(sp[0], sp[1]);
						}

						// Semicircle of Bone Tip
						for (; a < b; a += angle_step) {
							p = shape.p1 + Vector(shape.r1, a);
							p = comp_to_screen_coords(transform_stack.perform(p));
							cr->line_to(p[0], p[1]);
						}
						p = shape.p1 + Vector(shape.r1, b);
						p = comp_to_screen_coords(transform_stack.perform(p));
						cr->line_to(p[0], p[1]);

						// Line to Bone Origin
						p = shape.p1 + Vector(shape.r1, b);
						a = Angle::rad(angle1);
						q = shape.p0 + Vector(shape.r0, a);
						dir = (q - p) / num_segments;
						for (int i = 0; i < num_segments; ++i) {
							p += dir;
							Point sp = comp_to_screen_coords(transform_stack.perform(p));
							cr->line_to(sp[0], sp[1]);
						}

						cr->close_path();
					}
				}

			}

			cr->stroke_preserve();
			cr->fill();
		}
	}
}
