/* === S Y N F I G ========================================================= */
/*!	\file valuenode_registry.cpp
**	\brief Valuenode automatic registry
**
**	\legal
**	Copyright (c) 2016 caryoscelus
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

#include "valuenode_registry.h"

#include "general.h"
#include "localization.h"

/* === U S I N G =========================================================== */

using namespace synfig;

/* === G L O B A L S ======================================================= */

ValueNodeRegistry::Book* ValueNodeRegistry::book_ = nullptr;

/* === M E T H O D S ======================================================= */

ValueNodeRegistry::Book&
ValueNodeRegistry::book()
{
	if (!book_)
		book_ = new Book();
	return *book_;
}

String
ValueNodeRegistry::localize_name(const String &local_name)
	{ return _(local_name.c_str()); }

void
ValueNodeRegistry::register_node_type(const String &name, const String &local_name, ReleaseVersion version, Factory factory, CheckType check_type)
{
	book()[name].local_name = local_name;
	book()[name].release_version = version;
	book()[name].factory = factory;
	book()[name].check_type = check_type;
}

bool
ValueNodeRegistry::cleanup()
{
	if (book_)
	{
		delete book_;
		book_ = nullptr;
	}
	return true;
}

LinkableValueNode::Handle
ValueNodeRegistry::create(const String &name, const ValueBase& x)
{
	// forbid creating a node if class is not registered
	if(!ValueNodeRegistry::book().count(name)) {
		error(_("Bad name: ValueNode type name '%s' isn't registered"), name.c_str());
		return nullptr;
	}

	if (!check_type(name, x.get_type()))
	{
		error(_("Bad type: ValueNode '%s' doesn't accept type '%s'"), ValueNodeRegistry::book()[name].local_name.c_str(), x.get_type().description.local_name.c_str());
		return nullptr;
	}

	return ValueNodeRegistry::book()[name].factory(x, nullptr);
}

bool
ValueNodeRegistry::check_type(const String &name, Type &x)
{
	// the BoneRoot and Duplicate ValueNodes are exceptions - we don't want the
	// user creating them for themselves, so check_type() fails for
	// them even when it is valid
	if((name == "bone_root" && x == type_bone_object) ||
	   (name == "duplicate" && x == type_real))
		return true;

	if(!ValueNodeRegistry::book().count(name) || !ValueNodeRegistry::book()[name].check_type)
		return false;
	return ValueNodeRegistry::book()[name].check_type(x);
}
