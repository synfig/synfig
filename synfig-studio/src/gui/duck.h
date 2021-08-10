/* === S Y N F I G ========================================================= */
/*!	\file duck.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_DUCKMATIC_DUCK_H
#define __SYNFIG_DUCKMATIC_DUCK_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <set>
#include <list>

#include <ETL/smart_ptr>
#include <ETL/handle>

#include <sigc++/sigc++.h>

#include <synfig/vector.h>
#include <synfig/string.h>
#include <synfig/real.h>
#include <synfig/time.h>
#include <synfig/transform.h>

#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Duckmatic;

/*! \class Duck
**	\writeme */
class Duck
{
	friend class Duckmatic;

public:
	enum Type
	{
		TYPE_NONE					=	(0),		//    0
		TYPE_POSITION				=	(1 <<  0),	//    1
		TYPE_TANGENT				=	(1 <<  1),	//    2
		TYPE_RADIUS					=	(1 <<  2),	//    4
		TYPE_WIDTH					=	(1 <<  3),	//    8
		TYPE_ANGLE					=	(1 <<  4),	//   16
		TYPE_VERTEX					=	(1 <<  5),	//   32
		TYPE_BONE_RECURSIVE			=	(1 <<  6),	//   64
		TYPE_WIDTHPOINT_POSITION	=	(1 <<  7),	//  128
		TYPE_SCALE					=	(1 <<  8),	//  256
		TYPE_SCALE_X				=	(1 <<  9),	//  512
		TYPE_SCALE_Y				=	(1 << 10),	// 1024
		TYPE_SKEW					=	(1 << 11),	// 2048
		TYPE_FIRST_VERTEX			=	(1 << 12),	// 4096

		TYPE_ALL					=	(~0),

		TYPE_DEFAULT				=	0xdefadefa
	};
	//used for preference/interface handle tooltip flag
	enum Structure
	{
		STRUCT_NONE					=	(0),		//    0
		STRUCT_BLINEPOINT			=	(1 <<  0),	//    1
		STRUCT_TRANSFORMATION		=	(1 <<  1),	//    2
		STRUCT_WIDTHPOINT			=	(1 <<  2),	//    4
		STRUCT_BONE					=	(1 <<  3),	//    8
		STRUCT_GRADIENT				=	(1 <<  4),	//   16
		STRUCT_TEXT					=	(1 <<  5),	//   32
		STRUCT_RADIUS				=	(1 <<  6),	//   64
		STRUCT_TRANSFO_BY_VALUE		=	(1 <<  7),	//  128

		STRUCT_ALL					=	(~0),

		STRUCT_DEFAULT				=	0x44 //STRUCT_RADIUS+STRUCT_WIDTHPOINT
	};
	typedef std::shared_ptr<Duck> Handle;
	typedef std::shared_ptr<Duck> LooseHandle;

private:

	sigc::signal<bool,const Duck &> signal_edited_;
	sigc::signal<void> signal_user_click_[5];


	// information about represented value

	synfig::GUID guid_;
	synfig::String name;
	Type type_;
	synfigapp::ValueDesc value_desc_;
	synfigapp::ValueDesc alternative_value_desc_;


	// Flags

	bool editable_;
	bool alternative_editable_;
	bool edit_immediatelly_;
	bool radius_;
	bool tangent_;
	bool hover_;
	bool ignore_;
	bool exponential_;
	bool track_axes_;
	bool lock_aspect_;
	bool move_origin_;

	// positioning

	synfig::TransformStack transform_stack_;

	synfig::Real scalar_;

	synfig::Point origin_;
	Handle origin_duck_;

	Handle axis_x_angle_duck_;
	synfig::Angle axis_x_angle_;

	Handle axis_x_mag_duck_;
	synfig::Real axis_x_mag_;

	Handle axis_y_angle_duck_;
	synfig::Angle axis_y_angle_;

	Handle axis_y_mag_duck_;
	synfig::Real axis_y_mag_;

	Handle connect_duck_;
	Handle box_duck_;

	// value

