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
#include "general.h"
#include "valuenode_radialcomposite.h"
#include "vector.h"
#include "color.h"
#include "segment.h"
#include "savecanvas.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_Composite::ValueNode_Composite(const ValueBase &value, Canvas::LooseHandle canvas):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	switch(get_type())
	{
		case ValueBase::TYPE_VECTOR:
			set_link("x",ValueNode_Const::create(value.get(Vector())[0]));
			set_link("y",ValueNode_Const::create(value.get(Vector())[1]));
			break;
		case ValueBase::TYPE_COLOR:
			set_link("r",ValueNode_Const::create(value.get(Color()).get_r()));
			set_link("g",ValueNode_Const::create(value.get(Color()).get_g()));
			set_link("b",ValueNode_Const::create(value.get(Color()).get_b()));
			set_link("a",ValueNode_Const::create(value.get(Color()).get_a()));
			break;
		case ValueBase::TYPE_SEGMENT:
			set_link("p1",ValueNode_Const::create(value.get(Segment()).p1));
			set_link("t1",ValueNode_Const::create(value.get(Segment()).t1));
			set_link("p2",ValueNode_Const::create(value.get(Segment()).p2));
			set_link("t2",ValueNode_Const::create(value.get(Segment()).t2));
			break;
		case ValueBase::TYPE_BLINEPOINT:
		{
			BLinePoint bline_point(value);
			set_link("point",ValueNode_Const::create(bline_point.get_vertex()));
			set_link("width",ValueNode_Const::create(bline_point.get_width()));
			set_link("origin",ValueNode_Const::create(bline_point.get_origin()));
			set_link("split",ValueNode_Const::create(bline_point.get_split_tangent_both()));
			set_link("split_radius",ValueNode_Const::create(bline_point.get_split_tangent_radius()));
			set_link("split_angle",ValueNode_Const::create(bline_point.get_split_tangent_angle()));
			set_link("t1",ValueNode_RadialComposite::create(bline_point.get_tangent1()));
			set_link("t2",ValueNode_RadialComposite::create(bline_point.get_tangent2()));
			break;
		}
		case ValueBase::TYPE_WIDTHPOINT:
		{
			WidthPoint wpoint(value);
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
			break;
		}
		case ValueBase::TYPE_DASHITEM:
		{
			DashItem ditem(value);
			set_link("offset",ValueNode_Const::create(ditem.get_offset()));
			set_link("length",ValueNode_Const::create(ditem.get_length()));
			set_link("side_before",ValueNode_Const::create(ditem.get_side_type_before()));
			set_link("side_after",ValueNode_Const::create(ditem.get_side_type_after()));
			break;
		}
		default:
			assert(0);
			throw Exception::BadType(ValueBase::type_local_name(get_type()));
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

	switch(get_type())
	{
		case ValueBase::TYPE_VECTOR:
		{
			Vector vect;
			assert(components[0] && components[1]);
			vect[0]=(*components[0])(t).get(Vector::value_type());
			vect[1]=(*components[1])(t).get(Vector::value_type());
			return vect;
		}
		case ValueBase::TYPE_COLOR:
		{
			Color color;
			assert(components[0] && components[1] && components[2] && components[3]);
			color.set_r((*components[0])(t).get(Vector::value_type()));
			color.set_g((*components[1])(t).get(Vector::value_type()));
			color.set_b((*components[2])(t).get(Vector::value_type()));
			color.set_a((*components[3])(t).get(Vector::value_type()));
			return color;
		}
		case ValueBase::TYPE_SEGMENT:
		{
			Segment seg;
			assert(components[0] && components[1] && components[2] && components[3]);
			seg.p1=(*components[0])(t).get(Point());
			seg.t1=(*components[1])(t).get(Vector());
			seg.p2=(*components[2])(t).get(Point());
			seg.t2=(*components[3])(t).get(Vector());
			return seg;
		}
		case ValueBase::TYPE_BLINEPOINT:
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
		case ValueBase::TYPE_WIDTHPOINT:
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
		case ValueBase::TYPE_DASHITEM:
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
		default:
			synfig::error(string("ValueNode_Composite::operator():")+_("Bad type for composite"));
			assert(components[0]);
			return (*components[0])(t);
	}
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

	switch(get_type())
	{
		case ValueBase::TYPE_VECTOR:
			if(x->get_type()==ValueBase(Real()).get_type() || PlaceholderValueNode::Handle::cast_dynamic(x))
			{
				components[i]=x;
				return true;
			}
			break;

		case ValueBase::TYPE_COLOR:
			if(x->get_type()==ValueBase(Real()).get_type() || PlaceholderValueNode::Handle::cast_dynamic(x))
			{
				components[i]=x;
				return true;
			}
			break;

		case ValueBase::TYPE_SEGMENT:
			if(x->get_type()==ValueBase(Point()).get_type() || PlaceholderValueNode::Handle::cast_dynamic(x))
			{
				components[i]=x;
				return true;
			}
			break;

		case ValueBase::TYPE_BLINEPOINT:
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
			break;
		case ValueBase::TYPE_DASHITEM:
		case ValueBase::TYPE_WIDTHPOINT:
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
						if(i==4 && (*x)(0).get(Real()) < (*components[5])(0).get(Real()))
						{
							components[i]=x;
							return true;
						}
						else
							return false;
					}
					if(i==5 && components[4])
					{
						if((i==5 && (*x)(0).get(Real()) > (*components[4])(0).get(Real())))
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
			break;
		default:
			break;
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

	switch(get_type())
	{
	case ValueBase::TYPE_COLOR:
		if(name[0]=='r')
			return 0;
		if(name[0]=='g')
			return 1;
		if(name[0]=='b')
			return 2;
		if(name[0]=='a')
			return 3;
	case ValueBase::TYPE_SEGMENT:
		if(name=="p1")
			return 0;
		if(name=="t1")
			return 1;
		if(name=="p2")
			return 2;
		if(name=="t2")
			return 3;
	case ValueBase::TYPE_VECTOR:
		if(name[0]=='x')
			return 0;
		if(name[0]=='y')
			return 1;
		if(name[0]=='z')		// \todo "z"?  really?
			return 2;
	case ValueBase::TYPE_BLINEPOINT:
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
	case ValueBase::TYPE_WIDTHPOINT:
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
	case ValueBase::TYPE_DASHITEM:
		if(name=="offset")
			return 0;
		if(name=="length")
			return 1;
		if(name=="side_before")
			return 2;
		if(name=="side_after")
			return 3;
	default:
		break;
	}

	throw Exception::BadLinkName(name);
}

String
ValueNode_Composite::get_name()const
{
	return "composite";
}

String
ValueNode_Composite::get_local_name()const
{
	return _("Composite");
}

bool
ValueNode_Composite::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_SEGMENT ||
		type==ValueBase::TYPE_VECTOR ||
		type==ValueBase::TYPE_COLOR ||
		type==ValueBase::TYPE_BLINEPOINT ||
		type==ValueBase::TYPE_WIDTHPOINT ||
		type==ValueBase::TYPE_DASHITEM;
}

