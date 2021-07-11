/* === S Y N F I G ========================================================= */
/*!	\file valuedescset.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
**  Copyright (c) 2013 Konstantin Dmitriev
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

#include <synfig/general.h>

#include "layerparamset.h"
#include "valuenodeconstset.h"
#include "valuedescconnect.h"
#include "waypointsetsmart.h"
#include "waypointset.h"

#include "valuedescset.h"
#include <synfigapp/canvasinterface.h>
#include <synfig/layers/layer_skeletondeformation.h>
#include <synfig/valuenodes/valuenode_add.h>
#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/valuenodes/valuenode_wplist.h>
#include <synfig/valuenodes/valuenode_blinecalctangent.h>
#include <synfig/valuenodes/valuenode_blinecalcvertex.h>
#include <synfig/valuenodes/valuenode_blinecalcwidth.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/valuenodes/valuenode_radialcomposite.h>
#include <synfig/valuenodes/valuenode_range.h>
#include <synfig/valuenodes/valuenode_reference.h>
#include <synfig/valuenodes/valuenode_boneinfluence.h>
#include <synfig/valuenodes/valuenode_scale.h>
#include <synfig/valuenodes/valuenode_integer.h>
#include <synfig/valuenodes/valuenode_real.h>
#include <synfig/valuenodes/valuenode_bonelink.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_average.h>
#include <synfig/valuenodes/valuenode_weightedaverage.h>
#include <synfig/valueoperations.h>
#include <synfig/weightedvalue.h>
#include <synfig/pair.h>
#include <synfig/segment.h>
#include <synfigapp/main.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

ACTION_INIT_NO_GET_LOCAL_NAME(Action::ValueDescSet);
ACTION_SET_NAME(Action::ValueDescSet,"ValueDescSet");
ACTION_SET_LOCAL_NAME(Action::ValueDescSet,N_("Set ValueDesc"));
ACTION_SET_TASK(Action::ValueDescSet,"set");
ACTION_SET_CATEGORY(Action::ValueDescSet,Action::CATEGORY_VALUEDESC);
ACTION_SET_PRIORITY(Action::ValueDescSet,0);
ACTION_SET_VERSION(Action::ValueDescSet,"0.0");

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::ValueDescSet::ValueDescSet():
	time(0),
	recursive(true),
	animate(false),
	lock_animation(false)
{
}

synfig::String
Action::ValueDescSet::get_local_name()const
{
	return strprintf(_("Set %s"),
					 value_desc
					 ? value_desc.get_description().c_str()
					 : _("ValueDesc"));
}

Action::ParamVocab
Action::ValueDescSet::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("value_desc",Param::TYPE_VALUEDESC)
		.set_local_name(_("ValueDesc"))
	);

	ret.push_back(ParamDesc("new_value",Param::TYPE_VALUE)
		.set_local_name(_("ValueBase"))
	);

	ret.push_back(ParamDesc("time",Param::TYPE_TIME)
		.set_local_name(_("Time"))
		.set_optional()
	);
	ret.push_back(ParamDesc("recursive", Param::TYPE_BOOL)
		.set_local_name(_("Recursive"))
		.set_optional()
	);
	ret.push_back(ParamDesc("animate", Param::TYPE_BOOL)
		.set_local_name(_("Animate"))
		.set_optional()
	);
	ret.push_back(ParamDesc("lock_animation", Param::TYPE_BOOL)
		.set_local_name(_("Lock animation"))
		.set_optional()
	);

	return ret;
}

bool
Action::ValueDescSet::is_candidate(const ParamList &x)
{
	return candidate_check(get_param_vocab(),x);
}

bool
Action::ValueDescSet::set_param(const synfig::String& name, const Action::Param &param)
{
	if(name=="value_desc" && param.get_type()==Param::TYPE_VALUEDESC)
	{
		value_desc=param.get_value_desc();
		return true;
	}
	if(name=="new_value" && param.get_type()==Param::TYPE_VALUE)
	{
		value=param.get_value();
		return true;
	}
	if(name=="time" && param.get_type()==Param::TYPE_TIME)
	{
		time=param.get_time();
		return true;
	}
	if(name=="recursive" && param.get_type()==Param::TYPE_BOOL)
	{
		recursive=param.get_bool();
		return true;
	}
	if(name=="animate" && param.get_type()==Param::TYPE_BOOL)
	{
		animate=param.get_bool();
		return true;
	}
	if(name=="lock_animation" && param.get_type()==Param::TYPE_BOOL)
	{
		lock_animation=param.get_bool();
		return true;
	}

	return Action::CanvasSpecific::set_param(name,param);
}

bool
Action::ValueDescSet::is_ready()const
{
	if(!value_desc || !value.is_valid())
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::ValueDescSet::prepare()
{
	clear();

	get_canvas_interface()->signal_value_desc_set()(value_desc,value);

	// If current tool is Bone Tool, changing a bone of a Skeleton Deformation
	// Layer should affect its pair too. One follow each other with Bone Tool.
	// Just remembering: Skeleton Deformation a list of bone pairs:
	//   * the first element of every pair compounds the resting pose
	//   * the second element of every pair compounds the current animated pose
	if(get_canvas_interface()->get_state()=="bone") {
		ValueNode_Composite::Handle comp;

		// Skeleton Deformation layer > Bone pair list > Bone pair item > Bone
		const ValueDesc grand_parent = value_desc.get_parent_desc().get_parent_desc();
		if (grand_parent.get_parent_desc().parent_is_layer()) {
			if (dynamic_cast<Layer_SkeletonDeformation*>(grand_parent.get_parent_desc().get_layer().get())) {
				types_namespace::TypePair<Bone,Bone> type_bone_pair;
				type_bone_pair.initialize();
				if (grand_parent.parent_is_value_node() && grand_parent.get_parent_value_node()->get_type() == type_list)
					if (value_desc.get_parent_desc().parent_is_value_node()
						&& value_desc.get_parent_desc().get_parent_value_node()->get_type() == type_bone_pair)
					{
						comp = ValueNode_Composite::Handle::cast_dynamic(grand_parent.get_value_node());
					}
			}
		}

		if(comp && value_desc.get_parent_value_node() == comp->get_link("first")){
			ValueNode_Bone::Handle bone = ValueNode_Bone::Handle::cast_dynamic(comp->get_link("second"));
			ValueNode_Bone::Handle bone1 = 	ValueNode_Bone::Handle::cast_dynamic(comp->get_link("first"));
			if(bone){
				const int index = value_desc.get_index();
				const int origin_index = bone->get_link_index_from_name("origin");
				const int angle_index = bone->get_link_index_from_name("angle");
				const int scalelx_index = bone->get_link_index_from_name("scalelx");
				if(index==origin_index || index==angle_index || index==scalelx_index){
					ValueBase svalue(0);

					if(index==origin_index) {
						Point p = value.get(Point());
						ValueBase parentBone_vb = (*bone->get_link("parent"))(time);
						ValueBase parentBone_vb1 = (*bone1->get_link("parent"))(time);

						Bone parentBone;
						Bone parentBone1;

						if (parentBone_vb.get_type() == type_bone_valuenode) {
							parentBone = (*parentBone_vb.get(ValueNode_Bone::Handle()))(time).get(Bone());
						} else if (parentBone_vb.get_type() == type_bone_object) {
							parentBone = parentBone_vb.get(Bone());
						} else {
							throw Error(_("Parent bone type not supported"));
						}
						if (parentBone_vb1.get_type() == type_bone_valuenode) {
							parentBone1 = (*parentBone_vb1.get(ValueNode_Bone::Handle()))(time).get(Bone());
						} else if (parentBone_vb1.get_type() == type_bone_object) {
							parentBone1 = parentBone_vb1.get(Bone());
						} else {
							throw Error(_("Parent bone type not supported"));
						}

						Matrix parentAnimMatrix = parentBone.get_animated_matrix();
						Matrix parentAnimMatrix1 = parentBone1.get_animated_matrix();

						p += parentAnimMatrix.get_transformed(bone->get_link(index)->operator()(time).get(Point()));
						p -= parentAnimMatrix1.get_transformed(-bone1->get_link(index)->operator()(time).get(Point()));
						svalue = ValueBase(parentAnimMatrix.get_inverted().get_transformed(p));
					}else if(index==angle_index){
						Angle a = value.get(Angle());
						a+=bone->get_link(index)->operator()(time).get(Angle());
						a-=bone1->get_link(index)->operator()(time).get(Angle());
						svalue = ValueBase(a);
					}else if(index==scalelx_index){
						Real r = value.get(Real());
						r+= bone->get_link(index)->operator()(time).get(Real());
						r-=bone1->get_link(index)->operator()(time).get(Real());
						svalue = ValueBase(r);
					}
					add_action_valuedescset(svalue,ValueDesc(bone,index));
				}
			}
		}
	}


	// If we are a reference value node, then
	// we need to distribute the changes to the
	// referenced value node
	if(ValueNode_Reference::Handle reference_value_node = ValueNode_Reference::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueDesc reference_value_desc(reference_value_node,0);
		add_action_valuedescset(value,reference_value_desc);
		return;
	}

	// if we are a boneinfluence value node, then we need to distribute the changes to the linked value node
	// This is only valid for vector type converted to bone influence
	// Not valid for a whole blinepoint converted to bone influence
	if(value_desc.is_value_node() && value.get_type() == type_vector)
	{
		if (ValueNode_BoneInfluence::Handle bone_influence_value_node =
			ValueNode_BoneInfluence::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueDesc bone_influence_value_desc(bone_influence_value_node,
												bone_influence_value_node->get_link_index_from_name("link"));

			if (bone_influence_value_node->has_inverse_transform())
				value = bone_influence_value_node->get_inverse_transform().get_transformed(value.get(Vector()));
			else
				throw Error(_("this node isn't editable - in the future it will be greyed to prevent editing"));

			add_action_valuedescset(value,bone_influence_value_desc);

			return;
		}
	}

    // Set ValueNode_Average
	if(value_desc.is_value_node() && ValueNode_Average::check_type(value.get_type()))
	{
		if (ValueNode_Average::Handle bone_average_value_node =
			ValueNode_Average::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueBase::List values_list;
			values_list.reserve(bone_average_value_node->link_count());
			for(int i = 0; i < bone_average_value_node->link_count(); ++i)
				values_list.push_back((*bone_average_value_node->get_link(i))(time));

			ValueAverage::set_average_value_generic(values_list.begin(), values_list.end(), value);

			for(int i = 0; i < bone_average_value_node->link_count(); ++i)
				add_action_valuedescset(values_list[i], ValueDesc(bone_average_value_node, i ));

			return;
		}
	}

    // Set ValueNode_WeightedAverage
	if(value_desc.is_value_node() && ValueNode_WeightedAverage::check_type(value.get_type()))
	{
		if (ValueNode_WeightedAverage::Handle bone_weighted_average_value_node =
			ValueNode_WeightedAverage::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueBase::List values_list;
			values_list.reserve(bone_weighted_average_value_node->link_count());
			for(int i = 0; i < bone_weighted_average_value_node->link_count(); ++i)
				values_list.push_back((*bone_weighted_average_value_node->get_link(i))(time));

			ValueBase values_list_value(values_list);
			ValueAverage::set_average_value_weighted(values_list_value, value);
			const ValueBase::List &new_values_list = values_list_value.get_list();

			for(int i = 0; i < bone_weighted_average_value_node->link_count(); ++i)
				add_action_valuedescset(new_values_list[i], ValueDesc(bone_weighted_average_value_node, i ));

			return;
		}
	}

	// If we are a bone link value node, then
	// we need to change the transformation part
	if(ValueNode_BoneLink::Handle value_node = ValueNode_BoneLink::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueDesc base_value_desc(value_node, value_node->get_link_index_from_name("base_value"));

		ValueBase new_base_value =
			ValueTransformation::back_transform(
				value_node->get_bone_transformation(time), value );

		add_action_valuedescset(new_base_value,base_value_desc);
		return;
	}

	ValueNode_Composite::Handle composite_value_node = ValueNode_Composite::Handle::cast_dynamic(value_desc.get_value_node());

	// If we are a composite value node, then
	// we need to distribute the changes to the
	// individual parts
	// except if we are TYPE WIDTHPOINT which is handled individually
	// except if we are TYPE BLINEPOINT which is handled individually
	if (composite_value_node
	 && value_desc.get_value_node()->get_type()!=type_width_point
	 && value_desc.get_value_node()->get_type()!=type_bline_point)
	{
		int index = value_desc.parent_is_value_desc()
				  ? composite_value_node->get_link_index_from_name(value_desc.get_sub_name())
				  : -1;

		ValueBase components[8];
		int n_components(0);
		Type &type(value.get_type());
		if (type == type_vector)
		{
			components[0]=value.get(Vector())[0];
			components[1]=value.get(Vector())[1];
			n_components=2;
		}
		else
		if (type == type_color)
		{
			components[0]=value.get(Color()).get_r();
			components[1]=value.get(Color()).get_g();
			components[2]=value.get(Color()).get_b();
			components[3]=value.get(Color()).get_a();
			n_components=4;
		}
		else
		if (type == type_segment)
		{
			components[0]=value.get(Segment()).p1;
			components[1]=value.get(Segment()).t1;
			components[2]=value.get(Segment()).p2;
			components[3]=value.get(Segment()).t2;
			n_components=4;
		}
		else
		if (type == type_transformation)
		{
			components[0]=value.get(Transformation()).offset;
			components[1]=value.get(Transformation()).angle;
			components[2]=value.get(Transformation()).skew_angle;
			components[3]=value.get(Transformation()).scale;
			n_components=4;
		}
		else
		if (type == type_bline_point)
		{
			components[0]=value.get(BLinePoint()).get_vertex();
			components[1]=value.get(BLinePoint()).get_width();
			components[2]=value.get(BLinePoint()).get_origin();
			components[3]=value.get(BLinePoint()).get_split_tangent_both();
			components[4]=value.get(BLinePoint()).get_tangent1();
			components[5]=value.get(BLinePoint()).get_tangent2();
			components[6]=value.get(BLinePoint()).get_split_tangent_radius();
			components[7]=value.get(BLinePoint()).get_split_tangent_angle();
			n_components=8;
		}
		else
		if (types_namespace::TypeWeightedValueBase *tp = dynamic_cast<types_namespace::TypeWeightedValueBase*>(&type))
		{
			components[0]=tp->extract_weight(value);
			components[1]=tp->extract_value(value);
			n_components=2;
		}
		else
		if (types_namespace::TypePairBase *tp = dynamic_cast<types_namespace::TypePairBase*>(&type))
		{
			components[0]=tp->extract_first(value);
			components[1]=tp->extract_second(value);
			n_components=2;
		}
		else
			throw Error(_("Bad type for composite (%s)"), type.description.local_name.c_str());

		if (index >= 0 && index < n_components)
		{
			add_action_valuedescset(components[index],ValueDesc(composite_value_node,index));
		}
		else if (index < 0)
		{
			for(int i=0;i<n_components;i++)
			{
				ValueDesc component_value_desc(composite_value_node,i);
				add_action_valuedescset(components[i],component_value_desc);
			}
		}
		return;
	}

	// If we are a composite value node type BLINEPOINT, then
	// we need to distribute the changes to the
	// individual parts in a proper way
	if (composite_value_node
	 && value_desc.get_value_node()->get_type()==type_bline_point)
	{
		int index1 = value_desc.parent_is_value_desc()
				  ? composite_value_node->get_link_index_from_name(value_desc.get_sub_name())
				  : -1;
		int index2 = value_desc.parent_is_value_desc()
				  && value_desc.get_sub_name() == "t2"
				  && (!value.get(BLinePoint()).get_split_tangent_radius() || !value.get(BLinePoint()).get_split_tangent_angle())
				  ? composite_value_node->get_link_index_from_name("t1")
				  : -1;

		ValueBase components[8];
		int n_components(0);
		int order[8] = { 0,1,2,3,6,7,4,5 };
		components[0]=value.get(BLinePoint()).get_vertex();
		components[1]=value.get(BLinePoint()).get_width();
		components[2]=value.get(BLinePoint()).get_origin();
		components[3]=value.get(BLinePoint()).get_split_tangent_both();
		components[4]=value.get(BLinePoint()).get_tangent1();
		components[5]=value.get(BLinePoint()).get_tangent2();
		components[6]=value.get(BLinePoint()).get_split_tangent_radius();
		components[7]=value.get(BLinePoint()).get_split_tangent_angle();
		n_components=8;
		for(int i=0;i<n_components;i++)
		{
			if (index1 < 0 || index1 == order[i] || index2 == order[i])
			{
				ValueDesc component_value_desc(composite_value_node,order[i]);
				add_action_valuedescset(components[order[i]],component_value_desc,false);
			}
		}
		return;
	}

	// If we are a bone value node, then
	// we need to distribute the changes to the
	// individual parts except 'parent' and 'name' fields
	if (value.get_type() == type_bone_object
	 && ValueNode_Bone::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueNode_Bone::Handle bone_value_node =
			ValueNode_Bone::Handle::cast_dynamic(
				value_desc.get_value_node() );
		const Bone &bone = value.get(Bone());

		typedef std::pair<String, ValueBase> KeyValue;
		std::pair<String, ValueBase> values[] = {
			KeyValue("origin",   bone.get_origin()),
			KeyValue("angle",    bone.get_angle()),
			KeyValue("scalelx",  bone.get_scalelx()),
			KeyValue("width",    bone.get_width()),
			KeyValue("scalex",   bone.get_scalex()),
			KeyValue("tipwidth", bone.get_tipwidth()),
			KeyValue("bone_depth",    bone.get_depth()),
			KeyValue("length",   bone.get_length())
		};

		for(size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i)
		{
			ValueDesc component_value_desc(
				bone_value_node,
				bone_value_node->get_link_index_from_name(values[i].first));
			add_action_valuedescset(values[i].second,component_value_desc);
		}
		return;
	}

	// If we are a RADIAL composite value node, then
	// we need to distribute the changes to the
	// individual parts
	if(ValueNode_RadialComposite::Handle radialcomposite_value_node = ValueNode_RadialComposite::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase components[6];
		int n_components(0);
		Type &type(value.get_type());
		if (type == type_vector)
		{
			Angle old_angle = (*(radialcomposite_value_node->get_link("theta")))(time).get(Angle());
			Vector vect(value.get(Vector()));
			components[0]=vect.mag();
			Angle change = Angle(Angle::tan(vect[1],vect[0])) - old_angle;
			while (change < Angle::deg(-180)) change += Angle::deg(360);
			while (change > Angle::deg(180)) change -= Angle::deg(360);
			components[1]=old_angle + change;
			n_components=2;
		}
		else
		if (type == type_color)
		{
			components[0]=value.get(Color()).get_y();
			components[1]=value.get(Color()).get_s();
			components[2]=value.get(Color()).get_hue();
			components[3]=value.get(Color()).get_a();
			n_components=4;
		}
		else
			throw Error(_("Bad type for radial composite (%s)"), type.description.local_name.c_str());

		for(int i=0;i<n_components;i++)
		{
			ValueDesc component_value_desc(radialcomposite_value_node,i);
			add_action_valuedescset(components[i],component_value_desc);
		}
		return;
	}

	// Perform reverse manipulations

	// BlineCalcWidth: modify the scale value node
	// so that the target width is achieved
	if (ValueNode_BLineCalcWidth::Handle bline_width = ValueNode_BLineCalcWidth::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		Real old_width((*bline_width)(time).get(Real()));
		Real scale((*(bline_width->get_link("scale")))(time).get(Real()));
		ValueBase new_width(value.get(Real()) * scale / old_width);
		add_action_valuedescset(new_width,ValueDesc(bline_width, bline_width->get_link_index_from_name("scale")));
		return;
	}
	// BLineCalcVertex: snap the point to the nearest
	// allowed position.
	if (ValueNode_BLineCalcVertex::Handle bline_vertex = ValueNode_BLineCalcVertex::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueNode_BLine::Handle bline = ValueNode_BLine::Handle::cast_dynamic(bline_vertex->get_link("bline"));
		Real radius = 0.0;
		Real new_amount;
		if (((*(bline_vertex->get_link("loop")))(time).get(bool())))
		{
			// The bline is looped. Animation may require an amount parameter
			// outside the range of 0-1, so make sure that the amount does
			// not change drastically.
			Real amount_old((*(bline_vertex->get_link("amount")))(time).get(Real()));

			Real amount_new = synfig::find_closest_point((*bline)(time), value.get(Vector()), radius, bline->get_loop());
			Real difference = fmod( fmod(amount_new - amount_old, 1.0) + 1.0 , 1.0);
			//fmod is called twice to avoid negative values
			if (difference > 0.5)
				difference=difference-1.0;
			new_amount = amount_old+difference;
		}
		else
			new_amount = synfig::find_closest_point((*bline)(time), value.get(Vector()), radius, bline->get_loop());
		bool homogeneous((*(bline_vertex->get_link("homogeneous")))(time).get(bool()));
		if(homogeneous)
			new_amount=std_to_hom((*bline)(time), new_amount, (*(bline_vertex->get_link("loop")))(time).get(bool()), bline->get_loop() );
		add_action_valuedescset(new_amount,ValueDesc(bline_vertex, bline_vertex->get_link_index_from_name("amount")));
		return;
	}
	// BLineCalcTangent: adjust scale and offset
	// to achieve the desired tangent
	if (ValueNode_BLineCalcTangent::Handle bline_tangent = ValueNode_BLineCalcTangent::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		ValueBase new_scale(synfig::Real(0));
		ValueDesc scale_value_desc(bline_tangent,bline_tangent->get_link_index_from_name("scale"));
		ValueDesc offset_value_desc(bline_tangent,bline_tangent->get_link_index_from_name("offset"));
		Type &type(value_desc.get_value_type());
		if (type == type_real)
		{
			Real old_length = (*bline_tangent)(time).get(Real());
			Real new_length = value.get(Vector()).mag();
			Real scale((*(bline_tangent->get_link("scale")))(time).get(Real()));
			bool fixed_length((*(bline_tangent->get_link("fixed_length")))(time).get(bool()));
			if (fixed_length)
			{
				new_scale = new_length;
			}
			else
			{
				if (approximate_zero(old_length)) return;
				new_scale = new_length * scale / old_length;
			}
		}
		else
		if (type == type_vector)
		{
			Vector old_tangent = (*bline_tangent)(time).get(Vector());
			Angle old_angle = old_tangent.angle();
			Real old_length = old_tangent.mag();
			Angle new_angle = value.get(Vector()).angle();
			Real new_length = value.get(Vector()).mag();
			Angle old_offset((*(bline_tangent->get_link("offset")))(time).get(Angle()));
			Real scale((*(bline_tangent->get_link("scale")))(time).get(Real()));
			bool fixed_length((*(bline_tangent->get_link("fixed_length")))(time).get(bool()));
			if (fixed_length)
			{
				new_scale = new_length;
			}
			else
			if (approximate_not_zero(old_length))
			{
				new_scale = new_length * scale / old_length;
				add_action_valuedescset(ValueBase(old_offset + new_angle - old_angle),offset_value_desc);
			}
		}
		else
		if (type == type_angle)
		{
			Angle old_angle = (*bline_tangent)(time).get(Angle());
			Angle new_angle = value.get(Angle());
			Angle old_offset((*(bline_tangent->get_link("offset")))(time).get(Angle()));
			add_action_valuedescset(ValueBase(old_offset + new_angle - old_angle),offset_value_desc);
			return;
		}

		if (approximate_not_zero(new_scale.get(synfig::Real())))
			add_action_valuedescset(new_scale,scale_value_desc);
		return;
	}

	// WidthPoint Composite: adjust the width point position
	// to achieve the desired point on bline
	// (Code copied from BLineCalcVertex above)
	if (value_desc.parent_is_linkable_value_node() && value_desc.get_parent_value_node()->get_type() == type_list)
	{
		if(ValueNode_WPList::Handle wplist = ValueNode_WPList::Handle::cast_dynamic(value_desc.get_parent_value_node()))
		{
			bool wplistloop(wplist->get_loop());
			ValueNode_BLine::Handle bline(ValueNode_BLine::Handle::cast_dynamic(wplist->get_bline()));
			ValueNode_Composite::Handle wpoint_composite = composite_value_node;
			if(bline && wpoint_composite)
			{
				bool blineloop(bline->get_loop());
				// Retrieve the homogeneous layer parameter
				bool homogeneous=false;
				Layer::Handle layer_parent;
				std::set<Node*>::iterator iter;
				for(iter=wplist->parent_set.begin();iter!=wplist->parent_set.end();++iter)
					{
						Layer::Handle layer;
						layer=Layer::Handle::cast_dynamic(*iter);
						if(layer && layer->get_name() == "advanced_outline")
						{
							homogeneous=layer->get_param("homogeneous").get(bool());
							break;
						}
					}
				Real radius = 0.0;
				Real new_amount;
				WidthPoint wp((*wpoint_composite)(time).get(WidthPoint()));
				if (wplistloop)
				{
					// The wplist is looped. Animation may require a position parameter
					// outside the range of 0-1, so make sure that the position doesn't
					// change drastically.
					Real amount_old(wp.get_norm_position(wplistloop));
					Real amount_old_b(wp.get_bound_position(wplistloop));
					// If it is homogeneous then convert it to standard
					amount_old=homogeneous?hom_to_std((*bline)(time), amount_old, wplistloop, blineloop):amount_old;
					// grab a new position given by duck's position on the bline
					Real amount_new = synfig::find_closest_point((*bline)(time), value.get(Vector()), radius, blineloop);
					// calculate the difference between old and new amounts
					Real difference = fmod( fmod(amount_new - amount_old, 1.0) + 1.0 , 1.0);
					//fmod is called twice to avoid negative values
					if (difference > 0.5)
						difference=difference-1.0;
					// calculate a new value for the position
					new_amount=amount_old+difference;
					// restore the homogeneous value if needed
					new_amount = homogeneous ? std_to_hom((*bline)(time), new_amount, wplistloop, blineloop) : new_amount;
					// this is the difference between the new amount and the old amount inside the boundaries
					Real bound_diff((wp.get_lower_bound() + new_amount*(wp.get_upper_bound()-wp.get_lower_bound()))-amount_old_b);
					// add the new diff to the current amount
					new_amount = wp.get_position() + bound_diff;
				}
				else
				{
					// grab a new amount given by duck's position on the bline
					new_amount = synfig::find_closest_point((*bline)(time), value.get(Vector()), radius, blineloop);
					// if it is homogeneous then convert to it
					new_amount = homogeneous ? std_to_hom((*bline)(time), new_amount, wplistloop, blineloop) : new_amount;
					// convert the value inside the boundaries
					new_amount = wp.get_lower_bound()+new_amount*(wp.get_upper_bound()-wp.get_lower_bound());
				}
				add_action_valuedescset(new_amount,ValueDesc(wpoint_composite, wpoint_composite->get_link_index_from_name("position")));
				return;
			}
		}
	}

	// Linkable value nodes has methods to do reverse manipulations
	if (LinkableValueNode::Handle linkable_value_node = LinkableValueNode::Handle::cast_dynamic(value_desc.get_value_node()))
	{
		int link_index;
		LinkableValueNode::InvertibleStatus invertible = linkable_value_node->is_invertible(time, value, &link_index);
		switch (invertible) {
		case synfig::LinkableValueNode::INVERSE_OK: {
			ValueBase new_value = linkable_value_node->get_inverse(time, value);
			add_action_valuedescset(new_value,ValueDesc(linkable_value_node, link_index));
			return;
		}
		case synfig::LinkableValueNode::INVERSE_NOT_SUPPORTED:
			throw Error(_("Inverse manipulation of valuenode '%s' is not supported."),
			            linkable_value_node->get_type().description.local_name.c_str());
		case synfig::LinkableValueNode::INVERSE_ERROR_BAD_TYPE:
			throw Error(_("Inverse manipulation of valuenode '%s' with %s values is not implemented."),
			            linkable_value_node->get_type().description.local_name.c_str(),
			            value.type_name().c_str());
		case synfig::LinkableValueNode::INVERSE_ERROR_BAD_VALUE:
			throw Error(_("Inverse manipulation of valuenode '%s' is not possible for value %s."),
			            linkable_value_node->get_type().description.local_name.c_str(),
			            value.get_string().c_str());
		case synfig::LinkableValueNode::INVERSE_ERROR_BAD_TIME:
			throw Error(_("Inverse manipulation of valuenode '%s' is not possible at current time point %s."),
			            linkable_value_node->get_type().description.local_name.c_str(),
			            time.get_string(Time::FORMAT_NORMAL).c_str());
		case synfig::LinkableValueNode::INVERSE_ERROR_BAD_PARAMETER:
			throw Error(_("Attempt to edit valuenode '%s' is not possible due to another parameter of valuenode: %s."),
			            linkable_value_node->get_type().description.local_name.c_str(),
			            link_index >= 0 && link_index < linkable_value_node->link_count() ?
			                linkable_value_node->link_local_name(link_index).c_str()
			              : _("unknown parameter (please report)"));
		}
	}

	// end reverse manipulations

	// if value desc has parent value node and parent is composite widthpoint type and index is 4 or 5
	// then we are changing the value of a widthpoint boundary.
	// It is needed to check that we aren't doing the boundary range zero

	if(value_desc.parent_is_value_node() && ValueNode_Composite::Handle::cast_dynamic(value_desc.get_parent_value_node()))
	{
		ValueNode_Composite::Handle parent_value_node;
		parent_value_node=parent_value_node.cast_dynamic(value_desc.get_parent_value_node());
		assert(parent_value_node);
		int i=value_desc.get_index();
		if(parent_value_node->get_type() == type_width_point && (i==4 || i==5))
		{
			ValueNode::Handle low(parent_value_node->get_link("lower_bound"));
			ValueNode::Handle upp(parent_value_node->get_link("upper_bound"));
			Real new_value(value.get(Real()));
			Real lower = (*low)(Time(0.0)).get(Real());
			Real upper = (*upp)(Time(0.0)).get(Real());
			if( (i==4 && approximate_greater_or_equal(new_value, upper))
			||  (i==5 && approximate_less_or_equal(new_value, lower)) )
			{
				throw Error(_("It is forbidden to set lower boundary equal or bigger than upper boundary"));
				return;
			}
		}
	}

	// If we are changing the z_depth range parameters
	// send a signal to the canvas interface to say that the layer has changed
	if(value_desc.parent_is_layer()
	   &&
	   (value_desc.get_param_name() =="z_range"
		||
		value_desc.get_param_name() =="z_range_position"
		||
		value_desc.get_param_name() =="z_range_depth")
	   )
	{
		if (get_canvas_interface() && recursive)
		{
			get_canvas_interface()->signal_layer_z_range_changed()(value_desc.get_layer(),true);
		}
	}

	// If we are in animate editing mode
	// TODO: Can we replace local_value to value after all parameters will be converted into ValueBase type?
	if((animate || (get_edit_mode()&MODE_ANIMATE)) && !value_desc.get_static())
	{
		ValueNode_Animated::Handle& value_node(value_node_animated);
		// If this value isn't a ValueNode_Animated, but
		// it is somewhat constant, then go ahead and convert
		// it to a ValueNode_Animated.
		if(!value_desc.is_value_node() || ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
		{
			ValueBase value;
			if(value_desc.is_value_node())
				value=ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node())->get_value();
			else
				value=value_desc.get_value();
			Interpolation interp=value.get_interpolation();
			if(!value_node)value_node=ValueNode_Animated::create(value,time);
			// Be sure that the newly created waypoint is set with the default
			// interpolations.
			synfig::ValueNode_Animated::WaypointList::iterator iter(value_node->find(time));
			iter->set_before(interp==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():interp);
			iter->set_after(interp==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():interp);
			value_node->set_interpolation(interp);
			Action::Handle action;
			if(!value_desc.is_value_node())
			{
				action=(ValueDescConnect::create());
				action->set_param("dest",value_desc);
				action->set_param("src",ValueNode::Handle(value_node));
			}
			else
			{
				action=Action::create("ValueNodeReplace");
				action->set_param("dest",value_desc.get_value_node());
				action->set_param("src",ValueNode::Handle(value_node));
			}
			action->set_param("canvas",get_canvas());
			action->set_param("canvas_interface",get_canvas_interface());
			if(!action->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action_front(action);
		}
		else
		{
			value_node=value_node.cast_dynamic(value_desc.get_value_node());
		}
		if(!value_node)
			throw Error(_("Direct manipulation of this ValueNode type is not yet supported"));
		synfig::ValueNode_Animated::WaypointList::iterator iter;
		Waypoint waypoint;
		Action::Handle action(WaypointSetSmart::create());
		try
		{
			iter=value_node->find(time);
			// value_node->find throws an exception
			// when no waypoint is found at given time
			waypoint=*iter;
		}catch(Exception::NotFound&)
		{
			waypoint=value_node->new_waypoint_at_time(time);
			Interpolation inter=value_node->get_interpolation();
			waypoint.set_before(inter==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():inter);
			waypoint.set_after(inter==INTERPOLATION_UNDEFINED?synfigapp::Main::get_interpolation():inter);
		}
		waypoint.set_value(value);
		action->set_param("canvas",get_canvas());
		action->set_param("canvas_interface",get_canvas_interface());
		action->set_param("value_node",ValueNode::Handle(value_node));
		action->set_param("waypoint",waypoint);
		if(!action->is_ready())
			throw Error(Error::TYPE_NOTREADY);
		add_action(action);
		return;
	}
	else						// We are not in animate editing mode
	{
		if(value_desc.is_value_node())
		{
			if(ValueNode::Handle value_node = ValueNode_Const::Handle::cast_dynamic(value_desc.get_value_node()))
			{
				Action::Handle action(ValueNodeConstSet::create());
				action->set_param("canvas",get_canvas());
				action->set_param("canvas_interface",get_canvas_interface());
				action->set_param("value_node",value_node);
				action->set_param("new_value",value);
				if(!action->is_ready())
					throw Error(Error::TYPE_NOTREADY);
				add_action_front(action);
				return;
			}
			else
			if(ValueNode_Animated::Handle animated=ValueNode_Animated::Handle::cast_dynamic(value_desc.get_value_node()))
			{
				// If we are in not animate mode let's assume that the user wants to offset the
				// animated value node by the difference.
				// this is valid only for value types that allows it.
				Waypoint waypoint;
				Type &type=animated->get_type();
				Type &value_type=value.get_type();
				if(value_type==type &&
					(  type == type_integer
					|| type == type_angle
					|| type == type_time
					|| type == type_real
					|| type == type_vector)
					)
				{
					if (!lock_animation)
					{
						if ( !get_canvas_interface()
						  || !get_canvas_interface()->get_ui_interface()
						  || UIInterface::RESPONSE_OK != get_canvas_interface()->get_ui_interface()->confirmation(
								 _("You are trying to edit animated parameter while Animation Mode is off.\n\nDo you want to apply offset to this animation?" ),
								 _("Hint: You can hold Spacebar key while editing parameter to avoid this confirmation dialog."),
								 _("No"),
								 _("Yes"),
								 synfigapp::UIInterface::RESPONSE_OK ))
						{
							throw Error(Error::TYPE_UNABLE); // Issue  #693
						}
					}

					synfig::ValueNode_Animated::WaypointList::const_iterator iter;
					for(iter=animated->waypoint_list().begin(); iter<animated->waypoint_list().end(); iter++)
					{
						waypoint=*iter;
						ValueBase waypoint_value(waypoint.get_value());
						if (type == type_integer)
							waypoint_value=ValueBase(waypoint_value.get(int())+value.get(int())-(*animated)(time).get(int()));
						else
						if (type == type_angle)
							waypoint_value=ValueBase(waypoint_value.get(Angle())+value.get(Angle())-(*animated)(time).get(Angle()));
						else
						if (type == type_time)
							waypoint_value=ValueBase(waypoint_value.get(Time())+value.get(Time())-(*animated)(time).get(Time()));
						else
						if (type == type_real)
							waypoint_value=ValueBase(waypoint_value.get(Real())+value.get(Real())-(*animated)(time).get(Real()));
						else
						if (type == type_vector)
							waypoint_value=ValueBase(waypoint_value.get(Vector())+value.get(Vector())-(*animated)(time).get(Vector()));

						waypoint.set_value(waypoint_value);
						Action::Handle action(WaypointSet::create());
						action->set_param("canvas",get_canvas());
						action->set_param("canvas_interface",get_canvas_interface());
						action->set_param("value_node",ValueNode::Handle(animated));
						action->set_param("waypoint",waypoint);
						if(!action->is_ready())
							throw Error(Error::TYPE_NOTREADY);
						add_action(action);
					}
					return;
				}
				else
					throw Error(_("You must be in Animate-Editing-Mode to directly manipulate this value"));
			}
			else
				throw Error(_("Direct manipulation of this ValueNode type is not yet supported"));
		}
		else
		if(value_desc.parent_is_layer() && !value_desc.is_value_node())
		{
			Action::Handle layer_param_set(LayerParamSet::create());
			layer_param_set->set_param("canvas",get_canvas());
			layer_param_set->set_param("canvas_interface",get_canvas_interface());
			layer_param_set->set_param("layer",value_desc.get_layer());
			layer_param_set->set_param("param",value_desc.get_param_name());
			layer_param_set->set_param("new_value",value);
			if(!layer_param_set->is_ready())
				throw Error(Error::TYPE_NOTREADY);
			add_action_front(layer_param_set);
			return;
		}
		throw Error(_("Unsupported ValueDesc type"));
	}
	
}

void ValueDescSet::add_action_valuedescset(const ValueBase& value, const ValueDesc& value_desc, bool recursive)
{
	Action::Handle action(Action::create("ValueDescSet"));
	if(!action)
		throw Error(_("Unable to find action ValueDescSet (bug)"));
	action->set_param("canvas",get_canvas());
	action->set_param("canvas_interface",get_canvas_interface());
	action->set_param("time",time);
	action->set_param("new_value",value);
	action->set_param("value_desc",value_desc);
	if (!recursive)
		action->set_param("recursive", false);
	if(!action->is_ready())
		throw Error(Error::TYPE_NOTREADY);
	add_action(action);
}