	synfig::Point point_;
	etl::smart_ptr<synfig::Point> shared_point_;
	etl::smart_ptr<synfig::Angle> shared_angle_;
	etl::smart_ptr<synfig::Real> shared_mag_;
	synfig::Angle rotations_;
	synfig::Point aspect_point_;

	static int duck_count;
public:

	// constructors

	Duck();
	explicit Duck(const synfig::Point &point);
	Duck(const synfig::Point &point,const synfig::Point &origin);
	~Duck();


	// signals

	sigc::signal<bool,const Duck &> &signal_edited()
		{ return signal_edited_; }
	sigc::signal<void> &signal_user_click(int i=0)
		{ assert(i>=0); assert(i<5); return signal_user_click_[i]; }


	// information about represented value

	void set_guid(const synfig::GUID& x) { guid_=x; }
	const synfig::GUID& get_guid()const { return guid_; }
	synfig::GUID get_data_guid()const;

	//! Sets the name of the duck
	void set_name(const synfig::String &x);
	//! Retrieves the name of the duck
	synfig::String get_name()const { return name; }

	void set_type(Type x) { type_=x; }
	Type get_type()const { return type_; }

#ifdef _DEBUG
	//!	Returns a string containing the name of the given Type
	static synfig::String type_name(Type id);
	//!	Returns a string containing the name of the type
	synfig::String type_name()const { return type_name(get_type()); }
#endif	// _DEBUG

	void set_value_desc(const synfigapp::ValueDesc &x)
		{ value_desc_=x; }
	const synfigapp::ValueDesc& get_value_desc() const
		{ return value_desc_; }
	void set_alternative_value_desc(const synfigapp::ValueDesc &x)
		{ alternative_value_desc_=x; }
	const synfigapp::ValueDesc& get_alternative_value_desc() const
		{ return alternative_value_desc_; }


	// flags

	bool get_editable(bool is_alternative_mode)const
	{
		if (alternative_value_desc_.is_valid())
			return is_alternative_mode ? alternative_editable_ : editable_;
		return editable_;
	}
	//! Changes the editable flag.
	void set_editable(bool x)
		{ editable_=x; }
	//! Retrieves the status of the editable flag
	bool get_editable()const
		{ return editable_; }
	//! Changes the editable_alternative flag.
	void set_alternative_editable(bool x)
		{ alternative_editable_=x; }
	//! Retrieves the status of the editable_alternative flag
	bool get_alternative_editable()const
		{ return alternative_editable_; }

	bool is_radius()const
		{ return radius_; }
	void set_radius(bool r)
		{ radius_=r; }

	//! If set, the duck will send signal_edited while moving.
	//! If not set, the duck will send signal_edited when button released.
	void set_edit_immediatelly(bool x)
		{ edit_immediatelly_=x; }
	bool get_edit_immediatelly()const
		{ return edit_immediatelly_; }

	void set_tangent(bool x)
		{ tangent_=x; if (x) type_=TYPE_TANGENT; }
	bool get_tangent()const
		{ return tangent_; }

	//! Sets whether to show the duck as if it is being hovered over
	void set_hover(bool h)
		{ hover_=h; }
	//! Retrieves whether to show the duck as if it is being hovered over
	bool get_hover()const
		{ return hover_; }

	//! Sets whether to ignore the duck when checking for user interaction
	void set_ignore(bool i)
		{ ignore_=i; }
	//! Retrieves whether to ignore the duck when checking for user interaction
	bool get_ignore()const
		{ return ignore_; }

	//! Sets if the duck is using the exponential function
	/*!	Such representation allows to set the Real values in the range from \c -inf to \c inf . */
	void set_exponential(bool n)
		{ exponential_=n; }
	//! Retrieves the exponential value
	bool get_exponential()const
		{ return exponential_; }

	//! draw projection lines onto axes
	bool is_axes_tracks()const
		{ return track_axes_; }
	void set_track_axes(bool r)
		{ track_axes_=r; }

	bool is_aspect_locked()const
		{ return lock_aspect_; }
	void set_lock_aspect(bool r)
		{ if (!lock_aspect_ && r) aspect_point_=point_.norm(); lock_aspect_=r; }

