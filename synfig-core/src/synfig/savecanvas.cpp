/* === S Y N F I G ========================================================= */
/*!	\file savecanvas.cpp
**	\brief Writeme
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011, 2012 Carlos López
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

#include "savecanvas.h"

#include "general.h"
#include <synfig/localization.h>
#include "valuenode.h"
#include "valuenode_registry.h"
#include "valuenodes/valuenode_animated.h"
#include "valuenodes/valuenode_const.h"
#include "valuenodes/valuenode_staticlist.h"
#include "valuenodes/valuenode_dynamiclist.h"
#include "valuenodes/valuenode_subtract.h"
#include "valuenodes/valuenode_bline.h"
#include "valuenodes/valuenode_bone.h"
#include "dashitem.h"
#include "time.h"
#include "keyframe.h"
#include "layer.h"
#include "string.h"
#include "paramdesc.h"
#include "weightedvalue.h"
#include "pair.h"
#include "segment.h"
#include "transformation.h"

#include "zstreambuf.h"
#include "importer.h"

#include <libxml++/libxml++.h>
#include "gradient.h"


#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

#define COLOR_VALUE_TYPE_FORMAT		"%f"
#define	VECTOR_VALUE_TYPE_FORMAT	"%0.10f"
#define	TIME_TYPE_FORMAT			"%0.3f"
#define	VIEW_BOX_FORMAT				"%f %f %f %f"

/* === G L O B A L S ======================================================= */

ReleaseVersion save_canvas_version = ReleaseVersion(RELEASE_VERSION_END-1);
int valuenode_too_new_count;
save_canvas_external_file_callback_t save_canvas_external_file_callback = nullptr;
void *save_canvas_external_file_user_data = nullptr;

/* === P R O C E D U R E S ================================================= */

xmlpp::Element* encode_canvas(xmlpp::Element* root,Canvas::ConstHandle canvas);
xmlpp::Element* encode_value_node(xmlpp::Element* root,ValueNode::ConstHandle value_node,Canvas::ConstHandle canvas);
xmlpp::Element* encode_value_node_bone(xmlpp::Element* root,ValueNode::ConstHandle value_node,Canvas::ConstHandle canvas);
xmlpp::Element* encode_value_node_bone_id(xmlpp::Element* root,ValueNode::ConstHandle value_node,Canvas::ConstHandle canvas);

xmlpp::Element* encode_keyframe(xmlpp::Element* root,const Keyframe &kf, float fps)
{
	root->set_name("keyframe");
 	root->set_attribute("time",kf.get_time().get_string(fps));
	if(!kf.get_description().empty())
		root->set_child_text(kf.get_description());
	root->set_attribute("active", kf.active()?"true":"false");
	return root;
}

xmlpp::Element* encode_interpolation(xmlpp::Element* root,Interpolation value,String attribute)
{
	if (value!=INTERPOLATION_UNDEFINED)
	{
		switch(value)
		{
		case INTERPOLATION_HALT:
			root->set_attribute(attribute,"halt");
			break;
		case INTERPOLATION_LINEAR:
			root->set_attribute(attribute,"linear");
			break;
		case INTERPOLATION_MANUAL:
			root->set_attribute(attribute,"manual");
			break;
		case INTERPOLATION_CONSTANT:
			root->set_attribute(attribute,"constant");
			break;
		case INTERPOLATION_TCB:
			root->set_attribute(attribute,"auto");
			break;
		case INTERPOLATION_CLAMPED:
			root->set_attribute(attribute,"clamped");
			break;
		default:
			error("Unknown waypoint type for \""+attribute+"\" attribute");
		}
	}
	return root;
}

xmlpp::Element* encode_static(xmlpp::Element* root,bool s)
{
	if(s)
		root->set_attribute("static", s?"true":"false");
	return root;
}


xmlpp::Element* encode_real(xmlpp::Element* root,Real v)
{
	root->set_name("real");
	root->set_attribute("value",strprintf(VECTOR_VALUE_TYPE_FORMAT,v));
	return root;
}

xmlpp::Element* encode_time(xmlpp::Element* root,Time t)
{
	root->set_name("time");
	root->set_attribute("value",t.get_string());
	return root;
}

xmlpp::Element* encode_integer(xmlpp::Element* root,int i)
{
	root->set_name("integer");
	root->set_attribute("value",strprintf("%i",i));
	return root;
}

xmlpp::Element* encode_bool(xmlpp::Element* root, bool b)
{
	root->set_name("bool");
	root->set_attribute("value",b?"true":"false");
	return root;
}

xmlpp::Element* encode_string(xmlpp::Element* root,const String &str)
{
	root->set_name("string");
	root->set_child_text(str);
	return root;
}

xmlpp::Element* encode_vector(xmlpp::Element* root,Vector vect)
{
	root->set_name("vector");
	root->add_child("x")->set_child_text(strprintf(VECTOR_VALUE_TYPE_FORMAT,(float)vect[0]));
	root->add_child("y")->set_child_text(strprintf(VECTOR_VALUE_TYPE_FORMAT,(float)vect[1]));
	return root;
}

