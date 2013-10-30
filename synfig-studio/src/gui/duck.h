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

#include <list>

#include <ETL/smart_ptr>
#include <ETL/handle>

#include <synfig/vector.h>
#include <synfig/string.h>
#include <synfig/real.h>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include <synfig/time.h>
#include <ETL/smart_ptr>
#include <synfigapp/value_desc.h>
#include <synfig/transform.h>

/* === M A C R O S ========================================================= */

#ifdef HASH_MAP_H
#include HASH_MAP_H
#include FUNCTIONAL_H

#ifndef __STRING_HASH__
#define __STRING_HASH__
class StringHash
{
# ifdef FUNCTIONAL_HASH_ON_STRING
	HASH_MAP_NAMESPACE::hash<synfig::String> hasher_;
# else  // FUNCTIONAL_HASH_ON_STRING
	HASH_MAP_NAMESPACE::hash<const char*> hasher_;
# endif  // FUNCTIONAL_HASH_ON_STRING
public:
	size_t operator()(const synfig::String& x)const
	{
# ifdef FUNCTIONAL_HASH_ON_STRING
		return hasher_(x);
# else  // FUNCTIONAL_HASH_ON_STRING
		return hasher_(x.c_str());
# endif  // FUNCTIONAL_HASH_ON_STRING
	}
};
#endif
#else
#include <map>
#endif

#include <set>

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Duckmatic;

/*! \class Duck
**	\writeme */
class Duck : public etl::shared_object
{
	friend class Duckmatic;

public:
	enum Type
	{
		TYPE_NONE		=	(0),	//  0
		TYPE_POSITION	=	(1<<0),	//  1
		TYPE_TANGENT	=	(1<<1),	//  2
		TYPE_RADIUS		=	(1<<2),	//  4
		TYPE_WIDTH		=	(1<<3),	//  8
		TYPE_ANGLE		=	(1<<4),	// 16
		TYPE_VERTEX		=	(1<<5),	// 32
		TYPE_BONE_SETUP	=	(1<<6),	// 64
		TYPE_BONE_RECURSIVE	=	(1<<7),	// 128
		TYPE_WIDTHPOINT_POSITION	= (1<<8), //256

		TYPE_ALL		=	(~0),

		TYPE_DEFAULT	=	0xdefadefa
	};

	typedef etl::handle<Duck> Handle;
	typedef etl::loose_handle<Duck> LooseHandle;

private:

	sigc::signal<bool,const Duck &> signal_edited_;
	sigc::signal<void> signal_user_click_[5];

	Type type_;

	synfig::Point point;
	synfig::Angle rotations;

	etl::smart_ptr<synfig::Point> shared_point;

	synfig::Point origin;
	synfig::String name;
	synfig::Real scalar;

	etl::handle<Duck> origin_duck;

	etl::handle<Duck> connect_duck;
	etl::handle<Duck> box_duck;

	synfig::GUID guid_;

	// Flags
	bool editable;
	bool edit_immediatelly_;
	bool radius_;
	bool linear_;
	bool tangent_;
	bool hover_;
	bool ignore_;
	bool exponential_;

	synfig::Angle linear_angle_;

	synfig::TransformStack transform_stack_;

	synfigapp::ValueDesc value_desc_;

	static int duck_count;
public:
	Duck();
	Duck(const synfig::Point &point);
	Duck(const synfig::Point &point,const synfig::Point &origin);
	~Duck();

	sigc::signal<bool,const Duck &> &signal_edited() { return signal_edited_; }
	sigc::signal<void> &signal_user_click(int i=0) { assert(i>=0); assert(i<5); return signal_user_click_[i]; }

	void set_guid(const synfig::GUID& x) { guid_=x; }
	const synfig::GUID& get_guid()const { return guid_; }

	synfig::GUID get_data_guid()const;

	//! Changes the editable flag. If set, the duck will not be able to be moved.
	void set_editable(bool x) { editable=x; }

	//! Retrieves the status of the editable flag
	bool get_editable()const { return editable; }

	//! If set, the duck will send signal_edited while moving.
	//! If not set, the duck will send signal_edited when button released.
	void set_edit_immediatelly(bool x) { edit_immediatelly_=x; }

	bool get_edit_immediatelly()const { return edit_immediatelly_; }

	//! \writeme
	void set_tangent(bool x) { tangent_=x; type_=TYPE_TANGENT; }

	//! \writeme
	bool get_tangent()const { return tangent_; }

	//! Sets whether to show the duck as if it is being hovered over
	void set_hover(bool h) { hover_=h; }