	void set_move_origin(bool x)
		{ move_origin_=x; }
	//! Retrieves the exponential value
	bool get_move_origin()const
		{ return move_origin_; }

	// positioning

	void set_transform_stack(const synfig::TransformStack& x)
		{ transform_stack_=x; }
	const synfig::TransformStack& get_transform_stack()const
		{ return transform_stack_; }

	//! Sets the scalar multiplier for the duck with respect to the origin
	void set_scalar(synfig::Vector::value_type n)
		{ scalar_=n; }
	//! Retrieves the scalar value
	synfig::Vector::value_type get_scalar()const
		{ return scalar_; }

	//! Sets the origin point.
	void set_origin(const synfig::Point &x)
		{ origin_=x; origin_duck_=NULL; }
	//! Sets the origin point as another duck
	void set_origin(const Handle &x)
		{ origin_duck_=x; }
	//! Retrieves the origin location
	synfig::Point get_origin()const
		{ return origin_duck_?origin_duck_->get_point():origin_; }
	//! Retrieves the origin duck
	const Handle& get_origin_duck() const
		{ return origin_duck_; }

	void set_axis_x_angle(const synfig::Angle &a)
		{ axis_x_angle_=a; axis_x_angle_duck_=NULL; }
	void set_axis_x_angle(const Handle &duck, const synfig::Angle angle = synfig::Angle::zero())
		{ axis_x_angle_duck_=duck; axis_x_angle_=angle; }
	synfig::Angle get_axis_x_angle()const
		{ return axis_x_angle_duck_?get_sub_trans_point(axis_x_angle_duck_,false).angle()+axis_x_angle_:axis_x_angle_; }
	const Handle& get_axis_x_angle_duck()const
		{ return axis_x_angle_duck_; }

	void set_axis_x_mag(const synfig::Real &m)
		{ axis_x_mag_=m; axis_x_mag_duck_=NULL; }
	void set_axis_x_mag(const Handle &duck)
		{ axis_x_mag_duck_=duck; }
	synfig::Real get_axis_x_mag()const
		{ return axis_x_mag_duck_?get_sub_trans_point(axis_x_mag_duck_,false).mag():axis_x_mag_; }
	const Handle& get_axis_x_mag_duck()const
		{ return axis_x_mag_duck_; }

	synfig::Point get_axis_x()const
		{ return synfig::Point(get_axis_x_mag(), get_axis_x_angle()); }

	void set_axis_y_angle(const synfig::Angle &a)
		{ axis_y_angle_=a; axis_y_angle_duck_=NULL; }
	void set_axis_y_angle(const Handle &duck, const synfig::Angle angle = synfig::Angle::zero())
		{ axis_y_angle_duck_=duck; axis_y_angle_=angle; }
	synfig::Angle get_axis_y_angle()const
		{ return axis_y_angle_duck_?get_sub_trans_point(axis_y_angle_duck_,false).angle()+axis_y_angle_:axis_y_angle_; }
	const Handle& get_axis_y_angle_duck()const
		{ return axis_y_angle_duck_; }

	void set_axis_y_mag(const synfig::Real &m)
		{ axis_y_mag_=m; axis_y_mag_duck_=NULL; }
	void set_axis_y_mag(const Handle &duck)
		{ axis_y_mag_duck_=duck; }
	synfig::Real get_axis_y_mag()const
		{ return axis_y_mag_duck_?get_sub_trans_point(axis_y_mag_duck_,false).mag():axis_y_mag_; }
	const Handle& get_axis_y_mag_duck()const
		{ return axis_y_mag_duck_; }

	synfig::Point get_axis_y()const
		{ return synfig::Point(get_axis_y_mag(), get_axis_y_angle()); }

	//! linear ducks moves along specified axis only (angle locked)
	bool is_linear()const
		{ return !get_axis_y_mag_duck() && get_axis_y_mag() == 0; }
	void set_linear(bool r)
		{ if (is_linear() != r) set_axis_y_mag(r?0:1); }
	void set_linear(bool r, const synfig::Angle &a)
		{ set_linear(r); set_axis_x_angle(a); }
	void set_linear(bool r, const Handle &duck)
		{ set_linear(r); set_axis_x_angle(duck); }
	synfig::Angle get_linear_angle()const
		{ return get_axis_x_angle(); }
	const Handle& get_linear_duck()const
		{ return get_axis_x_angle_duck(); }