xmlpp::Element* encode_color(xmlpp::Element* root,Color color)
{
	root->set_name("color");
	root->add_child("r")->set_child_text(strprintf(COLOR_VALUE_TYPE_FORMAT,(float)color.get_r()));
	root->add_child("g")->set_child_text(strprintf(COLOR_VALUE_TYPE_FORMAT,(float)color.get_g()));
	root->add_child("b")->set_child_text(strprintf(COLOR_VALUE_TYPE_FORMAT,(float)color.get_b()));
	root->add_child("a")->set_child_text(strprintf(COLOR_VALUE_TYPE_FORMAT,(float)color.get_a()));
	return root;
}

xmlpp::Element* encode_angle(xmlpp::Element* root,Angle theta)
{
	root->set_name("angle");
	root->set_attribute("value",strprintf("%f",(float)Angle::deg(theta).get()));
	return root;
}

xmlpp::Element* encode_segment(xmlpp::Element* root,Segment seg)
{
	root->set_name("segment");
	encode_vector(root->add_child("p1")->add_child("vector"),seg.p1);
	encode_vector(root->add_child("t1")->add_child("vector"),seg.t1);
	encode_vector(root->add_child("p2")->add_child("vector"),seg.p2);
	encode_vector(root->add_child("t2")->add_child("vector"),seg.t2);
	return root;
}

xmlpp::Element* encode_bline_point(xmlpp::Element* root,BLinePoint bline_point)
{
	root->set_name(type_bline_point.description.name);

	encode_vector(root->add_child("vertex")->add_child("vector"),bline_point.get_vertex());
	encode_vector(root->add_child("t1")->add_child("vector"),bline_point.get_tangent1());

	if(bline_point.get_split_tangent_both())
		encode_vector(root->add_child("t2")->add_child("vector"),bline_point.get_tangent2());

	encode_real(root->add_child("width")->add_child("real"),bline_point.get_width());
	encode_real(root->add_child("origin")->add_child("real"),bline_point.get_origin());
	return root;
}

xmlpp::Element* encode_width_point(xmlpp::Element* root,WidthPoint width_point)
{
	root->set_name(type_width_point.description.name);
	encode_real(root->add_child("position")->add_child("real"),width_point.get_position());
	encode_real(root->add_child("width")->add_child("real"),width_point.get_width());
	encode_integer(root->add_child("side_before")->add_child("integer"),width_point.get_side_type_before());
	encode_integer(root->add_child("side_after")->add_child("integer"),width_point.get_side_type_after());
	return root;
}

xmlpp::Element* encode_dash_item(xmlpp::Element* root, DashItem dash_item)
{
	root->set_name(type_dash_item.description.name);
	encode_real(root->add_child("offset")->add_child("real"),dash_item.get_offset());
	encode_real(root->add_child("length")->add_child("real"),dash_item.get_length());
	encode_integer(root->add_child("side_before")->add_child("integer"),dash_item.get_side_type_before());
	encode_integer(root->add_child("side_after")->add_child("integer"),dash_item.get_side_type_after());
	return root;
}

xmlpp::Element* encode_gradient(xmlpp::Element* root,Gradient x)
{
	root->set_name("gradient");
	x.sort();
	for (Gradient::const_iterator iter = x.begin(); iter != x.end(); ++iter) {
		xmlpp::Element *cpoint(encode_color(root->add_child("color"),iter->color));
		cpoint->set_attribute("pos",strprintf("%f",iter->pos));
	}
	return root;
}


xmlpp::Element* encode_value(xmlpp::Element* root,const ValueBase &data,Canvas::ConstHandle canvas=nullptr);

xmlpp::Element* encode_list(xmlpp::Element* root,std::vector<ValueBase> list, Canvas::ConstHandle canvas=nullptr)
{
	root->set_name("list");

	while(!list.empty())
	{
		encode_value(root->add_child("value"),list.front(),canvas);
		list.erase(list.begin());
	}

	return root;
}

xmlpp::Element* encode_transformation(xmlpp::Element* root,const Transformation &transformation)
{
	root->set_name("transformation");
	encode_vector(root->add_child("offset")->add_child("vector"),transformation.offset);
	encode_angle(root->add_child("angle")->add_child("angle"),transformation.angle);
	encode_angle(root->add_child("skew_angle")->add_child("angle"),transformation.skew_angle);
	encode_vector(root->add_child("scale")->add_child("vector"),transformation.scale);
	return root;
}

xmlpp::Element* encode_weighted_value(xmlpp::Element* root,types_namespace::TypeWeightedValueBase &type, const ValueBase &data,Canvas::ConstHandle canvas)
{
	root->set_name(type.description.name);
	encode_real(root->add_child("weight")->add_child("real"), type.extract_weight(data));
	encode_value(root->add_child("value")->add_child("value"), type.extract_value(data), canvas);
	return root;
}

