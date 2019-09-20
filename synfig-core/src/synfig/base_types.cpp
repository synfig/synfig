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

#include "general.h"
#include <synfig/localization.h>

#include "value.h"
#include "canvas.h"
#include "valuenodes/valuenode_bone.h"
#include "gradient.h"
#include "bone.h"
#include "matrix.h"
#include "boneweightpair.h"
#include "transformation.h"
#include "vector.h"
#include "time.h"
#include "segment.h"
#include "color.h"
#include "blinepoint.h"
#include "widthpoint.h"
#include "dashitem.h"

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

class TypeBool: public synfig::Type
{
	static String to_string(const bool &x) { return etl::strprintf("Bool (%s)", x ? "true" : "false"); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "bool";
		description.aliases.push_back(_("bool"));
		description.local_name = N_("bool");
		register_all<bool, to_string>();
	}
public:
	static TypeBool instance;
};
TypeBool TypeBool::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(bool, TypeBool)


// Integer

class TypeInteger: public Type
{
	static String to_string(const int &x) { return etl::strprintf("Integer (%d)", x); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "integer";
		description.aliases.push_back("int");
		description.aliases.push_back(_("integer"));
		description.local_name = N_("integer");
		register_all<int, to_string>();
	}
public:
	static TypeInteger instance;
};
TypeInteger TypeInteger::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(int, TypeInteger)


// Angle

class TypeAngle: public Type
{
	static String to_string(const Angle &x) { return etl::strprintf("Angle (%.2f)", Angle::deg(x).get()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "angle";
		description.aliases.push_back("degrees");
		description.aliases.push_back("radians");
		description.aliases.push_back("rotations");
		description.local_name = N_("angle");
		register_all<Angle, to_string>();
	}
public:
	static TypeAngle instance;
};
TypeAngle TypeAngle::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Angle, TypeAngle)


// Real

class TypeReal: public Type
{
public:
	class Inner
	{
	public:
		mutable float f;
		mutable Time t;
		Real r;
		Inner(): f(0.f), t(0.0), r(0.0) { }

		bool operator== (const Inner &other) const { return r == other.r; }
		Inner& operator= (const Inner &other) { return *this = other.r; }

		Inner& operator= (const Real &other) { r = other; return *this; }
		operator const Real&() const { return r; }

		Inner& operator= (const float &other) { r = other; return *this; }
		operator const float&() const { return f = r; }

		Inner& operator= (const Time &other) { r = other; return *this; }
		operator const Time&() const { return t = r; }
	};
private:
	static bool equal(ConstInternalPointer a, ConstInternalPointer b)
		{ return fabs((*(Inner*)a).r - (*(Inner*)b).r) <= 1e-14; }
	static bool less(ConstInternalPointer a, ConstInternalPointer b)
		{ return !equal(a, b) && (*(Inner*)a).r < (*(Inner*)b).r; }
	static String to_string(const Inner &x) { return etl::strprintf("Real (%f)", (Real)x); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "real";
		description.aliases.push_back("float");
		description.aliases.push_back(_("real"));
		description.local_name = N_("real");
		register_all_but_compare<Inner, Real, to_string>();
		register_alias<Inner, float>();
		register_alias<Inner, Time>();
		register_equal(equal);
		register_less(less);
	}
public:
	static TypeReal instance;
};
TypeReal TypeReal::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Real, TypeReal)
SYNFIG_IMPLEMENT_TYPE_ALIAS(float, TypeReal)


// Time

class TypeTime: public Type
{
	typedef TypeReal::Inner Inner;
	static String to_string(const Inner &x) { return etl::strprintf("Time (%s)", ((const Time&)x).get_string().c_str()); }
	static bool equal(ConstInternalPointer a, ConstInternalPointer b)
		{ return (const Time&)*(Inner*)a == (const Time&)*(Inner*)b; }
	static bool less(ConstInternalPointer a, ConstInternalPointer b)
		{ return !equal(a, b) && (const Time&)*(Inner*)a < (const Time&)*(Inner*)b; }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		TypeReal::instance.initialize();
		description.name = "time";
		description.aliases.push_back(_("time"));
		description.local_name = N_("time");
		register_all_but_compare<Inner, Time, to_string>();
		register_alias<Inner, float>();
		register_alias<Inner, Real>();
		register_equal(equal);
		register_less(less);
		register_copy(identifier, TypeReal::instance.identifier, Operation::DefaultFuncs::copy<Inner>);
		register_copy(TypeReal::instance.identifier, identifier, Operation::DefaultFuncs::copy<Inner>);
	}
public:
	static TypeTime instance;
};
TypeTime TypeTime::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Time, TypeTime)


// Vector

class TypeVector: public Type
{
	static String to_string(const Vector &x) { return etl::strprintf("Vector (%f, %f)", x[0], x[1]); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "vector";
		description.aliases.push_back("point");
		description.local_name = N_("vector");
		register_all<Vector, to_string>();
	}
public:
	static TypeVector instance;
};
TypeVector TypeVector::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Vector, TypeVector)