	// guidelines to other ducks

	//! draw line from specified duck to this duck
	void set_connect_duck(const Handle& x)
		{ connect_duck_=x; }
	const Handle& get_connect_duck()const
		{ return connect_duck_; }

	//! draw rectangle by two points - from this duck and from specified duck
	void set_box_duck(const Handle& x)
		{ box_duck_=x; }
	const Handle& get_box_duck()const
		{ return box_duck_; }


	// value

	//! Sets the location of the duck with respect to the origin
	void set_point(const synfig::Point &x);
	//! Returns the location of the duck
	synfig::Point get_point()const;

	void set_shared_point(const etl::smart_ptr<synfig::Point>&x)
		{ shared_point_=x; }
	const etl::smart_ptr<synfig::Point>& get_shared_point()const
		{ return shared_point_; }

	void set_shared_angle(const etl::smart_ptr<synfig::Angle>&x)
		{ shared_angle_=x; }
	const etl::smart_ptr<synfig::Angle>& get_shared_angle()const
		{ return shared_angle_; }

	void set_shared_mag(const etl::smart_ptr<synfig::Real>&x)
		{ shared_mag_=x; }
	const etl::smart_ptr<synfig::Real>& get_shared_mag()const
		{ return shared_mag_; }

	//! Returns the rotations of the duck
	//! For angle and tangent ducks, rotations are used instead of the location
	//! so that the duck can me rotated more than 180 degrees
	synfig::Angle get_rotations()const
		{ return rotations_; };
	//! Sets the rotations of the duck
	void set_rotations(const synfig::Angle &x)
		{ rotations_=x; };


	// calculation of position of duck at workarea

	synfig::Point get_trans_point()const;
	synfig::Point get_trans_point(const synfig::Point &x)const;

	void set_trans_point(const synfig::Point &x);
	void set_trans_point(const synfig::Point &x, const synfig::Time &time);

	synfig::Point get_sub_trans_point(const synfig::Point &x)const;
	synfig::Point get_sub_trans_point()const;
	synfig::Point get_sub_trans_point_without_offset(const synfig::Point &x)const;
	synfig::Point get_sub_trans_point_without_offset()const;
	void set_sub_trans_point(const synfig::Point &x);
	void set_sub_trans_point(const synfig::Point &x, const synfig::Time &time);
	synfig::Point get_sub_trans_point(const Handle &duck, const synfig::Point &def, bool translate = true)const;
	synfig::Point get_sub_trans_point(const Handle &duck, bool translate = true)const
		{ return get_sub_trans_point(duck, synfig::Point(0,0), translate); }
	synfig::Point get_sub_trans_origin()const;

	//! Retrieves the origin location
	synfig::Point get_trans_origin()const;


	// operators

	bool operator==(const Duck &rhs)const;
}; // END of class Duck

//! Combine Flags
inline Duck::Type
operator|(Duck::Type lhs, const Duck::Type rhs)
{ return static_cast<Duck::Type>(int(lhs)|int(rhs)); }

//! Exclude Flags
inline Duck::Type
operator-(Duck::Type lhs, const Duck::Type rhs)
{ return static_cast<Duck::Type>(int(lhs)&~int(rhs)); }

inline Duck::Type&
operator|=(Duck::Type& lhs, const Duck::Type rhs)
{ *reinterpret_cast<int*>(&lhs)|=int(rhs); return lhs; }

inline Duck::Type
operator&(const Duck::Type lhs, const Duck::Type rhs)
{ return static_cast<Duck::Type>(int(lhs)&int(rhs)); }

class DuckMap : public std::map<synfig::GUID,std::shared_ptr<studio::Duck> >
{
	typedef std::map<synfig::GUID,std::shared_ptr<studio::Duck> > PARENT_TYPE;
public:
	void insert(const Duck::Handle& x) { operator[](x->get_guid())=x;  }
}; // END of class DuckMap

typedef std::list<Duck::Handle> DuckList;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
