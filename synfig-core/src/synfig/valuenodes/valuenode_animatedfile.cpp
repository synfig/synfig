/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animatedfile.cpp
**	\brief Implementation of the "AnimatedFile" valuenode conversion.
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#include <synfig/localization.h>
#include <synfig/general.h>

#include "valuenode_animatedfile.h"
#include "valuenode_const.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === C L A S S E S ======================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_AnimatedFile::ValueNode_AnimatedFile(Type &t):
	ValueNode_AnimatedInterfaceConst(*(ValueNode*)this),
	filename(ValueNode_Const::create(String("")))
{
	ValueNode_AnimatedInterfaceConst::set_type(t);
	set_children_vocab(get_children_vocab());
}

bool
ValueNode_AnimatedFile::check_type(Type & /* type */)
	{ return true; }

LinkableValueNode*
ValueNode_AnimatedFile::create_new() const
	{ return new ValueNode_AnimatedFile(get_type()); }

ValueNode_AnimatedFile*
ValueNode_AnimatedFile::create(const ValueBase &x)
	{ return new ValueNode_AnimatedFile(x.get_type()); }

String
ValueNode_AnimatedFile::get_name()const
	{ return "animatedfile"; }

String
ValueNode_AnimatedFile::get_local_name()const
	{ return _("Animation from File"); }

void
ValueNode_AnimatedFile::load_file(const String &filename)
{
	if (current_filename == filename) return;
	erase_all();
	if (!filename.empty())
	{
		//todo: load
	}
	current_filename = filename;
}

void
ValueNode_AnimatedFile::on_changed()
{
	ValueNode::on_changed();
	ValueNode_AnimatedInterfaceConst::on_changed();
}

ValueBase
ValueNode_AnimatedFile::operator()(Time t) const
{
	const_cast<ValueNode_AnimatedFile*>(this)->load_file((*filename)(t).get(String()));
	return ValueNode_AnimatedInterfaceConst::operator()(t);
}

ValueNode::LooseHandle
ValueNode_AnimatedFile::get_link_vfunc(int i) const
{
	assert(i>=0 && i<link_count());

	if(i==0)
		return filename;

	return ValueNode::LooseHandle();
}

bool
ValueNode_AnimatedFile::set_link_vfunc(int i, ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(filename, type_string);
	}
	return false;
}

LinkableValueNode::Vocab
ValueNode_AnimatedFile::get_children_vocab_vfunc() const
{
	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc("filename")
		.set_local_name(_("Filename"))
		.set_description(_("File with waypoints"))
		.set_hint("filename")
	);

	return ret;
}


void
ValueNode_AnimatedFile::get_times_vfunc(Node::time_set &set) const
{
	//TODO:
	LinkableValueNode::get_times_vfunc(set);
}

