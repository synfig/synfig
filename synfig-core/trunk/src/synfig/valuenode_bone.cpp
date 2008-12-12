/* === S Y N F I G ========================================================= */
/*!	\file valuenode_bone.cpp
**	\brief Implementation of the "Bone" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include "valuenode_bone.h"
#include "valuenode_const.h"
#include "valuenode_animated.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define HIDE_BONE_FIELDS

#define GET_NODE_PARENT_NODE(node,t) (*node->get_link("parent"))(t).get(ValueNode_Bone::Handle())
#define GET_NODE_PARENT(node,t) GET_NODE_PARENT_NODE(node,t)->get_guid()
#define GET_NODE_NAME(node,t) (*node->get_link("name"))(t).get(String())
#define GET_NODE_BONE(node,t) (*node)(t).get(Bone())

#define GET_GUID_CSTR(guid) guid.get_string().substr(0,GUID_PREFIX_LEN).c_str()
#define GET_NODE_GUID_CSTR(node) GET_GUID_CSTR(node->get_guid())
#define GET_NODE_NAME_CSTR(node,t) GET_NODE_NAME(node,t).c_str()
#define GET_NODE_BONE_CSTR(node,t) GET_NODE_BONE(node,t).c_str()
#define GET_NODE_DESC_CSTR(node,t) (node ? strprintf("%s (%s)", GET_NODE_GUID_CSTR(node), GET_NODE_NAME_CSTR(node,t)) : strprintf("%s <root>", GET_GUID_CSTR(GUID(0)))).c_str()
#define GET_NODE_PARENT_CSTR(node,t) GET_GUID_CSTR(GET_NODE_PARENT(node,t))

/* === G L O B A L S ======================================================= */

static ValueNode_Bone::BoneMap bone_map;
static int bone_counter;
static map<ValueNode_Bone::Handle, Matrix> setup_matrix_map;
static map<ValueNode_Bone::Handle, Matrix> animated_matrix_map;
static Time last_time = Time::begin();

/* === P R O C E D U R E S ================================================= */

struct compare_bones
{
	bool operator() (const ValueNode_Bone::LooseHandle b1, const ValueNode_Bone::LooseHandle b2) const
	{
		String n1(GET_NODE_NAME(b1,0));
		String n2(GET_NODE_NAME(b2,0));

		if (n1 < n2) return true;
		if (n1 > n2) return false;

		return b1->get_guid() < b2->get_guid();
	}
};

void
ValueNode_Bone::show_bone_map(const char *file, int line, String text, Time t)
{
	if (!getenv("SYNFIG_SHOW_BONE_MAP")) return;

	set<ValueNode_Bone::LooseHandle, compare_bones> bone_set;
	for (ValueNode_Bone::BoneMap::iterator iter = bone_map.begin(); iter != bone_map.end(); iter++)
		bone_set.insert(iter->second);

	printf("\n  %s:%d %s we now have %d bones (%d unreachable):\n", file, line, text.c_str(), int(bone_map.size()), int(bone_map.size() - bone_set.size()));

	for (set<ValueNode_Bone::LooseHandle>::iterator iter = bone_set.begin(); iter != bone_set.end(); iter++)
	{
		ValueNode_Bone::LooseHandle bone(*iter);
		GUID guid(bone->get_guid());
		ValueNode_Bone::LooseHandle parent(GET_NODE_PARENT_NODE(bone,t));
		String id;
		if (bone->is_exported()) id = String(" ") + bone->get_id();
//		printf("%s : %s (%d)\n",           		GET_GUID_CSTR(guid), GET_NODE_BONE_CSTR(bone,t), bone->rcount());
		printf("    %-20s : parent %-20s (%d refs, %d rrefs)%s\n",
			   GET_NODE_DESC_CSTR(bone,t),
			   GET_NODE_DESC_CSTR(parent,t),
			   bone->count(), bone->rcount(),
			   id.c_str());
	}
	printf("\n");
}

/* === M E T H O D S ======================================================= */