	//! Retrieves whether to show the duck as if it is being hovered over
	bool get_hover()const { return hover_; }

	//! Sets whether to ignore the duck when checking for user interaction
	void set_ignore(bool i) { ignore_=i; }

	//! Retrieves whether to ignore the duck when checking for user interaction
	bool get_ignore()const { return ignore_; }
	
	//! Sets if the duck is using the exponential function
	/*!	Such representation allows to set the Real values in the range from \c -inf to \c inf . */
	void set_exponential(bool n) { exponential_=n; }
	
	//! Retrieves the exponential value
	bool get_exponential()const { return exponential_; }

	void set_connect_duck(const etl::handle<Duck>& x) { connect_duck=x; }
	void set_box_duck(const etl::handle<Duck>& x) { box_duck=x; }

	const etl::handle<Duck>& get_connect_duck()const { return connect_duck; }
	const etl::handle<Duck>& get_box_duck()const { return box_duck; }

	void set_value_desc(synfigapp::ValueDesc x) { value_desc_=x; }

	synfigapp::ValueDesc& get_value_desc() { return value_desc_; }

	void set_transform_stack(const synfig::TransformStack& x) { transform_stack_=x; }

	const synfig::TransformStack& get_transform_stack()const { return transform_stack_; }

	//! Sets the type of duck
	void set_type(Type x) { type_=x; }

	//! Gets the type of duck
	Type get_type()const { return type_; }

	//! Sets the scalar multiplier for the duck with respect to the origin
	void set_scalar(synfig::Vector::value_type n) { scalar=n; }

	//! Retrieves the scalar value
	synfig::Vector::value_type get_scalar()const { return scalar; }
	
	void set_shared_point(const etl::smart_ptr<synfig::Point>&x) { shared_point=x; }

	//! Sets the location of the duck with respect to the origin
	void set_point(const synfig::Point &x) { (shared_point?*shared_point:point)=x; }

	//! Returns the location of the duck
	synfig::Point get_point()const { return shared_point?*shared_point:point; }

	//! Returns the rotations of the duck
	/*! For angle and tangent ducks, rotations are used instead of the location
	 *  so that the duck can me rotated more than 180 degrees
	 */
	synfig::Angle get_rotations()const { return rotations; };

	//! Sets the rotations of the duck
	void set_rotations(const synfig::Angle &x) { rotations=x; };

	synfig::Point get_trans_point()const;

	void set_trans_point(const synfig::Point &x);
	void set_trans_point(const synfig::Point &x, const synfig::Time &time);

	synfig::Point get_sub_trans_point()const;
	void set_sub_trans_point(const synfig::Point &x);
	void set_sub_trans_point(const synfig::Point &x, const synfig::Time &time);
	synfig::Point get_sub_trans_origin()const;

	//! Sets the origin point.
	void set_origin(const synfig::Point &x);

	//! Sets the origin point as another duck
	void set_origin(const etl::handle<Duck> &x);

	//! Retrieves the origin location
	synfig::Point get_origin()const;

	//! Retrieves the origin duck
	const etl::handle<Duck> & get_origin_duck() const;

	//! Retrieves the origin location
	synfig::Point get_trans_origin()const;

	void set_radius(bool r) { radius_=r; }
	bool is_radius()const { return radius_; }

	bool is_linear()const { return linear_; }
	void set_linear(bool r, synfig::Angle a) { linear_=r; linear_angle_=a; }
	synfig::Angle get_linear_angle() { return linear_angle_; }

	//! Sets the name of the duck
	void set_name(const synfig::String &x);

	//! Retrieves the name of the duck
	synfig::String get_name()const { return name; }

	bool operator==(const Duck &rhs)const;

#ifdef _DEBUG
	//!	Returns a string containing the name of the given Type
	static synfig::String type_name(Type id);

	//!	Returns a string containing the name of the type
	synfig::String type_name()const { return type_name(get_type()); }
#endif	// _DEBUG

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

class DuckMap : public
#ifdef HASH_MAP_H
HASH_MAP_CLASS<synfig::GUID,etl::handle<studio::Duck>,synfig::GUIDHash>
{
	typedef HASH_MAP_CLASS<synfig::GUID,etl::handle<studio::Duck>,synfig::GUIDHash> PARENT_TYPE;
#else
std::map<synfig::GUID,etl::handle<studio::Duck> >
{
	typedef std::map<synfig::GUID,etl::handle<studio::Duck> > PARENT_TYPE;
#endif
public:
	void insert(const Duck::Handle& x) { operator[](x->get_guid())=x;  }
}; // END of class DuckMap

typedef std::list<Duck::Handle> DuckList;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