xmlpp::Element* encode_pair(xmlpp::Element* root,types_namespace::TypePairBase &type, const ValueBase &data,Canvas::ConstHandle canvas)
{
	root->set_name(type.description.name);
	encode_value(root->add_child("first")->add_child("value"), type.extract_first(data), canvas);
	encode_value(root->add_child("second")->add_child("value"), type.extract_second(data), canvas);
	return root;
}

xmlpp::Element* encode_value(xmlpp::Element* root,const ValueBase &data,Canvas::ConstHandle canvas)
{
	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_value (type %s)\n", __FILE__, __LINE__, data.get_type().description.name.c_str());
	Type &type(data.get_type());
	if (type == type_real)
	{
		encode_real(root,data.get(Real()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_time)
	{
		encode_time(root,data.get(Time()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_integer)
	{
		encode_integer(root,data.get(int()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_color)
	{
		encode_color(root,data.get(Color()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_vector)
	{
		encode_vector(root,data.get(Vector()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_angle)
	{
		encode_angle(root,data.get(Angle()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_bool)
	{
		encode_bool(root,data.get(bool()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_string)
	{
		encode_string(root,data.get(String()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_segment)
	{
		encode_segment(root,data.get(Segment()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_bline_point)
		return encode_bline_point(root,data.get(BLinePoint()));
	if (type == type_width_point)
		return encode_width_point(root,data.get(WidthPoint()));
	if (type == type_dash_item)
		return encode_dash_item(root,data.get(DashItem()));
	if (type == type_gradient)
	{
		encode_gradient(root,data.get(Gradient()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_transformation)
	{
		encode_transformation(root,data.get(Transformation()));
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_list)
		return encode_list(root,data.get_list(),canvas);
	if (type == type_canvas)
	{
		return encode_canvas(root,data.get(Canvas::Handle()).get());
		//encode_static(root, data.get_static());
	}
	if (type == type_bone_valuenode)
	{
		if (!canvas)
		{
			printf("%s:%d ------------------------------------------------------------------------\n", __FILE__, __LINE__);
			printf("%s:%d zero canvas - please fix - report\n", __FILE__, __LINE__);
			printf("%s:%d ------------------------------------------------------------------------\n", __FILE__, __LINE__);
		}
		root = encode_value_node_bone_id(root,data.get(ValueNode_Bone::Handle()).get(),canvas);
		root->set_name("bone_valuenode");
		return root;
	}
	if (types_namespace::TypeWeightedValueBase* tw = dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type))
	{
		encode_weighted_value(root, *tw, data, canvas);
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (types_namespace::TypePairBase* tp = dynamic_cast<types_namespace::TypePairBase*>(&type))
	{
		encode_pair(root, *tp, data, canvas);
		encode_static(root, data.get_static());
		encode_interpolation(root, data.get_interpolation(), "interpolation");
		return root;
	}
	if (type == type_nil)
	{
		synfig::error("Encountered NIL ValueBase");
		root->set_name("nil");
		return root;
	}

	synfig::error(strprintf(_("Unknown value(%s), cannot create XML representation!"), data.get_type().description.local_name.c_str()));
	root->set_name("nil");
	return root;
}

xmlpp::Element* encode_animated(xmlpp::Element* root,ValueNode_Animated::ConstHandle value_node,Canvas::ConstHandle canvas=nullptr)
{
	assert(value_node);
	root->set_name("animated");

	root->set_attribute("type",value_node->get_type().description.name);

	const ValueNode_Animated::WaypointList &waypoint_list=value_node->waypoint_list();
	ValueNode_Animated::WaypointList::const_iterator iter;
	
	encode_interpolation(root, value_node->get_interpolation(), "interpolation");
	
	for(iter=waypoint_list.begin();iter!=waypoint_list.end();++iter)
	{
		xmlpp::Element *waypoint_node=root->add_child("waypoint");
		waypoint_node->set_attribute("time",iter->get_time().get_string());

		if(iter->get_value_node()->is_exported())
			waypoint_node->set_attribute("use",iter->get_value_node()->get_relative_id(canvas));
		else {
			ValueNode::ConstHandle value_node = iter->get_value_node();
			if(ValueNode_Const::ConstHandle value_node_const = ValueNode_Const::ConstHandle::cast_dynamic(value_node))
			{
				const ValueBase data = value_node_const->get_value();
				if (data.get_type() == type_canvas)
					waypoint_node->set_attribute("use",data.get(Canvas::Handle()).get()->get_relative_id(canvas));
				else
					encode_value_node(waypoint_node->add_child("value_node"),iter->get_value_node(),canvas);
			}
			else
				encode_value_node(waypoint_node->add_child("value_node"),iter->get_value_node(),canvas);
		}
		
		if (iter->get_before()!=INTERPOLATION_UNDEFINED)
			encode_interpolation(waypoint_node,iter->get_before(),"before");
		else
			error("Unknown waypoint type for \"before\" attribute");

		if (iter->get_after()!=INTERPOLATION_UNDEFINED)
			encode_interpolation(waypoint_node,iter->get_after(),"after");
		else
			error("Unknown waypoint type for \"after\" attribute");

		if(iter->get_tension()!=0.0)
			waypoint_node->set_attribute("tension",strprintf("%f",iter->get_tension()));
		if(iter->get_temporal_tension()!=0.0)
			waypoint_node->set_attribute("temporal-tension",strprintf("%f",iter->get_temporal_tension()));
		if(iter->get_continuity()!=0.0)
			waypoint_node->set_attribute("continuity",strprintf("%f",iter->get_continuity()));
		if(iter->get_bias()!=0.0)
			waypoint_node->set_attribute("bias",strprintf("%f",iter->get_bias()));

	}

	return root;
}


xmlpp::Element* encode_static_list(xmlpp::Element* root,ValueNode_StaticList::ConstHandle value_node,Canvas::ConstHandle canvas=nullptr)
{
	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_static_list %s\n", __FILE__, __LINE__, value_node->get_string().c_str());
	assert(value_node);

	root->set_name(value_node->get_name());

	root->set_attribute("type",value_node->get_contained_type().description.name);

	std::vector<ValueNode::RHandle>::const_iterator iter;

	for(iter=value_node->list.begin();iter!=value_node->list.end();++iter)
	{
		xmlpp::Element	*entry_node=root->add_child("entry");
		assert(*iter);
		if(!(*iter)->get_id().empty())
			entry_node->set_attribute("use",(*iter)->get_relative_id(canvas));
		else
		{
			DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode entry %s\n", __FILE__, __LINE__, (*iter)->get_string().c_str());
			encode_value_node(entry_node->add_child("value_node"),*iter,canvas);
		}
	}

	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_static_list %s done\n", __FILE__, __LINE__, value_node->get_string().c_str());
	return root;
}

xmlpp::Element* encode_dynamic_list(xmlpp::Element* root,ValueNode_DynamicList::ConstHandle value_node,Canvas::ConstHandle canvas=nullptr)
{
	assert(value_node);
	const float fps(canvas?canvas->rend_desc().get_frame_rate():0);

	root->set_name(value_node->get_name());

	root->set_attribute("type",value_node->get_contained_type().description.name);

	std::vector<ValueNode_DynamicList::ListEntry>::const_iterator iter;

	bool must_rotate_point_list = false;

	if(ValueNode_BLine::ConstHandle bline_value_node = ValueNode_BLine::ConstHandle::cast_dynamic(value_node))
	{
		if(bline_value_node->get_loop())
		{
			if (save_canvas_version < RELEASE_VERSION_1_4_0) // or get_file_version()?
				must_rotate_point_list = true;
		}
	}

	std::vector<ValueNode_DynamicList::ListEntry> corrected_valuenode_list = value_node->list;

	if (must_rotate_point_list) {
		if (must_rotate_point_list) {
			if (corrected_valuenode_list.size() > 0) {
				auto node = corrected_valuenode_list.front();
				corrected_valuenode_list.push_back(node);
				corrected_valuenode_list.erase(corrected_valuenode_list.begin());
			}
		}
	}

	if (value_node->get_loop())
		root->set_attribute("loop","true");

	for(iter=corrected_valuenode_list.begin();iter!=corrected_valuenode_list.end();++iter)
	{
		xmlpp::Element	*entry_node=root->add_child("entry");
		assert(iter->value_node);
		if(!iter->value_node->get_id().empty())
			entry_node->set_attribute("use",iter->value_node->get_relative_id(canvas));
		else
			encode_value_node(entry_node->add_child("value_node"),iter->value_node,canvas);

		// process waypoints
		{
			typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;
			String begin_sequence;
			String end_sequence;

			const ActivepointList& timing_info(iter->timing_info);
			ActivepointList::const_iterator entry_iter;

			for(entry_iter=timing_info.begin();entry_iter!=timing_info.end();++entry_iter)
				if(entry_iter->state==true)
				{
					if(entry_iter->priority)
						begin_sequence+=strprintf("p%d ",entry_iter->priority);
					begin_sequence+=entry_iter->time.get_string(fps)+", ";
				}
				else
				{
					if(entry_iter->priority)
						end_sequence+=strprintf("p%d ",entry_iter->priority);
					end_sequence+=entry_iter->time.get_string(fps)+", ";
				}

			// If this is just a plane-jane vanilla entry,
			// then don't bother with begins and ends
			if(end_sequence.empty() && begin_sequence=="SOT, ")
				begin_sequence.clear();

			if(!begin_sequence.empty())
			{
				// Remove the last ", " stuff
				begin_sequence=String(begin_sequence.begin(),begin_sequence.end()-2);
				// Add the attribute
				entry_node->set_attribute("on",begin_sequence);
			}

			if(!end_sequence.empty())
			{
				// Remove the last ", " stuff
				end_sequence=String(end_sequence.begin(),end_sequence.end()-2);
				// Add the attribute
				entry_node->set_attribute("off",end_sequence);
			}
		}
	}

	return root;
}

// Generic linkable data node entry
xmlpp::Element* encode_linkable_value_node(xmlpp::Element* root,LinkableValueNode::ConstHandle value_node,Canvas::ConstHandle canvas=nullptr)
{
	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_linkable_value_node %s\n", __FILE__, __LINE__, value_node->get_string().c_str());
	assert(value_node);

	String name(value_node->get_name());
	ReleaseVersion saving_version(get_file_version());
	ReleaseVersion feature_version(ValueNodeRegistry::book().at(name).release_version);

	if (saving_version < feature_version)
	{
		valuenode_too_new_count++;
		warning("can't save <%s> valuenodes in this old file format version", name.c_str());

		ValueBase value((*value_node)(0));
		encode_value(root,value,canvas);

		return root;
	}

	root->set_name(name);

	root->set_attribute("type",value_node->get_type().description.name);

	synfig::ParamVocab child_vocab(value_node->get_children_vocab());
	synfig::ParamVocab::iterator iter(child_vocab.begin());
	for (int i = 0; i < value_node->link_count(); ++i, ++iter)
	{
		// printf("saving link %d : %s\n", i, value_node->link_local_name(i).c_str());
		ValueNode::ConstHandle link=value_node->get_link(i).constant();
		if(!link)
			throw std::runtime_error("Bad link");
		if(link->is_exported())
			root->set_attribute(value_node->link_name(i),link->get_relative_id(canvas));
		else if(iter->get_critical())
		{
			if (name == "bone" && value_node->link_name(i) == "parent")
			{
				DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d saving bone's parent\n", __FILE__, __LINE__);
			}
			encode_value_node(root->add_child(value_node->link_name(i))->add_child("value_node"),link,canvas);
		}
	}

	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_linkable_value_node %s done\n", __FILE__, __LINE__, value_node->get_string().c_str());
	return root;
}

xmlpp::Element* encode_value_node(xmlpp::Element* root,ValueNode::ConstHandle value_node,Canvas::ConstHandle canvas)
{
	assert(value_node);
	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_value_node %s %s\n", __FILE__, __LINE__, value_node->get_string().c_str(), value_node->get_guid().get_string().c_str());

	if(value_node->rcount()>1)
		root->set_attribute("guid",(value_node->get_guid()^canvas->get_root()->get_guid()).get_string());

	if(ValueNode_Bone::ConstHandle value_node_bone = ValueNode_Bone::ConstHandle::cast_dynamic(value_node))
	{
		DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d shortcutting for valuenode_bone\n", __FILE__, __LINE__);
		encode_value_node_bone_id(root, value_node_bone,canvas);
	}
	else
	if (ValueNode_Animated::ConstHandle animated_value_node = ValueNode_Animated::ConstHandle::cast_dynamic(value_node))
		encode_animated(root,animated_value_node,canvas);
	else
	if (ValueNode_StaticList::ConstHandle static_list_value_node = ValueNode_StaticList::ConstHandle::cast_dynamic(value_node))
		encode_static_list(root,static_list_value_node,canvas);
	else
	if (ValueNode_DynamicList::ConstHandle dynamic_list_value_node = ValueNode_DynamicList::ConstHandle::cast_dynamic(value_node))
	{
		encode_dynamic_list(root,dynamic_list_value_node,canvas);
	}
	// if it's a ValueNode_Const
	else if (ValueNode_Const::ConstHandle const_value_node = ValueNode_Const::ConstHandle::cast_dynamic(value_node))
	{
		DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d got ValueNode_Const encoding value\n", __FILE__, __LINE__);
		// encode its get_value()
		encode_value(root,const_value_node->get_value(),canvas);
	}
	else
	if (LinkableValueNode::ConstHandle linkable_value_node = LinkableValueNode::ConstHandle::cast_dynamic(value_node))
		encode_linkable_value_node(root,linkable_value_node,canvas);
	else
	{
		error(_("Unknown ValueNode Type (%s), cannot create an XML representation"),value_node->get_local_name().c_str());
		root->set_name("nil");
	}

	assert(root);

	if(!value_node->get_id().empty())
		root->set_attribute("id",value_node->get_id());

//	if(ValueNode_Bone::ConstHandle::cast_dynamic(value_node))
//		root->set_attribute("guid",value_node->get_guid().get_string());

	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_value_node %s done\n", __FILE__, __LINE__, value_node->get_string().c_str());
	return root;
}

xmlpp::Element* encode_value_node_bone(xmlpp::Element* root,ValueNode::ConstHandle value_node,Canvas::ConstHandle canvas)
{
	assert(value_node);
	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_value_node_bone %s %s\n", __FILE__, __LINE__, value_node->get_string().c_str(), value_node->get_guid().get_string().c_str());

	if (ValueNode_Bone::ConstHandle bone_value_node = ValueNode_Bone::ConstHandle::cast_dynamic(value_node))
		encode_linkable_value_node(root,bone_value_node,canvas);
	else
	{
		error(_("Unknown ValueNode Type (%s), cannot create an XML representation"),value_node->get_local_name().c_str());
		assert(0);
		root->set_name("nil");
	}

	assert(root);

	if(!value_node->get_id().empty())
		root->set_attribute("id",value_node->get_id());

	if(ValueNode_Bone::ConstHandle::cast_dynamic(value_node))
		root->set_attribute("guid",(value_node->get_guid()^canvas->get_root()->get_guid()).get_string());

	if(value_node->rcount()>1)
	{
		// ~/notes/synfig/crash-when-saving.txt is an example of the execution reaching this line
		printf("%s:%d xxx value_node->rcount() = %d\n", __FILE__, __LINE__, value_node->rcount());
		root->set_attribute("guid",(value_node->get_guid()^canvas->get_root()->get_guid()).get_string());
	}

	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_value_node %s done\n", __FILE__, __LINE__, value_node->get_string().c_str());
	return root;
}

xmlpp::Element* encode_value_node_bone_id(xmlpp::Element* root,ValueNode::ConstHandle value_node,Canvas::ConstHandle canvas)
{
	root->set_name("bone");
	root->set_attribute("type",type_bone_object.description.name);
	DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d encode_value_node_bone_id %s %s\n", __FILE__, __LINE__, value_node->get_string().c_str(), value_node->get_guid().get_string().c_str());
	if(!value_node->get_id().empty())
		root->set_attribute("id",value_node->get_id());

	if(ValueNode_Bone::ConstHandle::cast_dynamic(value_node))
	{
		DEBUG_LOG("SYNFIG_DEBUG_SAVE_CANVAS", "%s:%d bone guid case 1 guid %s\n", __FILE__, __LINE__, value_node->get_guid().get_string().c_str());
		root->set_attribute("guid",(value_node->get_guid()^canvas->get_root()->get_guid()).get_string());
	}

	if(value_node->rcount()>1)
	{
		printf("%s:%d this happens too\n", __FILE__, __LINE__);
		root->set_attribute("guid",(value_node->get_guid()^canvas->get_root()->get_guid()).get_string());
	}

	return root;
}

xmlpp::Element* encode_layer(xmlpp::Element* root,Layer::ConstHandle layer)
{
	root->set_name("layer");

	root->set_attribute("type",layer->get_name());
	root->set_attribute("active",layer->active()?"true":"false");
	root->set_attribute("exclude_from_rendering",layer->get_exclude_from_rendering()?"true":"false");

	if(!layer->get_version().empty())
		root->set_attribute("version",layer->get_version());
	if(!layer->get_description().empty())
		root->set_attribute("desc",layer->get_description());
	if(!layer->get_group().empty())
		root->set_attribute("group",layer->get_group());

	Layer::Vocab vocab(layer->get_param_vocab());
	Layer::Vocab::const_iterator iter;

	const Layer::DynamicParamList &dynamic_param_list=layer->dynamic_param_list();

	for(iter=vocab.begin();iter!=vocab.end();++iter)
	{
		// Handle dynamic parameters
		if(dynamic_param_list.count(iter->get_name()))
		{
			xmlpp::Element *node=root->add_child("param");
			node->set_attribute("name",iter->get_name());

			ValueNode::ConstHandle value_node=dynamic_param_list.find(iter->get_name())->second;

			// If the valuenode has no ID, then it must be defined in-place
			if(value_node->get_id().empty())
			{
				encode_value_node(node->add_child("value_node"),value_node,layer->get_canvas().constant());
			}
			else
			{
				node->set_attribute("use",value_node->get_relative_id(layer->get_canvas()));
			}
		}
		else  // Handle normal parameters
		if(iter->get_critical())
		{
			ValueBase value=layer->get_param(iter->get_name());
			if(!value.is_valid())
			{
				error("Layer doesn't know its own vocabulary -- "+iter->get_name());
				continue;
			}

			if(value.get_type()==type_canvas)
			{
				// the ->is_inline() below was crashing if the canvas
				// contained a PasteCanvas with the default <No Image
				// Selected> Canvas setting;  this avoids the crash
				if (!value.get(Canvas::LooseHandle()))
					continue;

				if (!value.get(Canvas::LooseHandle())->is_inline())
				{
					Canvas::Handle child(value.get(Canvas::LooseHandle()));

					if(!value.get(Canvas::Handle()))
						continue;
					xmlpp::Element *node=root->add_child("param");
					node->set_attribute("name",iter->get_name());
					node->set_attribute("use",child->get_relative_id(layer->get_canvas()));
					if(value.get_static())
 						node->set_attribute("static", value.get_static()?"true":"false");
					continue;
				}
			}
			xmlpp::Element *node=root->add_child("param");
			node->set_attribute("name",iter->get_name());

			// remember filename param if need
			if (save_canvas_external_file_callback != nullptr
			 && iter->get_name() == "filename"
			 && value.get_type() == type_string)
			{
				std::string filename( value.get(String()) );
				std::string ext = filesystem::Path::filename_extension(filename);
				if (!ext.empty()) ext = ext.substr(1); // skip initial '.'
				bool registered_in_importer = Importer::book().count(ext) > 0;
				bool supports_by_importer = registered_in_importer
						                 && Importer::book()[ext].supports_file_system_wrapper;
				bool supports = supports_by_importer;
				if (supports)
					if (save_canvas_external_file_callback(save_canvas_external_file_user_data, layer, iter->get_name(), filename))
						value.set(filename);
			}

			encode_value(node->add_child("value"),value,layer->get_canvas().constant());
		}
	}


	return root;
}

xmlpp::Element* encode_canvas(xmlpp::Element* root,Canvas::ConstHandle canvas)
{
	assert(canvas);
	const RendDesc &rend_desc=canvas->rend_desc();
	root->set_name("canvas");

	if(canvas->is_root())
	{
		if (save_canvas_version < RELEASE_VERSION_1_4_0)
			root->set_attribute("version","1.0");
		else
			root->set_attribute("version",canvas->get_version());
	}

	if(!canvas->get_id().empty() && !canvas->is_root() && !canvas->is_inline())
		root->set_attribute("id",canvas->get_id());

	if(!canvas->parent() || canvas->parent()->rend_desc().get_w()!=rend_desc.get_w())
		root->set_attribute("width",strprintf("%d",rend_desc.get_w()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_h()!=rend_desc.get_h())
		root->set_attribute("height",strprintf("%d",rend_desc.get_h()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_x_res()!=rend_desc.get_x_res())
		root->set_attribute("xres",strprintf("%f",rend_desc.get_x_res()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_y_res()!=rend_desc.get_y_res())
		root->set_attribute("yres",strprintf("%f",rend_desc.get_y_res()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_gamma()!=rend_desc.get_gamma())
	{
		root->set_attribute("gamma-r",strprintf("%f",rend_desc.get_gamma().get_r()));
		root->set_attribute("gamma-g",strprintf("%f",rend_desc.get_gamma().get_g()));
		root->set_attribute("gamma-b",strprintf("%f",rend_desc.get_gamma().get_b()));
	}

	if(!canvas->parent() ||
		canvas->parent()->rend_desc().get_tl()!=canvas->rend_desc().get_tl() ||
		canvas->parent()->rend_desc().get_br()!=canvas->rend_desc().get_br())
	root->set_attribute("view-box",strprintf(VIEW_BOX_FORMAT,
		rend_desc.get_tl()[0],
		rend_desc.get_tl()[1],
		rend_desc.get_br()[0],
		rend_desc.get_br()[1])
	);

	if(!canvas->parent() || canvas->parent()->rend_desc().get_antialias()!=canvas->rend_desc().get_antialias())
		root->set_attribute("antialias",strprintf("%d",rend_desc.get_antialias()));

	if(!canvas->parent())
		root->set_attribute("fps",strprintf(TIME_TYPE_FORMAT,rend_desc.get_frame_rate()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_time_start()!=canvas->rend_desc().get_time_start())
		root->set_attribute("begin-time",rend_desc.get_time_start().get_string(rend_desc.get_frame_rate()));

	if(!canvas->parent() || canvas->parent()->rend_desc().get_time_end()!=canvas->rend_desc().get_time_end())
		root->set_attribute("end-time",rend_desc.get_time_end().get_string(rend_desc.get_frame_rate()));

	if(!canvas->is_inline())
	{
		root->set_attribute("bgcolor",strprintf(VIEW_BOX_FORMAT,
			rend_desc.get_bg_color().get_r(),
			rend_desc.get_bg_color().get_g(),
			rend_desc.get_bg_color().get_b(),
			rend_desc.get_bg_color().get_a())
		);

		if(!canvas->get_name().empty())
			root->add_child("name")->set_child_text(canvas->get_name());
		if(!canvas->get_description().empty())
			root->add_child("desc")->set_child_text(canvas->get_description());
		if(!canvas->get_author().empty())
			root->add_child("author")->set_child_text(canvas->get_description());

		std::list<String> meta_keys(canvas->get_meta_data_keys());
		while(!meta_keys.empty())
		{
			xmlpp::Element* meta_element(root->add_child("meta"));
			meta_element->set_attribute("name",meta_keys.front());
			meta_element->set_attribute("content",canvas->get_meta_data(meta_keys.front()));
			meta_keys.pop_front();
		}
		for(KeyframeList::const_iterator iter=canvas->keyframe_list().begin();iter!=canvas->keyframe_list().end();++iter)
			encode_keyframe(root->add_child("keyframe"),*iter,canvas->rend_desc().get_frame_rate());
	}

	// Output the <bones> section
	if((!canvas->is_inline() && !ValueNode_Bone::get_bone_map(canvas).empty()))
	{
		xmlpp::Element *node=root->add_child("bones");

		encode_value_node_bone(node->add_child("value_node"),ValueNode_Bone::get_root_bone(),canvas);

		ValueNode_Bone::BoneList bone_list(ValueNode_Bone::get_ordered_bones(canvas));
		for(ValueNode_Bone::BoneList::iterator iter=bone_list.begin();iter!=bone_list.end();++iter)
		{
			ValueNode_Bone::Handle bone(*iter);
			encode_value_node_bone(node->add_child("value_node"),bone,canvas);
		}
	}

	// Output the <defs> section

	//! \todo check where the parentheses should really go - around the && or the ||?
	// I guess it should be the other way - but then, why would an inline canvas have either exported valuenode or child canvases?  it shouldn't, right?
	// (not an inline canvas)   (has some exported valuenodes)         (has some child canvases)

	//! If children is not empty (there are exported canvases in the current canvas)
	//! they must be listed in the defs section regardless the result of check the
	//! Value Node list (exported value nodes in the canvas) and if the canvas is
	//! in line or not. Inline canvases cannot have exported canvases inside.

	if((!canvas->is_inline() && !canvas->value_node_list().empty()) || !canvas->children().empty())
	{
		xmlpp::Element *node=root->add_child("defs");
		const ValueNodeList &value_node_list(canvas->value_node_list());

		for(ValueNodeList::const_iterator iter=value_node_list.begin();iter!=value_node_list.end();++iter)
		{
			// If the value_node is a constant, then use the shorthand
			if (ValueNode_Const::Handle value_node = ValueNode_Const::Handle::cast_dynamic(*iter))
			{
				reinterpret_cast<xmlpp::Element*>(encode_value(node->add_child("value"),value_node->get_value(),canvas))->set_attribute("id",value_node->get_id());
				continue;
			}
			encode_value_node(node->add_child("value_node"),*iter,canvas);
			// writeme
		}

		for(Canvas::Children::const_iterator iter=canvas->children().begin();iter!=canvas->children().end();++iter)
		{
			encode_canvas(node->add_child("canvas"),*iter);
		}
	}

	Canvas::const_reverse_iterator iter;

	for(iter=canvas->rbegin();iter!=canvas->rend();++iter)
		encode_layer(root->add_child("layer"),*iter);

	return root;
}

xmlpp::Element* encode_canvas_toplevel(xmlpp::Element* root,Canvas::ConstHandle canvas)
{
	valuenode_too_new_count = 0;

	xmlpp::Element* ret = encode_canvas(root, canvas);

	if (valuenode_too_new_count)
		warning("saved %d valuenodes as constant values in old file format\n", valuenode_too_new_count);

	return ret;
}

bool
synfig::save_canvas(const FileSystem::Identifier &identifier, Canvas::ConstHandle canvas, bool safe)
{
    ChangeLocale change_locale(LC_NUMERIC, "C");

	synfig::String tmp_filename(identifier.filename.u8string());
	if (safe)
		tmp_filename.append(".TMP");

	try
	{
		assert(canvas);
		xmlpp::Document document;

		encode_canvas_toplevel(document.create_root_node("canvas"),canvas);

		FileSystem::WriteStream::Handle stream = identifier.file_system->get_write_stream(tmp_filename);
		if (!stream)
		{
			synfig::error("synfig::save_canvas(): Unable to open file for write");
			return false;
		}

		if (identifier.filename.extension().u8string() == ".sifz")
			stream = FileSystem::WriteStream::Handle(new ZWriteStream(stream));

		document.write_to_stream_formatted(*stream, "UTF-8");

		// close stream
		stream.reset();

		if (safe)
		{
			if (!identifier.file_system->file_rename(tmp_filename, identifier.filename.u8string())) {
				synfig::error("synfig::save_canvas(): Unable to rename file to correct filename");
				return false;
			}
		}
	}
	catch(...) { synfig::error("synfig::save_canvas(): Caught unknown exception"); return false; }

	return true;
}

String
synfig::canvas_to_string(Canvas::ConstHandle canvas)
{
    ChangeLocale change_locale(LC_NUMERIC, "C");
	assert(canvas);

	xmlpp::Document document;

	encode_canvas_toplevel(document.create_root_node("canvas"),canvas);

	return document.write_to_string_formatted();
}

void
synfig::set_save_canvas_external_file_callback(save_canvas_external_file_callback_t callback, void *user_data)
{
	save_canvas_external_file_callback = callback;
	save_canvas_external_file_user_data = user_data;
}

void
synfig::set_file_version(ReleaseVersion version)
{
	save_canvas_version = version;
}

ReleaseVersion
synfig::get_file_version()
{
	return save_canvas_version;
}
