/* === S Y N F I G ========================================================= */
/*!	\file layer_skeletondeformation.cpp
**	\brief SkeletonDeformation layer
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include "layer_skeletondeformation.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/common/task/tasklayer.h>

#include <vector>
#include <map>
#include <algorithm>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === C L A S S E S ======================================================= */

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_SkeletonDeformation);
SYNFIG_LAYER_SET_NAME(Layer_SkeletonDeformation,"skeleton_deformation");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_SkeletonDeformation,N_("Skeleton Deformation"));
SYNFIG_LAYER_SET_CATEGORY(Layer_SkeletonDeformation,N_("Distortions"));
SYNFIG_LAYER_SET_VERSION(Layer_SkeletonDeformation,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_SkeletonDeformation,"$Id$");

/* === M E T H O D S ======================================================= */

Layer_SkeletonDeformation::Layer_SkeletonDeformation():
	Layer_MeshTransform(1.0, Color::BLEND_STRAIGHT),
	param_point1(ValueBase(Point(-4,4))),
	param_point2(ValueBase(Point(4,-4))),
	param_x_subdivisions(32),
	param_y_subdivisions(32)
{
	param_bones.set_list_of(std::vector<BonePair>(1));

	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_SkeletonDeformation::~Layer_SkeletonDeformation()
{
}

String
Layer_SkeletonDeformation::get_local_name()const
{
	String s = Layer_MeshTransform::get_local_name();
	return s.empty() ? _("Skeleton Deformation") : '[' + s + ']';
}

Layer::Vocab
Layer_SkeletonDeformation::get_param_vocab()const
{
	Layer::Vocab ret(Layer_MeshTransform::get_param_vocab());

	ret.push_back(ParamDesc("bones")
		.set_local_name(_("Bones"))
		.set_description(_("List of bones"))
		.set_static(true)
	);

	ret.push_back(ParamDesc("point1")
		.set_local_name(_("Point 1"))
		.set_box("point2")
		.set_description(_("First corner of the bounds rectangle"))
	);

	ret.push_back(ParamDesc("point2")
		.set_local_name(_("Point 2"))
		.set_description(_("Second corner of the bounds rectangle"))
	);

	ret.push_back(ParamDesc("x_subdivisions")
		.set_local_name(_("Horizontal subdivisions"))
		.set_description(_("Count of horizontal subdivisions of the transformation grid"))
	);

	ret.push_back(ParamDesc("y_subdivisions")
		.set_local_name(_("Vertical subdivisions"))
		.set_description(_("Count of vertical subdivisions of the transformation grid"))
	);

	return ret;
}

void
Layer_SkeletonDeformation::prepare_mask()
{
	rendering::Contour::Handle mask(new rendering::Contour());
	mask->antialias = true;
  
	const std::vector<ValueBase> &list = param_bones.get_list();
	for(std::vector<ValueBase>::const_iterator i = list.begin(); i != list.end(); ++i) {
		if (!i->same_type_as(BonePair())) continue;
		const BonePair &bonePair = i->get(BonePair());
		const Bone &bone = bonePair.first;
		Matrix matrix = bone.get_animated_matrix();
		Vector origin = matrix.get_transformed(Vector(0.0, 0.0));
		Vector direction = matrix.get_transformed(Vector(1.0, 0.0), false).norm();
		Real length = bone.get_length() * bone.get_scalelx();

		if (length < 0) {
			length *= -1;
			direction *= -1;
		}

		const Vector &p0 = origin;
		const Vector p1 = origin + direction * length;

		Real r0 = fabs(bone.get_width());
		Real r1 = fabs(bone.get_tipwidth());
		
		if (approximate_greater_or_equal(r0, length + r1)) {
			mask->arc(p0, r0, 0.0, 2*PI, false);
			mask->close();
		} else
		if (approximate_greater_or_equal(r1, length + r0)) {
			mask->arc(p1, r1, 0.0, 2*PI, false);
			mask->close();
		} else {
			Real dr = r1 - r0;
			Real direction_angle = atan2(direction[1], direction[0]);
			Real da = PI - atan2(sqrt(length*length - dr*dr), dr);
			mask->arc(p0, r0, direction_angle + da, direction_angle + 2*PI - da, false);
			mask->arc(p1, r1, direction_angle - da, direction_angle + da, true);
			mask->close();
		}
	}
	this->mask = mask;
}

struct Layer_SkeletonDeformation::GridPoint {
	Vector initial_position;
	Vector summary_position;
	Real summary_depth;
	Real summary_weight;
	Real average_depth;
	bool used;

	inline GridPoint():
		summary_depth(0.0), summary_weight(0.0), average_depth(0.0), used(false) { }
	inline explicit GridPoint(const Vector &initial_position):
		initial_position(initial_position), summary_depth(0.0), summary_weight(0.0), average_depth(0.0), used(false) { }
	static bool compare_triagles(
		const std::pair<Real, rendering::Mesh::Triangle> &a,
		const std::pair<Real, rendering::Mesh::Triangle> &b )
	{
		return a.first < b.first ? false
			 : b.first < a.first ? true
			 : a.second.vertices[0] < b.second.vertices[0] ? true
			 : b.second.vertices[0] < a.second.vertices[0] ? false
			 : a.second.vertices[1] < b.second.vertices[1] ? true
			 : b.second.vertices[1] < a.second.vertices[1] ? false
			 : a.second.vertices[2] < b.second.vertices[2];
	}
};

Real Layer_SkeletonDeformation::distance_to_line(const Vector &p0, const Vector &p1, const Vector &x)
{
	const Real epsilon = 1e-10;

	Real distance_to_p0 = (x - p0).mag();
	Real distance_to_p1 = (x - p1).mag();
	Real distance_to_line = INFINITY;

	Vector line = p1 - p0;
	Real line_length = line.mag();
	if (line_length > epsilon)
	{
		Real dist = fabs((x - p0) * line.perp() / line_length);
		Real pos = (x - p0) * line / line_length;
		if (pos > 0.0 && pos < line_length)
			distance_to_line = dist;
	}

	return std::min(distance_to_line, std::min(distance_to_p0, distance_to_p1) );
}

void
Layer_SkeletonDeformation::prepare_mesh()
{
	static const Real precision = 1e-10;

	rendering::Mesh::Handle mesh(new rendering::Mesh());

	// TODO: build grid with dynamic size

	const Point grid_p0 = param_point1.get(Point());
	const Point grid_p1 = param_point2.get(Point());
	const int grid_side_count_x = std::max(1, param_x_subdivisions.get(int())) + 1;
	const int grid_side_count_y = std::max(1, param_y_subdivisions.get(int())) + 1;

	const Real grid_step_x = (grid_p1[0] - grid_p0[0]) / (Real)(grid_side_count_x - 1);
	const Real grid_step_y = (grid_p1[1] - grid_p0[1]) / (Real)(grid_side_count_y - 1);
	const Real grid_step_diagonal = sqrt(grid_step_x*grid_step_x + grid_step_y*grid_step_y);

	// build grid
	std::vector<GridPoint> grid;
	grid.reserve(grid_side_count_x * grid_side_count_y);
	for(int j = 0; j < grid_side_count_y; ++j)
		for(int i = 0; i < grid_side_count_x; ++i)
			grid.push_back(GridPoint(Vector(
				grid_p0[0] + i*grid_step_x,
				grid_p0[1] + j*grid_step_y )));

	// apply deformation
	if (param_bones.can_get(ValueBase::List()))
	{
		const ValueBase::List &bones = param_bones.get_list();
		for(ValueBase::List::const_iterator i = bones.begin(); i != bones.end(); ++i)
		{
			if (i->can_get(BonePair()))
			{
				const BonePair &bone_pair = i->get(BonePair());
				Bone::Shape shape0 = bone_pair.first.get_shape();
				Bone::Shape shape1 = bone_pair.second.get_shape();
				Bone::Shape expandedShape0 = shape0;
				expandedShape0.r0 += 2.0*grid_step_diagonal;
				expandedShape0.r1 += 2.0*grid_step_diagonal;
				Real depth = bone_pair.second.get_depth();

				Matrix into_bone(
					shape0.p1[0] - shape0.p0[0], shape0.p1[1] - shape0.p0[1], 0.0,
					shape0.p0[1] - shape0.p1[1], shape0.p1[0] - shape0.p0[0], 0.0,
					shape0.p0[0], shape0.p0[1], 1.0
				);
				into_bone.invert();
				Matrix from_bone(
					shape1.p1[0] - shape1.p0[0], shape1.p1[1] - shape1.p0[1], 0.0,
					shape1.p0[1] - shape1.p1[1], shape1.p1[0] - shape1.p0[0], 0.0,
					shape1.p0[0], shape1.p0[1], 1.0
				);
				Matrix matrix = from_bone * into_bone;

				for(std::vector<GridPoint>::iterator j = grid.begin(); j != grid.end(); ++j)
				{
					Real percent = Bone::distance_to_shape_center_percent(expandedShape0, j->initial_position);
					if (percent > precision) {
						Real distance = distance_to_line(shape0.p0, shape0.p1, j->initial_position);
						if (distance < precision) distance = precision;
						Real weight =
							percent/(distance*distance);
							// 1.0/distance;
							// 1.0/(distance*distance);
							// 1.0/(distance*distance*distance);
							// exp(-4.0*distance);
						j->summary_position += matrix.get_transformed(j->initial_position) * weight;
						j->summary_depth += depth * weight;
						j->summary_weight += weight;
						j->used = true;
					}
				}
			}
		}
	}

	// build vertices
	mesh->vertices.reserve(grid.size());
	for(std::vector<GridPoint>::iterator i = grid.begin(); i != grid.end(); ++i) {
		Vector average_position = i->summary_weight > precision ? i->summary_position/i->summary_weight : i->initial_position;
		i->average_depth = i->summary_weight > precision ? i->summary_depth/i->summary_weight : 0.0;
		mesh->vertices.push_back( rendering::Mesh::Vertex(
			average_position, i->initial_position ));
	}

	// build triangles
	std::vector< std::pair<Real, rendering::Mesh::Triangle> > triangles;
	triangles.reserve(2*(grid_side_count_x-1)*(grid_side_count_y-1));
	for(int j = 1; j < grid_side_count_y; ++j)
	{
		for(int i = 1; i < grid_side_count_x; ++i)
		{
			int v[] = {
				(j-1)*grid_side_count_x + (i-1),
				(j-1)*grid_side_count_x +  i,
				 j   *grid_side_count_x +  i,
				 j   *grid_side_count_x + (i-1),
			};
			if (grid[v[0]].used && grid[v[1]].used && grid[v[2]].used && grid[v[3]].used)
			{
				Real depth = 0.25*(grid[v[0]].average_depth
						         + grid[v[1]].average_depth
								 + grid[v[2]].average_depth
								 + grid[v[3]].average_depth);
				triangles.push_back(std::make_pair(depth, rendering::Mesh::Triangle(v[0], v[1], v[3])));
				triangles.push_back(std::make_pair(depth, rendering::Mesh::Triangle(v[1], v[2], v[3])));
			}
		}
	}

	// sort triangles
	std::sort(triangles.begin(), triangles.end(), GridPoint::compare_triagles);
	mesh->triangles.reserve(triangles.size());
	for(std::vector< std::pair<Real, rendering::Mesh::Triangle> >::iterator i = triangles.begin(); i != triangles.end(); ++i)
		mesh->triangles.push_back(i->second);

	prepare_mask();
	this->mesh = mesh;
}

bool
Layer_SkeletonDeformation::set_param(const String & param, const ValueBase &value)
{
	IMPORT_VALUE_PLUS(param_bones, prepare_mesh());
	IMPORT_VALUE_PLUS(param_point1, prepare_mesh());
	IMPORT_VALUE_PLUS(param_point2, prepare_mesh());
	IMPORT_VALUE_PLUS(param_x_subdivisions, prepare_mesh());
	IMPORT_VALUE_PLUS(param_y_subdivisions, prepare_mesh());
	return Layer_MeshTransform::set_param(param,value);
}

ValueBase
Layer_SkeletonDeformation::get_param(const String& param)const
{
	EXPORT_VALUE(param_bones);
	EXPORT_VALUE(param_point1);
	EXPORT_VALUE(param_point2);
	EXPORT_VALUE(param_x_subdivisions);
	EXPORT_VALUE(param_y_subdivisions);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_MeshTransform::get_param(param);
}

