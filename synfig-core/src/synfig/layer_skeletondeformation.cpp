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
#include "string.h"
#include "time.h"
#include "context.h"
#include "paramdesc.h"
#include "value.h"
#include "valuenode.h"
#include "canvas.h"


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
SYNFIG_LAYER_SET_VERSION(Layer_SkeletonDeformation,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_SkeletonDeformation,"$Id$");

/* === M E T H O D S ======================================================= */

Layer_SkeletonDeformation::Layer_SkeletonDeformation():
	param_point1(ValueBase(Point(-4,4))),
	param_point2(ValueBase(Point(4,-4))),
	param_x_subdivisions(32),
	param_y_subdivisions(32)
{
	max_texture_scale = 1.f;
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

struct Layer_SkeletonDeformation::GridPoint {
	Vector initial;
	Vector summary;
	Real summary_weight;
	inline GridPoint():
		summary_weight(0.0) { }
	inline explicit GridPoint(const Vector &initial):
		initial(initial), summary_weight(0.0) { }
	Vector get_average()const {
		static const Real precision = 1e-10;
		return summary_weight > precision ? summary/summary_weight : initial;
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
	const Real epsilon = 1e-10;

	mesh.clear();

	// TODO: build grid with dynamic size

	const Point grid_p0 = param_point1.get(Point());
	const Point grid_p1 = param_point2.get(Point());
	const int grid_side_count_x = std::max(1, param_x_subdivisions.get(int())) + 1;
	const int grid_side_count_y = std::max(1, param_y_subdivisions.get(int())) + 1;

	const Real grid_step_x = (grid_p1[0] - grid_p0[0]) / (Real)(grid_side_count_x - 1);
	const Real grid_step_y = (grid_p1[1] - grid_p0[1]) / (Real)(grid_side_count_y - 1);

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
				Matrix matrix = into_bone * from_bone;

				for(std::vector<GridPoint>::iterator j = grid.begin(); j != grid.end(); ++j)
				{
					Real distance = distance_to_line(shape0.p0, shape0.p1, j->initial);
					if (distance < epsilon) distance = epsilon;
					Real weight =
						// 1.0/distance;
						1.0/(distance*distance);
						// 1.0/(distance*distance*distance);
						//exp(-4.0*distance);
					j->summary += matrix.get_transformed(j->initial) * weight;
					j->summary_weight += weight;
				}
			}
		}
	}

	// build vertices
	mesh.vertices.reserve(grid.size());
	for(std::vector<GridPoint>::iterator i = grid.begin(); i != grid.end(); ++i)
		mesh.vertices.push_back(Mesh::Vertex(i->get_average(), i->initial));

	// build triangles
	mesh.triangles.reserve(2*(grid_side_count_x-1)*(grid_side_count_y-1));
	for(int j = 1; j < grid_side_count_y; ++j)
	{
		for(int i = 1; i < grid_side_count_x; ++i)
		{
			mesh.triangles.push_back(Mesh::Triangle(
				(j-1)*grid_side_count_x + (i-1),
				(j-1)*grid_side_count_x + i,
				j*grid_side_count_x + (i-1) ));
			mesh.triangles.push_back(Mesh::Triangle(
				j*grid_side_count_x + i,
				j*grid_side_count_x + (i-1),
				(j-1)*grid_side_count_x + i ));
		}
	}

	update_mesh();
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