ValueNode_Bone::ValueNode_Bone(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	printf("\n%s:%d ------------------------------------------------------------------------\n", __FILE__, __LINE__);
	printf("%s:%d --- ValueNode_Bone() for %s at %lx---\n", __FILE__, __LINE__, GET_GUID_CSTR(get_guid()), ulong(this));
	printf("%s:%d ------------------------------------------------------------------------\n\n", __FILE__, __LINE__);

	switch(value.get_type())
	{
	case ValueBase::TYPE_BONE:
	{
		Bone bone(value.get(Bone()));
		set_link("name",ValueNode_Const::create(bone.get_name().empty() ? strprintf(_("Bone %d"), ++bone_counter) : bone.get_name()));
#ifndef HIDE_BONE_FIELDS
		set_link("origin",ValueNode_Const::create(bone.get_origin()));
		set_link("origin0",ValueNode_Const::create(bone.get_origin0()));
		set_link("angle",ValueNode_Const::create(bone.get_angle()));
		set_link("angle0",ValueNode_Const::create(bone.get_angle0()));
		set_link("scale",ValueNode_Const::create(bone.get_scale()));
		set_link("length",ValueNode_Const::create(bone.get_length()));
		set_link("strength",ValueNode_Const::create(bone.get_strength()));
#endif
		set_link("parent",ValueNode_Const::create(find(bone.get_parent())));

		bone_map[get_guid()] = this;

		show_bone_map(__FILE__, __LINE__, strprintf("in constructor of %s at %lx", GET_GUID_CSTR(get_guid()), ulong(this)));

		break;
	}
	default:
		throw Exception::BadType(ValueBase::type_local_name(value.get_type()));
	}

	DCAST_HACK_ENABLE();
}

void ValueNode_Bone::on_changed()
{
	if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
		printf("%s:%d ValueNode_Bone::on_changed()\n", __FILE__, __LINE__);

	LinkableValueNode::on_changed();
}

LinkableValueNode*
ValueNode_Bone::create_new()const
{
	return new ValueNode_Bone(get_type());
}

ValueNode_Bone*
ValueNode_Bone::create(const ValueBase &x)
{
	return new ValueNode_Bone(x);
}

ValueNode_Bone::~ValueNode_Bone()
{
	printf("\n%s:%d ------------------------------------------------------------------------\n", __FILE__, __LINE__);
	printf("%s:%d --- ~ValueNode_Bone() for %s at %lx---\n", __FILE__, __LINE__, GET_GUID_CSTR(get_guid()), ulong(this));
	printf("%s:%d ------------------------------------------------------------------------\n\n", __FILE__, __LINE__);

	bone_map.erase(get_guid());

	show_bone_map(__FILE__, __LINE__, "in destructor");

	unlink_all();
}

void
ValueNode_Bone::set_guid(const GUID& new_guid)
{
	GUID old_guid(get_guid());
	// show_bone_map(__FILE__, __LINE__, strprintf("before changing guid from %s to %s", GET_GUID_CSTR(old_guid), GET_GUID_CSTR(new_guid)));
	LinkableValueNode::set_guid(new_guid);
	bone_map[new_guid] = bone_map[old_guid];
	bone_map.erase(old_guid);
	// show_bone_map(__FILE__, __LINE__, strprintf("after changing guid from %s to %s", GET_GUID_CSTR(old_guid), GET_GUID_CSTR(new_guid)));
}

ValueBase
ValueNode_Bone::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	show_bone_map(__FILE__, __LINE__, strprintf("in op() at %s", t.get_string().c_str()), t);

	Bone ret;
	ret.set_name		((*name_	)(t).get(String()));
#ifndef HIDE_BONE_FIELDS
	ret.set_origin		((*origin_	)(t).get(Point()));
	ret.set_origin0		((*origin0_	)(t).get(Point()));
	ret.set_angle		((*angle_	)(t).get(Angle()));
	ret.set_angle0		((*angle0_	)(t).get(Angle()));
	ret.set_scale		((*scale_	)(t).get(Real()));
	ret.set_length		((*length_	)(t).get(Real()));
	ret.set_strength	((*strength_)(t).get(Real()));
