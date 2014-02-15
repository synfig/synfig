/* === S Y N F I G ========================================================= */
/*!	\file base_types.cpp
**	\brief Template Header
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

#include <cstdlib>
#include "base_types.h"

#include "value.h"
#include "general.h"
#include "canvas.h"
#include "valuenode_bone.h"
#include "gradient.h"
#include "bone.h"
#include "matrix.h"
#include "boneweightpair.h"
#include "transformation.h"
#include "vector.h"
#include "time.h"
#include "segment.h"
#include "color.h"

#endif

using namespace synfig;
using namespace types_namespace;
using namespace etl;

/* === M A C R O S ========================================================= */

#define TRY_FIX_FOR_BUG_27

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace synfig {
namespace types_namespace {


// Bool

class TypeBool: public TypeComparableGeneric<bool>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "bool";
		description.aliases.push_back(_("bool"));
		description.local_name = N_("bool");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Bool (%s)", x ? "true" : "false"); }
	static TypeBool instance;
};
TypeBool TypeBool::instance;
Type& get_type(const bool*) { return TypeBool::instance; }


// Integer

class TypeInteger: public TypeComparableGeneric<int>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "integer";
		description.aliases.push_back("int");
		description.aliases.push_back(_("integer"));
		description.local_name = N_("integer");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Integer (%d)", x); }
	static TypeInteger instance;
};
TypeInteger TypeInteger::instance;
Type& get_type(const int*) { return TypeInteger::instance; }


// Angle

class TypeAngle: public TypeComparableGeneric<Angle>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "angle";
		description.aliases.push_back("degrees");
		description.aliases.push_back("radians");
		description.aliases.push_back("rotations");
		description.local_name = N_("angle");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Angle (%.2f)", Angle::deg(x).get()); }
	static TypeAngle instance;
};
TypeAngle TypeAngle::instance;
Type& get_type(const Angle*) { return TypeAngle::instance; }


// Time

class TypeTime: public TypeComparableGeneric<Time>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "time";
		description.aliases.push_back(_("time"));
		description.local_name = N_("time");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Time (%s)", x.get_string().c_str()); }
	static TypeTime instance;
};
TypeTime TypeTime::instance;
Type& get_type(const Time*) { return TypeTime::instance; }


// Real

class TypeReal: public TypeGeneric<Real>
{
protected:
	static bool comparison(const void *a, const void *b) { return abs(*(Real*)a - *(Real*)b) <= 0.00000000000001; }

	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "real";
		description.aliases.push_back("float");
		description.aliases.push_back(_("real"));
		description.local_name = N_("real");
		register_comparison(&comparison);
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Real (%f)", x); }
	static TypeReal instance;
};
TypeReal TypeReal::instance;
Type& get_type(const Real*) { return TypeReal::instance; }


// Vector

class TypeVector: public TypeComparableGeneric<Vector>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "vector";
		description.aliases.push_back("point");
		description.local_name = N_("vector");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Vector (%f, %f)", x[0], x[1]); }
	static TypeVector instance;
};
TypeVector TypeVector::instance;
Type& get_type(const Vector*) { return TypeVector::instance; }


// Color

class TypeColor: public TypeComparableGeneric<Color>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "color";
		description.local_name = N_("color");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Color (%s)", x.get_string().c_str()); }
	static TypeColor instance;
};
TypeColor TypeColor::instance;
Type& get_type(const Color*) { return TypeColor::instance; }


// Segment

class TypeSegment: public TypeGeneric<Segment>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "segment";
		description.local_name = N_("segment");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Segment ((%f, %f) to (%f, %f))", x.p1[0], x.p1[1], x.p2[0], x.p2[1]); }
	static TypeSegment instance;
};
TypeSegment TypeSegment::instance;
Type& get_type(const Segment*) { return TypeSegment::instance; }


// BLinePoint

class TypeBLinePoint: public TypeGeneric<BLinePoint>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "bline_point";
		description.aliases.push_back("blinepoint");
		description.local_name = N_("spline_point");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("BLinePoint (%f, %f)", x.get_vertex()[0], x.get_vertex()[1]); }
	static TypeBLinePoint instance;
};
TypeBLinePoint TypeBLinePoint::instance;
Type& get_type(const BLinePoint*) { return TypeBLinePoint::instance; }


// Matrix

class TypeMatrix: public TypeGeneric<Matrix>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "matrix";
		description.local_name = N_("matrix");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Matrix (%s)", x.get_string().c_str()); }
	static TypeMatrix instance;
};
TypeMatrix TypeMatrix::instance;
Type& get_type(const Matrix*) { return TypeMatrix::instance; }


// BoneWeightPair

class TypeBoneWeightPair: public TypeGeneric<BoneWeightPair>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "bone_weight_pair";
		description.local_name = N_("bone_weight_pair");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Bone Weight Pair (%s)", x.get_string().c_str()); }
	static TypeBoneWeightPair instance;
};
TypeBoneWeightPair TypeBoneWeightPair::instance;
Type& get_type(const BoneWeightPair*) { return TypeBoneWeightPair::instance; }


// WidthPoint

class TypeWidthPoint: public TypeGeneric<WidthPoint>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "width_point";
		description.aliases.push_back("widthpoint");
		description.local_name = N_("width_point");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("WidthPoint (%f, %f)", x.get_position(), x.get_width()); }
	static TypeWidthPoint instance;
};
TypeWidthPoint TypeWidthPoint::instance;
Type& get_type(const WidthPoint*) { return TypeWidthPoint::instance; }


// DashItem