// Color

class TypeColor: public Type
{
	static String to_string(const Color &x) { return etl::strprintf("Color (%s)", x.get_string().c_str()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "color";
		description.local_name = N_("color");
		register_all<Color, to_string>();
	}
public:
	static TypeColor instance;
};
TypeColor TypeColor::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Color, TypeColor)


// Segment

class TypeSegment: public Type
{
	static String to_string(const Segment &x) { return etl::strprintf("Segment ((%f, %f) to (%f, %f))", x.p1[0], x.p1[1], x.p2[0], x.p2[1]); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "segment";
		description.local_name = N_("segment");
		register_all_but_compare<Segment, to_string>();
	}
public:
	static TypeSegment instance;
};
TypeSegment TypeSegment::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Segment, TypeSegment)


// BLinePoint

class TypeBLinePoint: public Type
{
	static String to_string(const BLinePoint &x) { return etl::strprintf("BLinePoint (%f, %f)", x.get_vertex()[0], x.get_vertex()[1]); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "bline_point";
		description.aliases.push_back("blinepoint");
		description.local_name = N_("spline_point");
		register_all_but_compare<BLinePoint, to_string>();
	}
public:
	static TypeBLinePoint instance;
};
TypeBLinePoint TypeBLinePoint::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(BLinePoint, TypeBLinePoint)


// Matrix

class TypeMatrix: public Type
{
	static String to_string(const Matrix &x) { return etl::strprintf("Matrix (%s)", x.get_string().c_str()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "matrix";
		description.local_name = N_("matrix");
		register_all_but_compare<Matrix, to_string>();
	}
public:
	static TypeMatrix instance;
};
TypeMatrix TypeMatrix::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Matrix, TypeMatrix)


// BoneWeightPair

class TypeBoneWeightPair: public Type
{
	static String to_string(const BoneWeightPair &x) { return etl::strprintf("Bone Weight Pair (%s)", x.get_string().c_str()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "bone_weight_pair";
		description.local_name = N_("bone_weight_pair");
		register_all_but_compare<BoneWeightPair, to_string>();
	}
public:
	static TypeBoneWeightPair instance;
};
TypeBoneWeightPair TypeBoneWeightPair::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(BoneWeightPair, TypeBoneWeightPair)


// WidthPoint

class TypeWidthPoint: public Type
{
	static String to_string(const WidthPoint &x) { return etl::strprintf("WidthPoint (%f, %f)", x.get_position(), x.get_width()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "width_point";
		description.aliases.push_back("widthpoint");
		description.local_name = N_("width_point");
		register_all_but_compare<WidthPoint, to_string>();
	}
public:
	static TypeWidthPoint instance;
};
TypeWidthPoint TypeWidthPoint::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(WidthPoint, TypeWidthPoint)


// DashItem

class TypeDashItem: public Type
{
	static String to_string(const DashItem &x) { return etl::strprintf("DashItem (%f, %f)", x.get_offset(), x.get_length()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "dash_item";
		description.aliases.push_back("dashitem");
		description.local_name = N_("dash_item");
		register_all<DashItem, to_string>();
	}
public:
	static TypeDashItem instance;
};
TypeDashItem TypeDashItem::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(DashItem, TypeDashItem)


// List

class TypeList: public Type
{
	static String to_string(const ValueBase::List &x) { return etl::strprintf("List (%zu elements)", x.size()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "list";
		description.local_name = N_("list");
		register_all<ValueBase::List, to_string>();
	}
public:
	static TypeList instance;
};
TypeList TypeList::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(ValueBase::List, TypeList)


// Canvas

class TypeCanvas: public Type
{
	class Inner
	{
	public:
		typedef Canvas *CanvasPtr;

#ifdef TRY_FIX_FOR_BUG_27
		bool fake_handle;
#endif
		etl::handle<Canvas> h;
		etl::loose_handle<Canvas> lh;
		mutable CanvasPtr p;
#ifdef TRY_FIX_FOR_BUG_27
		Inner(): fake_handle(false), p(NULL) { }
		~Inner() { if (fake_handle) h->ref(); }
#else
		Inner(): p(NULL) { }
#endif
		Inner& operator= (const etl::loose_handle<Canvas> &other)
		{
#ifdef TRY_FIX_FOR_BUG_27
			if (fake_handle) h->ref();
#endif
			lh = other;
			h = other;
#ifdef TRY_FIX_FOR_BUG_27
			fake_handle = h && !h->is_inline();
			if (fake_handle) h->unref_inactive();
#endif
			return *this;
		}
		Inner& operator= (const etl::handle<Canvas> &other)
			{ return *this = etl::loose_handle<Canvas>(other); }
		Inner& operator= (const CanvasPtr &other)
			{ return *this = etl::loose_handle<Canvas>(other); }
		Inner& operator= (const Inner &other)
			{ return *this = other.lh; }
		bool operator== (const Inner &other) const
			{ return lh == other.lh; }