#endif

	// check if we are an ancestor of the proposed parent
	ValueNode_Bone::ConstHandle parent((*parent_)(t).get(ValueNode_Bone::Handle()));
	if (ValueNode_Bone::ConstHandle result = is_ancestor_of(parent,t))
	{
		if (result == ValueNode_Bone::ConstHandle(this))
			synfig::error("A bone cannot be parent of itself or any of its descendants");
		else
			synfig::error("A loop was detected in the ancestry at bone %s", GET_NODE_DESC_CSTR(result,t));
	}
	else // proposed parent is root or not a descendant of current bone
	{
		ValueNode_Bone::ConstHandle parent((*parent_)(t).get(ValueNode_Bone::Handle()));
		if (parent)
			ret.set_parent(parent->get_guid());
		else
			ret.set_parent(0);
	}

	return ret;
}

String
ValueNode_Bone::get_name()const
{
	return "bone";
}

String
ValueNode_Bone::get_local_name()const
{
	return _("Bone");
}

bool
ValueNode_Bone::check_type(ValueBase::Type type)
{
	return type==ValueBase::TYPE_BONE;
}

bool
ValueNode_Bone::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(name_,		ValueBase::TYPE_STRING);
#ifdef HIDE_BONE_FIELDS
	case 1:
	{
		// CHECK_TYPE_AND_SET_VALUE(parent_,	ValueBase::TYPE_VALUENODE_BONE);
		VALUENODE_CHECK_TYPE(ValueBase::TYPE_VALUENODE_BONE);
		VALUENODE_SET_VALUE(parent_);
	}
#else
	case 1: CHECK_TYPE_AND_SET_VALUE(origin_,	ValueBase::TYPE_VECTOR);
	case 2: CHECK_TYPE_AND_SET_VALUE(origin0_,	ValueBase::TYPE_VECTOR);
	case 3: CHECK_TYPE_AND_SET_VALUE(angle_,	ValueBase::TYPE_ANGLE);
	case 4: CHECK_TYPE_AND_SET_VALUE(angle0_,	ValueBase::TYPE_ANGLE);
	case 5: CHECK_TYPE_AND_SET_VALUE(scale_,	ValueBase::TYPE_REAL);
	case 6: CHECK_TYPE_AND_SET_VALUE(length_,	ValueBase::TYPE_REAL);
	case 7: CHECK_TYPE_AND_SET_VALUE(strength_,	ValueBase::TYPE_REAL);
	case 8: CHECK_TYPE_AND_SET_VALUE(parent_,	ValueBase::TYPE_VALUENODE_BONE);
#endif
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Bone::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return name_;
#ifdef HIDE_BONE_FIELDS
	case 1: return parent_;
#else
	case 1: return origin_;
	case 2: return origin0_;
	case 3: return angle_;
	case 4: return angle0_;
	case 5: return scale_;
	case 6: return length_;
	case 7: return strength_;
	case 8: return parent_;
#endif
	}

	return 0;
}

int
ValueNode_Bone::link_count()const
{
#ifdef HIDE_BONE_FIELDS
	return 2;
#else
	return 9;
#endif
}

String
ValueNode_Bone::link_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return "name";
#ifdef HIDE_BONE_FIELDS
	case 1: return "parent";
#else
	case 1: return "origin";
	case 2: return "origin0";
	case 3: return "angle";
	case 4: return "angle0";
	case 5: return "scale";
	case 6: return "length";
	case 7: return "strength";
	case 8: return "parent";
#endif
	}

	return String();
}

String
ValueNode_Bone::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return _("Name");
#ifdef HIDE_BONE_FIELDS
	case 1: return _("Parent");
#else
	case 1: return _("Origin");
	case 2: return _("Origin0");
	case 3: return _("Angle");
	case 4: return _("Angle0");
	case 5: return _("Scale");
	case 6: return _("Length");
	case 7: return _("Strength");
	case 8: return _("Parent");
#endif
	}

	return String();
}

int
ValueNode_Bone::get_link_index_from_name(const String &name)const
{
	if (name == "name") return 0;
#ifdef HIDE_BONE_FIELDS
	if (name == "parent") return 1;
#else
	if (name == "origin") return 1;
	if (name == "origin0") return 2;
	if (name == "angle") return 3;
	if (name == "angle0") return 4;
	if (name == "scale") return 5;
	if (name == "length") return 6;
	if (name == "strength") return 7;
	if (name == "parent") return 8;
#endif

	throw Exception::BadLinkName(name);
}