LinkableValueNode::Vocab
ValueNode_Composite::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	switch(get_type())
	{
	case ValueBase::TYPE_COLOR:
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
	case ValueBase::TYPE_SEGMENT:
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
	case ValueBase::TYPE_VECTOR:
		ret.push_back(ParamDesc(ValueBase(),"x")
			.set_local_name(_("X-Axis"))
			.set_description(_("The X-Axis component of the vector"))
		);
		ret.push_back(ParamDesc(ValueBase(),"y")
			.set_local_name(_("Y-Axis"))
			.set_description(_("The Y-Axis component of the vector"))
		);
		return ret;
	case ValueBase::TYPE_BLINEPOINT:
		ret.push_back(ParamDesc(ValueBase(),"point")
			.set_local_name(_("Vertex"))
			.set_description(_("The vertex of the Spline Point"))
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
			.set_description(_("When checked, tangent's radius are independent"))
		);
		ret.push_back(ParamDesc(ValueBase(),"split_angle")
			.set_local_name(_("Angle Split"))
			.set_description(_("When checked, tangent's angles are independent"))
		);
		return ret;
	case ValueBase::TYPE_WIDTHPOINT:
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
	case ValueBase::TYPE_DASHITEM:
		ret.push_back(ParamDesc(ValueBase(),"offset")
			.set_local_name(_("Offset"))
			.set_description(_("The offset length of the Dash Item over the Spline"))
			.set_is_distance()
		);
		ret.push_back(ParamDesc(ValueBase(),"length")
			.set_local_name(_("Length"))
			.set_description(_("The length of the Dash Item"))
			.set_is_distance()
		);
		ret.push_back(ParamDesc(ValueBase(),"side_before")
			.set_local_name(_("Side Type Before"))
			.set_description(_("Defines the side type of the dash item"))
			.set_hint("enum")
			.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
			.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
			.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
			);
		ret.push_back(ParamDesc(ValueBase(),"side_after")
			.set_local_name(_("Side Type After"))
			.set_description(_("Defines the side type of the dash item"))
			.set_hint("enum")
			.add_enum_value(WidthPoint::TYPE_ROUNDED,"rounded", _("Rounded Stop"))
			.add_enum_value(WidthPoint::TYPE_SQUARED,"squared", _("Squared Stop"))
			.add_enum_value(WidthPoint::TYPE_PEAK,"peak", _("Peak Stop"))
			.add_enum_value(WidthPoint::TYPE_FLAT,"flat", _("Flat Stop"))
		);
		return ret;
	default:
		break;
	}

	return ret;
}
