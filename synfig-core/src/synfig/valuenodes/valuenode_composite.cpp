/* === S Y N F I G ========================================================= */
/*!	\file valuenode_composite.cpp
**	\brief Implementation of the "Composite" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
**  Copyright (c) 2011 Carlos López
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

#include "valuenode_composite.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include "valuenode_radialcomposite.h"
#include <synfig/vector.h>
#include <synfig/color.h>
#include <synfig/segment.h>
#include <synfig/savecanvas.h>
#include <synfig/transformation.h>
#include <synfig/weightedvalue.h>
#include <synfig/pair.h>
#include <synfig/blinepoint.h>
#include <synfig/widthpoint.h>
#include <synfig/dashitem.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Composite, RELEASE_VERSION_0_61_06, "composite", "Composite")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Composite::ValueNode_Composite(const ValueBase &value, Canvas::LooseHandle canvas):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Type &type(get_type());
	if (type == type_vector)
	{
		set_link("x",ValueNode_Const::create(value.get(Vector())[0]));
		set_link("y",ValueNode_Const::create(value.get(Vector())[1]));
	}
	else
	if (type == type_color)
	{
		set_link("r",ValueNode_Const::create(value.get(Color()).get_r()));
		set_link("g",ValueNode_Const::create(value.get(Color()).get_g()));
		set_link("b",ValueNode_Const::create(value.get(Color()).get_b()));
		set_link("a",ValueNode_Const::create(value.get(Color()).get_a()));
	}
	else
	if (type == type_segment)
	{
		set_link("p1",ValueNode_Const::create(value.get(Segment()).p1));
		set_link("t1",ValueNode_Const::create(value.get(Segment()).t1));
		set_link("p2",ValueNode_Const::create(value.get(Segment()).p2));
		set_link("t2",ValueNode_Const::create(value.get(Segment()).t2));
	}
	else
	if (type == type_bline_point)
	{
		BLinePoint bline_point(value.get(BLinePoint()));
		set_link("point",ValueNode_Const::create(bline_point.get_vertex()));
		set_link("width",ValueNode_Const::create(bline_point.get_width()));
		set_link("origin",ValueNode_Const::create(bline_point.get_origin()));
		set_link("split",ValueNode_Const::create(bline_point.get_split_tangent_both()));
		set_link("split_radius",ValueNode_Const::create(bline_point.get_split_tangent_radius()));
		set_link("split_angle",ValueNode_Const::create(bline_point.get_split_tangent_angle()));
		set_link("t1",ValueNode_RadialComposite::create(bline_point.get_tangent1()));
		set_link("t2",ValueNode_RadialComposite::create(bline_point.get_tangent2()));
	}
	else
	if (type == type_width_point)
	{
		WidthPoint wpoint(value.get(WidthPoint()));
		set_link("position",ValueNode_Const::create(wpoint.get_position()));
		set_link("width",ValueNode_Const::create(wpoint.get_width()));
		set_link("side_before",ValueNode_Const::create(wpoint.get_side_type_before()));
		set_link("side_after",ValueNode_Const::create(wpoint.get_side_type_after()));
		ValueNode_Const::Handle value_node;
		value_node=ValueNode_Const::Handle::cast_dynamic(ValueNode_Const::create(wpoint.get_lower_bound()));
		if(value_node)
		{
			value_node->set_static(true);
			set_link("lower_bound",value_node);
		}
		value_node=ValueNode_Const::Handle::cast_dynamic(ValueNode_Const::create(wpoint.get_upper_bound()));
		if(value_node)
		{
			value_node->set_static(true);
			set_link("upper_bound",value_node);
		}
	}
	else
	if (type == type_dash_item)
	{
		DashItem ditem(value.get(DashItem()));
		set_link("offset",ValueNode_Const::create(ditem.get_offset()));
		set_link("length",ValueNode_Const::create(ditem.get_length()));
		set_link("side_before",ValueNode_Const::create(ditem.get_side_type_before()));
		set_link("side_after",ValueNode_Const::create(ditem.get_side_type_after()));
	}
	else
	if (type == type_transformation)
	{
		Transformation transformation(value.get(Transformation()));
		set_link("offset",ValueNode_Const::create(transformation.offset));
		set_link("angle",ValueNode_Const::create(transformation.angle));
		set_link("skew_angle",ValueNode_Const::create(transformation.skew_angle));
		set_link("scale",ValueNode_Const::create(transformation.scale));
	}
	else
	if (dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type) != NULL)
	{
		types_namespace::TypeWeightedValueBase *t =
			dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type);
		set_link("weight",ValueNode_Const::create(t->extract_weight(value)));
		set_link("value",ValueNode_Const::create(t->extract_value(value)));
	}
	else
	if (dynamic_cast<types_namespace::TypePairBase*>(&type) != NULL)
	{
		types_namespace::TypePairBase *t =
			dynamic_cast<types_namespace::TypePairBase*>(&type);
		set_link("first",ValueNode_Const::create(t->extract_first(value)));
		set_link("second",ValueNode_Const::create(t->extract_second(value)));
	}
	else
	{
		assert(0);
		throw Exception::BadType(get_type().description.local_name);
	}

	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d set parent canvas for composite %lx to %lx\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(canvas.get()));
	set_parent_canvas(canvas);
}

ValueNode_Composite::~ValueNode_Composite()
{
	unlink_all();
}

ValueNode_Composite*
ValueNode_Composite::create(const ValueBase &value, Canvas::LooseHandle canvas)
{
	return new ValueNode_Composite(value, canvas);
}

LinkableValueNode*
ValueNode_Composite::create_new()const
{
	return new ValueNode_Composite(ValueBase(get_type()));
}

ValueBase
synfig::ValueNode_Composite::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Type &type(get_type());
	if (type == type_vector)
	{
		Vector vect;
		assert(components[0] && components[1]);
		vect[0]=(*components[0])(t).get(Vector::value_type());
		vect[1]=(*components[1])(t).get(Vector::value_type());
		return vect;
	}
	else
	if (type == type_color)
	{
		Color color;
		assert(components[0] && components[1] && components[2] && components[3]);
		color.set_r((*components[0])(t).get(Vector::value_type()));
		color.set_g((*components[1])(t).get(Vector::value_type()));
		color.set_b((*components[2])(t).get(Vector::value_type()));
		color.set_a((*components[3])(t).get(Vector::value_type()));
		return color;
	}
	else
	if (type == type_segment)
	{
		Segment seg;
		assert(components[0] && components[1] && components[2] && components[3]);
		seg.p1=(*components[0])(t).get(Point());
		seg.t1=(*components[1])(t).get(Vector());
		seg.p2=(*components[2])(t).get(Point());
		seg.t2=(*components[3])(t).get(Vector());
		return seg;
	}
	else
	if (type == type_bline_point)
	{
		BLinePoint ret;
		assert(components[0] && components[1] && components[2] && components[3] && components[4] && components[5] && components[6] && components[7]);
		ret.set_vertex((*components[0])(t).get(Point()));
		ret.set_width((*components[1])(t).get(Real()));
		ret.set_origin((*components[2])(t).get(Real()));
		ret.set_split_tangent_both((*components[3])(t).get(bool()));
		ret.set_split_tangent_radius((*components[6])(t).get(bool()));
		ret.set_split_tangent_angle((*components[7])(t).get(bool()));
		ret.set_tangent1((*components[4])(t).get(Vector()));
		ret.set_tangent2((*components[5])(t).get(Vector()));
		return ret;
	}
	else
	if (type == type_width_point)
	{
		WidthPoint ret;
		assert(components[0] && components[1] && components[2] && components[3] && components[4] && components[5]);
		ret.set_position((*components[0])(t).get(Real()));
		ret.set_width((*components[1])(t).get(Real()));
		ret.set_side_type_before((*components[2])(t).get(int()));
		ret.set_side_type_after((*components[3])(t).get(int()));
		ret.set_lower_bound((*components[4])(t).get(Real()));
		ret.set_upper_bound((*components[5])(t).get(Real()));
		return ret;
	}
	else
	if (type == type_dash_item)
	{
		DashItem ret;
		assert(components[0] && components[1] && components[2] && components[3]);
		Real offset((*components[0])(t).get(Real()));
		if(offset < 0.0) offset=0.0;
		Real length((*components[1])(t).get(Real()));
		if(length < 0.0) length=0.0;
		ret.set_offset(offset);
		ret.set_length(length);
		ret.set_side_type_before((*components[2])(t).get(int()));
		ret.set_side_type_after((*components[3])(t).get(int()));
		return ret;
	}
	else
	if (type == type_transformation)
	{
		Transformation ret;
		assert(components[0] && components[1] && components[2] && components[3]);
		ret.offset    = (*components[0])(t).get(Vector());
		ret.angle     = (*components[1])(t).get(Angle());
		ret.skew_angle = (*components[2])(t).get(Angle());
		ret.scale     = (*components[3])(t).get(Vector());
		return ret;
	}
	else
	if (dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type) != NULL)
	{
		types_namespace::TypeWeightedValueBase *tp =
			dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type);
		assert(components[0] && components[1]);
		return tp->create_weighted_value((*components[0])(t).get(Real()), (*components[1])(t));
	}
	else
	if (dynamic_cast<types_namespace::TypePairBase*>(&type) != NULL)
	{
		types_namespace::TypePairBase *tp =
			dynamic_cast<types_namespace::TypePairBase*>(&type);
		assert(components[0] && components[1]);
		return tp->create_value((*components[0])(t), (*components[1])(t));
	}

	synfig::error(string("ValueNode_Composite::operator():")+_("Bad type for composite"));
	assert(components[0]);
	return (*components[0])(t);
}

bool
ValueNode_Composite::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<link_count());

	if(PlaceholderValueNode::Handle::cast_dynamic(x))
	{
		components[i]=x;
		return true;
	}

	Type &type(get_type());
	if (type == type_vector)
	{
		if(x->get_type()==ValueBase(Real()).get_type() || PlaceholderValueNode::Handle::cast_dynamic(x))
		{
			components[i]=x;
			return true;
		}
	}
	else
	if (type == type_color)
	{
		if(x->get_type()==ValueBase(Real()).get_type() || PlaceholderValueNode::Handle::cast_dynamic(x))
		{
			components[i]=x;
			return true;
		}
	}
	else
	if (type == type_segment)
	{
		if(x->get_type()==ValueBase(Point()).get_type() || PlaceholderValueNode::Handle::cast_dynamic(x))
		{
			components[i]=x;
			return true;
		}
	}
	else
	if (type == type_bline_point)
	{
		if((i==0 || i==4 || i==5) && x->get_type()==ValueBase(Point()).get_type())
		{
			components[i]=x;
			return true;
		}
		if((i==1 || i==2) && x->get_type()==ValueBase(Real()).get_type())
		{
			components[i]=x;
			return true;
		}
		if((i==3 || i==6 || i==7) && x->get_type()==ValueBase(bool()).get_type())
		{
			components[i]=x;
			return true;
		}
	}
	else
	if (type == type_dash_item
	 || type == type_width_point)
	{
		if((i==0 || i==1) && x->get_type()==ValueBase(Real()).get_type())
		{
			components[i]=x;
			return true;
		}
		if((i==2 || i==3) && x->get_type()==ValueBase(int()).get_type())
		{
			components[i]=x;
			return true;
		}
		if((i==4 || i==5) && x->get_type()==ValueBase(Real()).get_type())
		{
			if(ValueNode_Const::Handle::cast_dynamic(x))
			{
				if(i==4 && components[5])
				{
					if ((*x)(0).get(Real()) < (*components[5])(0).get(Real()))
					{
						components[i]=x;
						return true;
					}
					else
						return false;
				}
				if (i==5 && components[4])
				{
					if ((*x)(0).get(Real()) > (*components[4])(0).get(Real()))
					{
						components[i]=x;
						return true;
					}
					else
						return false;
				}
				components[i]=x;
				return true;
			}
			return false;
		}
	}
	else
	if (type == type_transformation)
	{
		if( PlaceholderValueNode::Handle::cast_dynamic(x)
		 || (i == 0 && x->get_type()==ValueBase(Vector()).get_type())
		 || (i == 1 && x->get_type()==ValueBase(Angle()).get_type())
		 || (i == 2 && x->get_type()==ValueBase(Angle()).get_type())
		 || (i == 3 && x->get_type()==ValueBase(Vector()).get_type())
		) {
			components[i]=x;
			return true;
		}
	}
	else
	if (dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type) != NULL)
	{
		types_namespace::TypeWeightedValueBase *tp =
			dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type);
		if( PlaceholderValueNode::Handle::cast_dynamic(x)
		 || (i == 0 && x->get_type()==ValueBase(Real()).get_type())
		 || (i == 1 && x->get_type()==tp->get_contained_type())
		) {
			components[i]=x;
			return true;
		}
	}
	else
	if (dynamic_cast<types_namespace::TypePairBase*>(&type) != NULL)
	{
		types_namespace::TypePairBase *tp =
			dynamic_cast<types_namespace::TypePairBase*>(&type);
		if( PlaceholderValueNode::Handle::cast_dynamic(x)
		 || (i == 0 && x->get_type()==tp->get_first_type())
		 || (i == 1 && x->get_type()==tp->get_second_type())
		) {
			components[i]=x;
			return true;
		}
	}

	return false;
}

ValueNode::LooseHandle
ValueNode_Composite::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	return components[i];
}


String
ValueNode_Composite::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if (get_file_version() < RELEASE_VERSION_0_61_08)
		return strprintf("c%d",i+1);

	return LinkableValueNode::link_name(i);
}

int
ValueNode_Composite::get_link_index_from_name(const String &name)const
{
	// Here we don't use the LinkableValueNode::get_link_index_from_name
	// due to the particularities of the link index from name for old files.
	// So we keep this alive to maintain old file compatibilities.
	if(name.empty())
		throw Exception::BadLinkName(name);

	if(name[0]=='c' && name.size() == 2 && name[1]-'1' >= 0 && name[1]-'1' < link_count())
		return name[1]-'1';

	Type &type(get_type());
	if (type == type_color)
	{
		if(name[0]=='r')
			return 0;
		if(name[0]=='g')
			return 1;
		if(name[0]=='b')
			return 2;
		if(name[0]=='a')
			return 3;
	}
	else
	if (type == type_segment)
	{
		if(name=="p1")
			return 0;
		if(name=="t1")
			return 1;
		if(name=="p2")
			return 2;
		if(name=="t2")
			return 3;
	}
	else
	if (type == type_vector)
	{
		if(name[0]=='x')
			return 0;
		if(name[0]=='y')
			return 1;
		if(name[0]=='z')		// \todo "z"?  really?
			return 2;
	}
	else
	if (type == type_bline_point)
	{
		if(name[0]=='p' || name=="v1" || name=="p1")
			return 0;
		if(name=="w" || name=="width")
			return 1;
		if(name=="o" || name=="origin")
			return 2;
		if(name=="split")
			return 3;
		if(name=="t1")
			return 4;
		if(name=="t2")
			return 5;
		if(name=="split_radius")
			return 6;
		if(name=="split_angle")
			return 7;
	}
	else
	if (type == type_width_point)
	{
		if(name=="position")
			return 0;
		if(name=="width")
			return 1;
		if(name=="side_before")
			return 2;
		if(name=="side_after")
			return 3;
		if(name=="lower_bound")
			return 4;
		if(name=="upper_bound")
			return 5;
	}
	else
	if (type == type_dash_item)
	{
		if(name=="offset")
			return 0;
		if(name=="length")
			return 1;
		if(name=="side_before")
			return 2;
		if(name=="side_after")
			return 3;
	}
	else
	if (type == type_transformation)
	{
		if(name=="offset")
			return 0;
		if(name=="angle")
			return 1;
		if(name=="skew_angle")
			return 2;
		if(name=="scale")
			return 3;
	}
	else
	if (dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type) != NULL)
	{
		if(name=="weight")
			return 0;
		if(name=="value")
			return 1;
	}
	else
	if (dynamic_cast<types_namespace::TypePairBase*>(&type) != NULL)
	{
		if(name=="first")
			return 0;
		if(name=="second")
			return 1;
	}

	throw Exception::BadLinkName(name);
}



bool
ValueNode_Composite::check_type(Type &type)
{
	return type==type_segment
		|| type==type_vector
		|| type==type_color
		|| type==type_bline_point
		|| type==type_width_point
		|| type==type_dash_item
		|| type==type_transformation
		|| dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type) != NULL
		|| dynamic_cast<types_namespace::TypePairBase*>(&type) != NULL;
}

LinkableValueNode::Vocab
ValueNode_Composite::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	Type &type(get_type());
	if (type == type_color)
	{
		ret.push_back(ParamDesc(ValueBase(),"red")
			.set_local_name(_("Red"))
			.set_description(_("The red component of the color"))
		);
		ret.push_back(ParamDesc(ValueBase(),"green")
			.set_local_name(_("Green"))
			.set_description(_("The green component of the color"))
		);
		ret.push_back(ParamDesc(ValueBase(),"blue")
			.set_local_name(_("Blue"))
			.set_description(_("The blue component of the color"))
		);
		ret.push_back(ParamDesc(ValueBase(),"alpha")
			.set_local_name(_("Alpha"))
			.set_description(_("The alpha of the color"))
		);
		return ret;
	}
	else
	if (type == type_segment)
	{
		ret.push_back(ParamDesc(ValueBase(),"p1")
			.set_local_name(_("Vertex 1"))
			.set_description(_("The first vertex of the segment"))
		);
		ret.push_back(ParamDesc(ValueBase(),"t1")
			.set_local_name(_("Tangent 1"))
			.set_description(_("The first tangent of the segment"))
		);
		ret.push_back(ParamDesc(ValueBase(),"p2")
			.set_local_name(_("Vertex 2"))
			.set_description(_("The second vertex of the segment"))
		);
		ret.push_back(ParamDesc(ValueBase(),"t2")
			.set_local_name(_("Tangent 2"))
			.set_description(_("The second tangent of the segment"))
		);
		return ret;
	}
	else
	if (type == type_vector)
	{
		ret.push_back(ParamDesc(ValueBase(),"x")
			.set_local_name(_("X-Axis"))
			.set_description(_("The X-Axis component of the vector"))
		);
		ret.push_back(ParamDesc(ValueBase(),"y")
			.set_local_name(_("Y-Axis"))
			.set_description(_("The Y-Axis component of the vector"))
		);
		return ret;
	}
	else
	if (type == type_bline_point)
	{
		ret.push_back(ParamDesc(ValueBase(),"point")
			.set_local_name(_("Vertex"))
			.set_description(_("The vertex of the Spline Point"))
			.set_is_distance()
		);
		ret.push_back(ParamDesc(ValueBase(),"width")
			.set_local_name(_("Width"))
			.set_description(_("The width of the Spline Point"))
		);
		ret.push_back(ParamDesc(ValueBase(),"origin")
			.set_local_name(_("Origin"))
			.set_description(_("Defines the Off and On position relative to neighbours"))
		);
		ret.push_back(ParamDesc(ValueBase(),"split")
			.set_local_name(_("Split"))
			.set_description(_("When checked, tangents are independent"))
			.hidden()
		);
		ret.push_back(ParamDesc(ValueBase(),"t1")
			.set_local_name(_("Tangent 1"))
			.set_description(_("The first tangent of the Spline Point"))
		);
		ret.push_back(ParamDesc(ValueBase(),"t2")
			.set_local_name(_("Tangent 2"))
			.set_description(_("The second tangent of the Spline Point"))
		);
		ret.push_back(ParamDesc(ValueBase(),"split_radius")
			.set_local_name(_("Radius Split"))
			.set_description(_("When checked, tangent's radii are independent"))
		);
		ret.push_back(ParamDesc(ValueBase(),"split_angle")
			.set_local_name(_("Angle Split"))
			.set_description(_("When checked, tangent's angles are independent"))
		);
		return ret;
	}
	else
	if (type == type_width_point)
	{
		ret.push_back(ParamDesc(ValueBase(),"position")
			.set_local_name(_("Position"))
			.set_description(_("The [0,1] position of the Width Point over the Spline"))
		);
		ret.push_back(ParamDesc(ValueBase(),"width")
			.set_local_name(_("Width"))
			.set_description(_("The width of the Width Point"))
		);
		ret.push_back(ParamDesc(ValueBase(),"side_before")
			.set_local_name(_("Side Type Before"))
			.set_description(_("Defines the interpolation type of the width point"))
			.set_hint("enum")
			.add_enum_value(WidthPoint::TYPE_INTERPOLATE,"interpolate",_("Interpolate"))
			.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
			.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
			.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
			.add_enum_value(WidthPoint::TYPE_INNER_ROUNDED,"inner_rounded", _("Inner Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_INNER_PEAK,"inner_peak", _("Off-Peak Stop"))
			);
		ret.push_back(ParamDesc(ValueBase(),"side_after")
			.set_local_name(_("Side Type After"))
			.set_description(_("Defines the interpolation type of the width point"))
			.set_hint("enum")
			.add_enum_value(WidthPoint::TYPE_INTERPOLATE,"interpolate",_("Interpolate"))
			.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
			.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
			.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
			.add_enum_value(WidthPoint::TYPE_INNER_ROUNDED,"inner_rounded", _("Inner Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_INNER_PEAK,"inner_peak", _("Off-Peak Stop"))
		);
		ret.push_back(ParamDesc(ValueBase(),"lower_bound")
			.set_local_name(_("Lower Boundary"))
			.set_description(_("Defines the position at start of the Spline"))
		);
		ret.push_back(ParamDesc(ValueBase(),"upper_bound")
			.set_local_name(_("Upper Boundary"))
			.set_description(_("Defines the position at end of the Spline"))
		);
		return ret;
	}
	else
	if (type == type_dash_item)
	{
		ret.push_back(ParamDesc(ValueBase(),"offset")
			.set_local_name(_("Offset"))
			.set_description(_("The offset length of the Dash Item over the Spline"))
		);
		ret.push_back(ParamDesc(ValueBase(),"length")
			.set_local_name(_("Length"))
			.set_description(_("The length of the Dash Item"))
		);
		ret.push_back(ParamDesc(ValueBase(),"side_before")
			.set_local_name(_("Side Type Before"))
			.set_description(_("Defines the side type of the dash item"))
			.set_hint("enum")
			.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
			.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
			.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
			.add_enum_value(WidthPoint::TYPE_INNER_ROUNDED,"inner_rounded", _("Inner Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_INNER_PEAK,"inner_peak", _("Off-Peak Stop"))
			);
		ret.push_back(ParamDesc(ValueBase(),"side_after")
			.set_local_name(_("Side Type After"))
			.set_description(_("Defines the side type of the dash item"))
			.set_hint("enum")
			.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
			.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
			.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
			.add_enum_value(WidthPoint::TYPE_INNER_ROUNDED,"inner_rounded", _("Inner Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_INNER_PEAK,"inner_peak", _("Off-Peak Stop"))
		);
		return ret;
	}
	else
	if (type == type_transformation)
	{
		ret.push_back(ParamDesc(ValueBase(),"offset")
			.set_local_name(_("Offset"))
			.set_description(_("The Offset component of the transformation"))
		);
		ret.push_back(ParamDesc(ValueBase(),"angle")
			.set_local_name(_("Angle"))
			.set_description(_("The Angle component of the transformation"))
		);
		ret.push_back(ParamDesc(ValueBase(),"skew_angle")
			.set_local_name(_("Skew Angle"))
			.set_description(_("The Skew Angle component of the transformation"))
		);
		ret.push_back(ParamDesc(ValueBase(),"scale")
			.set_local_name(_("Scale"))
			.set_description(_("The Scale component of the transformation"))
		);
		return ret;
	}
	else
	if (dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type) != NULL)
	{
		ret.push_back(ParamDesc(ValueBase(),"weight")
			.set_local_name(_("Weight"))
			.set_description(_("The Weight of the value"))
		);
		ret.push_back(ParamDesc(ValueBase(),"value")
			.set_local_name(_("Value"))
			.set_description(_("The Value"))
		);
		return ret;
	}
	else
	if (dynamic_cast<types_namespace::TypePairBase*>(&type) != NULL)
	{
		ret.push_back(ParamDesc(ValueBase(),"first")
			.set_local_name(_("First"))
			.set_description(_("The First Value"))
		);
		ret.push_back(ParamDesc(ValueBase(),"second")
			.set_local_name(_("Second"))
			.set_description(_("The Second Value"))
		);
		return ret;
	}

	return ret;
}