ValueNode_Bone::BoneMap::const_iterator
ValueNode_Bone::map_begin()
{
	return bone_map.begin();
}

ValueNode_Bone::BoneMap::const_iterator
ValueNode_Bone::map_end()
{
	return bone_map.end();
}

ValueNode_Bone::Handle
ValueNode_Bone::find(GUID guid)
{
	if (!guid) return 0;
	// printf("looking up bone %s\n", GET_GUID_CSTR(guid));
	if (bone_map.count(guid))
	{
		// check that the bone we find is the one we looked up - this currently isn't the case when we load bones from a .sifz file
		if (guid != bone_map[guid]->get_guid())
		{
			printf("wtf? todo: fix loading so it doesn't damage the guid\n");
			assert(0);
		}
		return bone_map[guid];
	}
	else
	{
		printf("didn't find bone with guid %s\n", GET_GUID_CSTR(guid));
		return 0;
	}
}

ValueNode_Bone::BoneSet
ValueNode_Bone::get_bones(ValueNode::Handle value)
{
	BoneSet ret;
	if (!value)
	{
		printf("%s:%d failed?\n", __FILE__, __LINE__);
		assert(0);
		return ret;
	}

	if (ValueNode_Const::Handle value_node_const = ValueNode_Const::Handle::cast_dynamic(value))
	{
		ValueBase value(value_node_const->get_value());
		if (value.get_type() == ValueBase::TYPE_VALUENODE_BONE)
			if (ValueNode_Bone::Handle bone = value.get(ValueNode_Bone::Handle()))
			{
				// do we want to check for bone references in other bone fields or just 'parent'?
				ret = get_bones(bone);
				// ret = get_bones(bone->get_link("parent"));
				ret.insert(bone);
			}
		return ret;
	}

	if (ValueNode_Animated::Handle value_node_animated = ValueNode_Animated::Handle::cast_dynamic(value))
	{
		// ValueNode_Animated::Handle ret = ValueNode_Animated::create(ValueBase::TYPE_BONE);
		ValueNode_Animated::WaypointList list(value_node_animated->waypoint_list());
		for (ValueNode_Animated::WaypointList::iterator iter = list.begin(); iter != list.end(); iter++)
		{
			printf("%s:%d getting bones from waypoint\n", __FILE__, __LINE__);
			BoneSet ret2(get_bones(iter->get_value_node()));
			ret.insert(ret2.begin(), ret2.end());
			printf("added %d bones from waypoint to get %d\n", int(ret2.size()), int(ret.size()));
		}
		printf("returning %d bones\n", int(ret.size()));
		return ret;
	}

	if (LinkableValueNode::Handle linkable_value_node = LinkableValueNode::Handle::cast_dynamic(value))
	{
		for (int i = 0; i < linkable_value_node->link_count(); i++)
		{
			BoneSet ret2(get_bones(linkable_value_node->get_link(i)));
			ret.insert(ret2.begin(), ret2.end());
		}
		return ret;
	}

	error("%s:%d BUG: failed to clone ValueNode '%s'", __FILE__, __LINE__, value->get_description().c_str());
	assert(0);
	return ret;
}

// checks whether the current object is an ancestor of the supplied bone
// returns a handle to NULL if it isn't
// if there's a loop in the ancestry it returns a handle to the valuenode where the loop is detected
// otherwise it returns the current object
ValueNode_Bone::ConstHandle
ValueNode_Bone::is_ancestor_of(ValueNode_Bone::ConstHandle bone, Time t)const
{
	ValueNode_Bone::ConstHandle ancestor(this);
	set<ValueNode_Bone::ConstHandle> seen;

	if (getenv("SYNFIG_DEBUG_ANCESTOR_CHECK"))
		printf("%s:%d checking whether %s is ancestor of %s\n", __FILE__, __LINE__, GET_NODE_DESC_CSTR(ancestor,t), GET_NODE_DESC_CSTR(bone,t));

	while (bone)
	{
		if (bone == ancestor)
		{
			if (getenv("SYNFIG_DEBUG_ANCESTOR_CHECK"))
				printf("%s:%d bone reached us - so we are its ancestor - return true\n", __FILE__, __LINE__);
			return bone;
		}

		if (seen.count(bone))
		{
			if (getenv("SYNFIG_DEBUG_ANCESTOR_CHECK"))
				printf("%s:%d stuck in a loop - return true\n", __FILE__, __LINE__);
			return bone;
		}

		seen.insert(bone);
		bone=GET_NODE_PARENT_NODE(bone,t);

		if (getenv("SYNFIG_DEBUG_ANCESTOR_CHECK"))
			printf("%s:%d step on to parent %s\n", __FILE__, __LINE__, GET_NODE_DESC_CSTR(bone,t));
	}

	if (getenv("SYNFIG_DEBUG_ANCESTOR_CHECK"))
		printf("%s:%d reached root - return false\n", __FILE__, __LINE__);
	return 0;
}