		operator const etl::loose_handle<Canvas>&() const { return lh; }
		operator const etl::handle<Canvas>&() const { return h; }
		operator const CanvasPtr &() const { return p = &*lh; }
	};
	static String to_string(const Inner &x) { return etl::strprintf("Canvas (%s)", x.lh ? x.lh->get_id().c_str() : "NULL"); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "canvas";
		description.local_name = N_("canvas");
		register_all<Inner, etl::loose_handle<Canvas>, to_string>();
		register_alias< Inner, etl::handle<Canvas> >();
		register_alias<Inner, Canvas*>();
	}
public:
	static TypeCanvas instance;
};
TypeCanvas TypeCanvas::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(etl::loose_handle<Canvas>, TypeCanvas)
SYNFIG_IMPLEMENT_TYPE_ALIAS(etl::handle<Canvas>, TypeCanvas)
SYNFIG_IMPLEMENT_TYPE_ALIAS(Canvas*, TypeCanvas)


// String

class TypeString: public Type
{
	class Inner: public String
	{
		mutable const char *c;
	public:
		operator const char * const &() const { return c = c_str(); }
		String& operator=(const String &x) { std::string::operator=(x); return *this; }
		String& operator=(const char * const &x) { std::string::operator=(x); return *this; }
	};
	static String to_string(const Inner &x) { return etl::strprintf("String (%s)", x.c_str()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "string";
		description.local_name = N_("string");
		register_all<Inner, String, to_string>();
		register_alias<Inner, const char*>();
	}
public:
	static TypeString instance;
};
TypeString TypeString::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(String, TypeString)
SYNFIG_IMPLEMENT_TYPE_ALIAS(const char*, TypeString)


// Gradient

class TypeGradient: public Type
{
	static String to_string(const Gradient &x) { return etl::strprintf("Gradient (%zu cpoints)", x.size()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "gradient";
		description.local_name = N_("gradient");
		register_all_but_compare<Gradient, to_string>();
	}
public:
	static TypeGradient instance;
};
TypeGradient TypeGradient::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Gradient, TypeGradient)


// Bone

class TypeBoneObject: public Type
{
	static String to_string(const Bone &x) { return etl::strprintf("Bone (%s)", x.get_string().c_str()); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "bone_object";
		description.local_name = N_("bone_object");
		register_all_but_compare<Bone, to_string>();
	}
public:
	static TypeBoneObject instance;
};
TypeBoneObject TypeBoneObject::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Bone, TypeBoneObject)


// BoneValueNode

class TypeBoneValueNode: public Type
{
	class Inner
	{
	public:
		typedef ValueNode_Bone *ValueNode_BonePtr;

		etl::handle<ValueNode_Bone> h;
		mutable etl::loose_handle<ValueNode_Bone> lh;
		mutable ValueNode_BonePtr p;

		Inner(): p(NULL) { }
		Inner& operator= (const etl::handle<ValueNode_Bone> &other) { h = other; return *this; }
		Inner& operator= (const etl::loose_handle<ValueNode_Bone> &other) { h = other; return *this; }
		Inner& operator= (const ValueNode_BonePtr &other) { h = other; return *this; }
		Inner& operator= (const Inner &other) { return *this = other.h; }
		bool operator== (const Inner &other) const { return h == other.h; }

		operator const ValueNode_Bone::Handle&() const { return h; }
		operator const ValueNode_Bone::LooseHandle&() const { return lh = h; }
		operator const ValueNode_BonePtr &() const { return p = &*h; }
	};
	static String to_string(const Inner &x) { return etl::strprintf("ValueNodeBone (%s)", x.lh ? x.lh->get_string().c_str() : "NULL"); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "bone_valuenode";
		description.local_name = N_("bone_valuenode");
		register_all<Inner, etl::loose_handle<ValueNode_Bone>, to_string>();
		register_alias< Inner, etl::handle<ValueNode_Bone> >();
		register_alias<Inner, ValueNode_Bone*>();
	}
public:
	static TypeBoneValueNode instance;
};
TypeBoneValueNode TypeBoneValueNode::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(etl::handle<ValueNode_Bone>, TypeBoneValueNode)
SYNFIG_IMPLEMENT_TYPE_ALIAS(etl::loose_handle<ValueNode_Bone>, TypeBoneValueNode)
SYNFIG_IMPLEMENT_TYPE_ALIAS(ValueNode_Bone*, TypeBoneValueNode)


// Transformation

class TypeTransformation: public Type
{
	static String to_string(const Transformation &x) { return etl::strprintf("Transformation (%f, %f) (%f) (%f, %f)", x.offset[0], x.offset[1], Angle::deg(x.angle).get(), x.scale[0], x.scale[1]); }
	void initialize_vfunc(Description &description)
	{
		Type::initialize_vfunc(description);
		description.name = "transformation";
		description.local_name = N_("transformation");
		register_all<Transformation, to_string>();
	}
public:
	static TypeTransformation instance;
};
TypeTransformation TypeTransformation::instance;
SYNFIG_IMPLEMENT_TYPE_ALIAS(Transformation, TypeTransformation)

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
