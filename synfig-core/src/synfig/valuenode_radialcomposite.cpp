/* === S Y N F I G ========================================================= */
/*!	\file valuenode_radialcomposite.cpp
**	\brief Implementation of the "Radial Composite" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_radialcomposite.h"
#include "valuenode_const.h"
#include <stdexcept>
#include "general.h"
#include "color.h"
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

synfig::ValueNode_RadialComposite::ValueNode_RadialComposite(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	switch(get_type())
	{
		case ValueBase::TYPE_VECTOR:
		{
			Vector vect(value.get(Vector()));
			set_link("r",ValueNode_Const::create(vect.mag()));
			set_link("t",ValueNode_Const::create(Angle(Angle::tan(vect[1],vect[0]))));
		}
			break;
		case ValueBase::TYPE_COLOR:
			set_link("y",ValueNode_Const::create(value.get(Color()).get_y()));
			set_link("s",ValueNode_Const::create(value.get(Color()).get_s()));
			set_link("h",ValueNode_Const::create(value.get(Color()).get_hue()));
			set_link("a",ValueNode_Const::create(value.get(Color()).get_a()));
			break;
		default:
			assert(0);
			throw Exception::BadType(ValueBase::type_local_name(get_type()));
	}
}

ValueNode_RadialComposite::~ValueNode_RadialComposite()
{
	unlink_all();
}

ValueNode_RadialComposite*
ValueNode_RadialComposite::create(const ValueBase &value)
{
	return new ValueNode_RadialComposite(value);
}

LinkableValueNode*
ValueNode_RadialComposite::create_new()const
{
	return new ValueNode_RadialComposite(ValueBase(get_type()));
}

ValueBase
synfig::ValueNode_RadialComposite::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	switch(get_type())
	{
		case ValueBase::TYPE_VECTOR:
		{
			Real mag;
			Angle angle;
			assert(components[0] && components[1]);
			mag=(*components[0])(t).get(mag);
			angle=(*components[1])(t).get(angle);
			return Vector(Angle::cos(angle).get()*mag,Angle::sin(angle).get()*mag);
		}
		case ValueBase::TYPE_COLOR:
		{
			assert(components[0] && components[1] && components[2] && components[3]);
			return Color::YUV(
				(*components[0])(t).get(Real()),
				(*components[1])(t).get(Real()),
				(*components[2])(t).get(Angle()),
				(*components[3])(t).get(Real())
			);
		}
		default:
			synfig::error(string("ValueNode_RadialComposite::operator():")+_("Bad type for radialcomposite"));
			assert(components[0]);
			return (*components[0])(t);
	}
}

bool
ValueNode_RadialComposite::set_link_vfunc(int i,ValueNode::Handle x)
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
			if(i==0 && x->get_type()!=ValueBase::TYPE_REAL)
				return false;
			if(i==1 && x->get_type()!=ValueBase::TYPE_ANGLE)
				return false;
			components[i]=x;
			return true;
			break;

		case ValueBase::TYPE_COLOR:
			if((i==0 || i==1 || i==3) && x->get_type()!=ValueBase::TYPE_REAL)
				return false;
			if((i==2) && x->get_type()!=ValueBase::TYPE_ANGLE)
				return false;
			components[i]=x;
			return true;
			break;


		default:
			break;
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_RadialComposite::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	return components[i];
}

String
ValueNode_RadialComposite::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	if (get_file_version() < RELEASE_VERSION_0_61_08)
		return strprintf("c%d",i);

	return LinkableValueNode::link_name(i);
}

int
ValueNode_RadialComposite::get_link_index_from_name(const String &name)const
{
	// Here we don't use the LinkableValueNode::get_link_index_from_name
	// due to the particularities of the link index from name for old files.
	// So we keep this alive to maintain old file compatibilities.
	if(name.empty())
		throw Exception::BadLinkName(name);

	if(name[0]=='c' && name.size() == 2 && name[1]-'0' >= 0 && name[1]-'0' < link_count())
		return name[1]-'0';

	switch(get_type())
	{
	case ValueBase::TYPE_COLOR:
		if(name[0]=='y')
			return 0;
		if(name[0]=='s')
			return 1;
		if(name[0]=='h')
			return 2;
		if(name[0]=='a')
			return 3;
	case ValueBase::TYPE_VECTOR:
		if(name[0]=='r')
			return 0;
		if(name[0]=='t')
			return 1;
	default:
		break;
	}

	throw Exception::BadLinkName(name);
}

String
ValueNode_RadialComposite::get_name()const
{
	return "radial_composite";
}

String
ValueNode_RadialComposite::get_local_name()const
{
	return _("Radial Composite");
}

bool
ValueNode_RadialComposite::check_type(ValueBase::Type type)
{
	return
		type==ValueBase::TYPE_VECTOR ||
		type==ValueBase::TYPE_COLOR;
}

LinkableValueNode::Vocab
ValueNode_RadialComposite::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	switch(get_type())
	{
	case ValueBase::TYPE_COLOR:
		ret.push_back(ParamDesc(ValueBase(),"y_luma")
		.set_local_name(_("Luma"))
		);
		ret.push_back(ParamDesc(ValueBase(),"saturation")
		.set_local_name(_("Saturation"))
		);
		ret.push_back(ParamDesc(ValueBase(),"hue")
		.set_local_name(_("Hue"))
		);
		ret.push_back(ParamDesc(ValueBase(),"alpha")
		.set_local_name(_("Saturation"))
		);
		return ret;
		break;
	case ValueBase::TYPE_VECTOR:
		ret.push_back(ParamDesc(ValueBase(),"radius")
		.set_local_name(_("Radius"))
		.set_description(_("The length of the vector"))
		);
		ret.push_back(ParamDesc(ValueBase(),"theta")
		.set_local_name(_("Theta"))
		.set_description(_("The angle of the vector with the X axis"))
		);
		return ret;
		break;
	default:
		break;
	}

	return ret;
}