// return a set holding the bones that would be affected if the given ValueNode were edited
// value_node is either a ValueNode_Const or a ValueNode_Animated, of type VALUENODE_BONE
ValueNode_Bone::BoneSet
ValueNode_Bone::get_bones_affected_by(ValueNode::Handle value_node)
{
	printf("%s:%d\n", __FILE__, __LINE__);
	set<ValueNode_Bone::Handle> ret;
	set<const Node*> seen, current_nodes, new_nodes;
	int generation = 0;

	// initialise current_nodes with the node we're editing
	current_nodes.insert(value_node.get());
	do
	{
		generation++;
		printf("generation %d has %zd nodes\n", generation, current_nodes.size());

		int count = 0;
		// loop through current_nodes
		for (set<const Node*>::iterator iter = current_nodes.begin(); iter != current_nodes.end(); iter++, count++)
		{
			// loop through the parents of each node in current_nodes
			set<Node*> node_parents((*iter)->parent_set);
			printf("%s:%d node %d %lx (%s) has %zd parents\n", __FILE__, __LINE__, count, ulong(*iter), (*iter)->get_string().c_str(), node_parents.size());
			int count2 = 0;
			for (set<Node*>::iterator iter2 = node_parents.begin(); iter2 != node_parents.end(); iter2++, count2++)
			{
				Node* node(*iter2);
				printf("%s:%d parent %d: %lx (%s)\n", __FILE__, __LINE__, count2, ulong(node), node->get_string().c_str());
				// for each parent we've not already seen
				if (!seen.count(node))
				{
					// note that we've seen it now
					seen.insert(node);
					// add it to the list of new nodes to loop though in the next iteration
					new_nodes.insert(node);
					// and if it's a ValueNode_Bone, add it to the set to be returned
					if (dynamic_cast<ValueNode_Bone*>(node))
					{
						ret.insert(dynamic_cast<ValueNode_Bone*>(node));
						printf("%s:%d it's an affected bone\n", __FILE__, __LINE__);
					}
				}
			}
		}
		current_nodes = new_nodes;
		new_nodes.clear();
	} while (current_nodes.size());

	printf("%s:%d got %zd affected bones\n", __FILE__, __LINE__, ret.size());
	return ret;
}

#ifdef _DEBUG
void
ValueNode_Bone::ref()const
{
	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%s:%d %s   ref valuenode_bone %*s -> %2d\n", __FILE__, __LINE__, GET_GUID_CSTR(get_guid()), (count()*2), "", count()+1);

	LinkableValueNode::ref();
}

bool
ValueNode_Bone::unref()const
{
	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%s:%d %s unref valuenode_bone %*s%2d <-\n", __FILE__, __LINE__, GET_GUID_CSTR(get_guid()), ((count()-1)*2), "", count()-1);

	return LinkableValueNode::unref();
}

void
ValueNode_Bone::rref()const
{
	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%s:%d %s               rref valuenode_bone %d -> ", __FILE__, __LINE__, GET_GUID_CSTR(get_guid()), rcount());

	LinkableValueNode::rref();

	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%d\n", rcount());
}

void
ValueNode_Bone::runref()const
{
	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%s:%d %s             runref valuenode_bone %d -> ", __FILE__, __LINE__, GET_GUID_CSTR(get_guid()), rcount());

	LinkableValueNode::runref();

	if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
		printf("%d\n", rcount());
}
#endif