class TypeDashItem: public TypeComparableGeneric<DashItem>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "dash_item";
		description.aliases.push_back("dashitem");
		description.local_name = N_("dash_item");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("DashItem (%f, %f)", x.get_offset(), x.get_length()); }
	static TypeDashItem instance;
};
TypeDashItem TypeDashItem::instance;
Type& get_type(const DashItem*) { return TypeDashItem::instance; }


// List

class TypeList: public TypeComparableGeneric< ValueBase::List >
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "list";
		description.local_name = N_("list");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("List (%d elements)", x.size()); }
	static TypeList instance;
};
TypeList TypeList::instance;
Type& get_type(const ValueBase::List*) { return TypeList::instance; }


// Canvas

class TypeCanvas: public TypeComparableGeneric< etl::loose_handle<Canvas> >
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "canvas";
		description.local_name = N_("canvas");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Canvas (%s)", x->get_id().c_str()); }
	static TypeCanvas instance;

#ifdef TRY_FIX_FOR_BUG_27
	virtual void* create() {
		void *p0 = malloc(sizeof(etl::loose_handle<Canvas>) + sizeof(etl::handle<Canvas>));
		void *p1 = (char*)p0 + sizeof(etl::loose_handle<Canvas>);
		new (p0) etl::loose_handle<Canvas>();
		new (p1) etl::handle<Canvas>();
		return p0;
	}

	virtual void assign(void *dest, const void *src) {
		void *dest1 = (char*)dest + sizeof(etl::loose_handle<Canvas>);
		const etl::loose_handle<Canvas> &slh = *(etl::loose_handle<Canvas>*)src;
		etl::loose_handle<Canvas> &dlh = *(etl::loose_handle<Canvas>*)dest;
		etl::handle<Canvas> &dh = *(etl::handle<Canvas>*)dest1;
		dlh = slh;
		if (slh && slh->is_inline()) dh = slh; else dh = etl::handle<Canvas>();
	}

	virtual void destroy(const void *data) {
		void *data1 = (char*)data + sizeof(etl::loose_handle<Canvas>);
		const etl::loose_handle<Canvas> &lh = *(const etl::loose_handle<Canvas>*)data;
		const etl::handle<Canvas> &h = *(const etl::handle<Canvas>*)data1;
		lh.~loose_handle();
		h.~handle();
		free(*(void**)(void*)&data);
	}
#endif // TRY_FIX_FOR_BUG_27
};
TypeCanvas TypeCanvas::instance;
Type& get_type(const etl::loose_handle<Canvas>*) { return TypeCanvas::instance; }


// String

class TypeString: public TypeComparableGeneric<String>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "string";
		description.local_name = N_("string");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("String (%s)", x.c_str()); }
	static TypeString instance;
};
TypeString TypeString::instance;
Type& get_type(const String*) { return TypeString::instance; }


// Gradient

class TypeGradient: public TypeGeneric<Gradient>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "gradient";
		description.local_name = N_("gradient");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Gradient (%d cpoints)", x.size()); }
	static TypeGradient instance;
};
TypeGradient TypeGradient::instance;
Type& get_type(const Gradient*) { return TypeGradient::instance; }


// Bone

class TypeBoneObject: public TypeGeneric<Bone>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "bone_object";
		description.local_name = N_("bone_object");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Bone (%s)", x.get_string().c_str()); }
	static TypeBoneObject instance;
};
TypeBoneObject TypeBoneObject::instance;
Type& get_type(const Bone*) { return TypeBoneObject::instance; }


// BoneValueNode

class TypeBoneValueNode: public TypeComparableGeneric<ValueNode_Bone::Handle>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "bone_valuenode";
		description.local_name = N_("bone_valuenode");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("ValueNodeBone (%s)", x->get_string().c_str()); }
	static TypeBoneValueNode instance;
};
TypeBoneValueNode TypeBoneValueNode::instance;
Type& get_type(const ValueNode_Bone::Handle*) { return TypeBoneValueNode::instance; }


// Transformation

class TypeTransformation: public TypeComparableGeneric<Transformation>
{
protected:
	void initialize_vfunc(Description &description)
	{
		Parent::initialize_vfunc(description);
		description.name = "transformation";
		description.local_name = N_("transformation");
	}
public:
	String to_string(const Base &x) { return etl::strprintf("Transformation (%f, %f) (%f) (%f, %f)", x.offset[0], x.offset[1], Angle::deg(x.angle).get(), x.scale[0], x.scale[1]); }
	static TypeTransformation instance;
};
TypeTransformation TypeTransformation::instance;
Type& get_type(const Transformation*) { return TypeTransformation::instance; }

}} // END of namespaces types_namespace and synfig

namespace synfig {
	Type &type_bool				= TypeBool::instance;
	Type &type_integer			= TypeInteger::instance;
	Type &type_angle			= TypeAngle::instance;
	Type &type_time				= TypeTime::instance;
	Type &type_real				= TypeReal::instance;
	Type &type_vector			= TypeVector::instance;
	Type &type_color			= TypeColor::instance;
	Type &type_segment			= TypeSegment::instance;
	Type &type_bline_point		= TypeBLinePoint::instance;
	Type &type_matrix			= TypeMatrix::instance;
	Type &type_bone_weight_pair	= TypeBoneWeightPair::instance;
	Type &type_width_point		= TypeWidthPoint::instance;
	Type &type_dash_item		= TypeDashItem::instance;
	Type &type_list				= TypeList::instance;
	Type &type_canvas			= TypeCanvas::instance;
	Type &type_string			= TypeString::instance;
	Type &type_gradient			= TypeGradient::instance;
	Type &type_bone_object		= TypeBoneObject::instance;
	Type &type_bone_valuenode	= TypeBoneValueNode::instance;
	Type &type_transformation	= TypeTransformation::instance;
}; // END of namespace synfig
