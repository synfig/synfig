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
#include "bone.h"


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
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_SkeletonDeformation,N_("SkeletonDeformation"));
SYNFIG_LAYER_SET_CATEGORY(Layer_SkeletonDeformation,N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_SkeletonDeformation,"0.1");
SYNFIG_LAYER_SET_CVS_ID(Layer_SkeletonDeformation,"$Id$");

/* === M E T H O D S ======================================================= */

Layer_SkeletonDeformation::Layer_SkeletonDeformation()
{
	param_bones=ValueBase(ValueBase::List);

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

	return ret;
}

void
Layer_SkeletonDeformation::prepare_mesh()
{
	static const Real precision = 1e-10;
	mesh.clear();

	// TODO: custom grid step
	// TODO: custom grid size
	// TODO: build grid with dynamic size

	struct GridPoint {
		Vector initial;
		Vector summary;
		Real summary_weight;
		inline GridPoint():
			summary_weight(0.0) { }
		inline explicit GridPoint(const Vector &initial):
			initial(initial), summary_weight(0.0) { }
		Vector get_average()const
			{ return summary_weight > precision ? summary/summary_weight : initial; }
	};

	const Real recommended_grid_step = 0.05;
	const Real grid_size = 10.0;
	const int grid_side_count = (int)roundf(grid_size / recommended_grid_step) + 1;
	const Real grid_step = grid_size / (Real)(grid_side_count - 1);

	// build grid
	std::vector<GridPoint> grid;
	grid.reserve(grid_side_count * grid_side_count);
	for(int i = 0; i < grid_side_count; ++i)
		for(int j = 0; j < grid_side_count; ++j)
			grid.push_back(GridPoint(Vector(
				i*grid_step - 0.5f*grid_size,
				j*grid_step - 0.5f*grid_size )));

	// apply deformation
	if (param_bones.can_get(ValueBase::List()))
	{
		const ValueBase::List &bones = param_bones.get_list();
		for(ValueBase::List::const_iterator i = bones.begin(); i != bones.end(); ++i)
		{
			if (i->can_get(SkeletonDeformationEntry()))
			{
				const SkeletonDeformationEntry &entry = i->get(SkeletonDeformationEntry());
				Matrix into_bone(
					entry.initial_p1[0] - entry.initial_p0[0], entry.initial_p1[1] - entry.initial_p0[1], 0.0,
					entry.initial_p0[1] - entry.initial_p1[1], entry.initial_p1[0] - entry.initial_p0[0], 0.0,
					entry.initial_p0[0], entry.initial_p0[1], 1.0
				);
				into_bone.invert();
				Matrix from_bone(
					entry.current_p1[0] - entry.current_p0[0], entry.current_p1[1] - entry.current_p0[1], 0.0,
					entry.current_p0[1] - entry.current_p1[1], entry.current_p1[0] - entry.current_p0[0], 0.0,
					entry.current_p0[0], entry.current_p0[1], 1.0
				);
				Matrix matrix = into_bone * from_bone;
				Bone::Shape shape(entry.initial_p0, entry.r0, entry.initial_p1, entry.r1);

				for(std::vector<GridPoint>::iterator j = grid.begin(); j != grid.end(); ++j)
				{
					Real influence = Bone::influence_percent(shape, j->initial);
					if (influence > 0.0)
					{
						j->summary += influence * matrix.get_transformed(j->initial);
						j->summary_weight += influence;
					}
				}
			}
		}
	}

	// build vertices
	mesh.vertices.reserve(grid.size());
	for(std::vector<GridPoint>::iterator i = grid.begin(); i != grid.end(); ++i)
		mesh.vertices.push_back(Mesh::Vertex(i->get_average(), i->initial));

	// build triangles
	mesh.triangles.reserve(2*(grid_side_count-1)*(grid_side_count-1));
	for(int i = 1; i < grid_side_count; ++i)
	{
		for(int j = 1; j < grid_side_count; ++j)
		{
			mesh.triangles.push_back(Mesh::Triangle(
				(j-1)*grid_side_count + (i-1),
				(j-1)*grid_side_count + i,
				j*grid_side_count + (i-1) ));
			mesh.triangles.push_back(Mesh::Triangle(
				j*grid_side_count + i,
				j*grid_side_count + (i-1),
				(j-1)*grid_side_count + i ));
		}
	}
}

bool
Layer_SkeletonDeformation::set_param(const String & param, const ValueBase &value)
{
    IMPORT_VALUE_PLUS(param_bones, prepare_mesh());
    return Layer_MeshTransform::set_param(param,value);
}

ValueBase
Layer_SkeletonDeformation::get_param(const String& param)const
{
	EXPORT_VALUE(param_bones);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_MeshTransform::get_param(param);
}

