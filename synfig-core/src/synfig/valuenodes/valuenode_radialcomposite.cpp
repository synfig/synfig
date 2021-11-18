/* === S Y N F I G ========================================================= */
/*!	\file valuenode_radialcomposite.cpp
**	\brief Implementation of the "Radial Composite" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include "valuenode_radialcomposite.h"
#include "valuenode_const.h"
#include <stdexcept>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/color.h>
#include <synfig/savecanvas.h>
#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_RadialComposite, RELEASE_VERSION_0_61_06, "radial_composite", "Radial Composite")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::ValueNode_RadialComposite::ValueNode_RadialComposite(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	Type &type(get_type());
	if (type == type_vector)
	{
		Vector vect(value.get(Vector()));
		set_link("r",ValueNode_Const::create(vect.mag()));
		set_link("t",ValueNode_Const::create(Angle(Angle::tan(vect[1],vect[0]))));
	}
	else
	if (type == type_color)
	{
		set_link("y",ValueNode_Const::create(value.get(Color()).get_y()));
		set_link("s",ValueNode_Const::create(value.get(Color()).get_s()));
		set_link("h",ValueNode_Const::create(value.get(Color()).get_hue()));
		set_link("a",ValueNode_Const::create(value.get(Color()).get_a()));
	}
	else
	{
		assert(0);
		throw Exception::BadType(type.description.local_name);
	}
}

ValueNode_RadialComposite::~ValueNode_RadialComposite()
{
	unlink_all();
}

ValueNode_RadialComposite*
ValueNode_RadialComposite::create(const ValueBase& value, etl::loose_handle<Canvas>)
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

	Type &type(get_type());
	if (type == type_vector)
	{
		Real mag = 0.f;
		Angle angle;
		assert(components[0] && components[1]);
		mag=(*components[0])(t).get(mag);
		angle=(*components[1])(t).get(angle);
		return Vector(Angle::cos(angle).get()*mag,Angle::sin(angle).get()*mag);
	}
	else
	if (type == type_color)
	{
		assert(components[0] && components[1] && components[2] && components[3]);
		return Color::YUV(
			(*components[0])(t).get(Real()),
			(*components[1])(t).get(Real()),
			(*components[2])(t).get(Angle()),
			(*components[3])(t).get(Real())
		);
	}

	synfig::error(std::string("ValueNode_RadialComposite::operator():")+_("Bad type for radialcomposite"));
	assert(components[0]);
	return (*components[0])(t);
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

	Type &type(get_type());
	if (type == type_vector)
	{
		if(i==0 && x->get_type()!=type_real)
			return false;
		if(i==1 && x->get_type()!=type_angle)
			return false;
		components[i]=x;
		return true;
	}
	else
	if (type == type_color)
	{
		if((i==0 || i==1 || i==3) && x->get_type()!=type_real)
			return false;
		if((i==2) && x->get_type()!=type_angle)
			return false;
		components[i]=x;
		return true;
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

	Type &type(get_type());
	if (type == type_color)
	{
		if(name[0]=='y')
			return 0;
		if(name[0]=='s')
			return 1;
		if(name[0]=='h')
			return 2;
		if(name[0]=='a')
			return 3;
	}
	else
	if (type == type_vector)
	{
		if(name[0]=='r')
			return 0;
		if(name[0]=='t')
			return 1;
	}

	throw Exception::BadLinkName(name);
}



bool
ValueNode_RadialComposite::check_type(Type &type)
{
	return
		type==type_vector ||
		type==type_color;
}

LinkableValueNode::Vocab
ValueNode_RadialComposite::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	Type &type(get_type());
	if (type == type_color)
	{
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
		.set_local_name(_("Alpha"))
		);
		return ret;
	}
	else
	if (type == type_vector)
	{
		ret.push_back(ParamDesc(ValueBase(),"radius")
		.set_local_name(_("Radius"))
		.set_description(_("The length of the vector"))
		);
		ret.push_back(ParamDesc(ValueBase(),"theta")
		.set_local_name(_("Theta"))
		.set_description(_("The angle of the vector with the X axis"))
		);
		return ret;
	}

	return ret;
}
