/* === S I N F G =========================================================== */
/*!	\file duck.h
**	\brief Template Header
**
**	$Id: duck.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_DUCKMATIC_DUCK_H
#define __SINFG_DUCKMATIC_DUCK_H

/* === H E A D E R S ======================================================= */

#include <list>

#include <ETL/smart_ptr>
#include <ETL/handle>

#include <sinfg/vector.h>
#include <sinfg/string.h>
#include <sinfg/real.h>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include <sinfg/time.h>
#include <ETL/smart_ptr>
#include <sinfgapp/value_desc.h>
#include <sinfg/transform.h>

/* === M A C R O S ========================================================= */

#define HASH_MAP_H <ext/hash_map>
#define HASH_SET_H <ext/hash_set>

#ifdef HASH_MAP_H
#include HASH_MAP_H
#ifndef __STRING_HASH__
#define __STRING_HASH__
class StringHash
{
	__gnu_cxx::hash<const char*> hasher_;
public:
	size_t operator()(const sinfg::String& x)const
	{
		return hasher_(x.c_str());
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
		TYPE_NONE		=	(0),
		TYPE_POSITION	=	(1<<0),
		TYPE_TANGENT	=	(1<<1),
		TYPE_RADIUS		=	(1<<2),
		TYPE_WIDTH		=	(1<<3),
		TYPE_ANGLE		=	(1<<4),
		TYPE_VERTEX		=	(1<<5),

		TYPE_ALL		=	(~0),

		TYPE_DEFAULT	=	0xdefadefa
	};

	typedef etl::handle<Duck> Handle;
	typedef etl::loose_handle<Duck> LooseHandle;
	
private:

	sigc::signal<bool,const sinfg::Point &> signal_edited_;
	sigc::signal<void> signal_user_click_[5];
	
	Type type_;

	sinfg::Point point;

	etl::smart_ptr<sinfg::Point> shared_point;
	
	sinfg::Point origin;
	sinfg::String name;
	sinfg::Real scalar;

	etl::handle<Duck> origin_duck;

	etl::handle<Duck> connect_duck;
	etl::handle<Duck> box_duck;

	sinfg::GUID guid_;	

	// Flags
	bool editable;
	bool radius_;
	bool tangent_;
	
	sinfg::TransformStack transform_stack_;

	sinfgapp::ValueDesc value_desc_;

	static int duck_count;
public:
	Duck();
	Duck(const sinfg::Point &point);
	Duck(const sinfg::Point &point,const sinfg::Point &origin);
	~Duck();
		
	sigc::signal<bool,const sinfg::Point &> &signal_edited() { return signal_edited_; }
	sigc::signal<void> &signal_user_click(int i=0) { assert(i>=0); assert(i<5); return signal_user_click_[i]; }

	void set_guid(const sinfg::GUID& x) { guid_=x; }
	const sinfg::GUID& get_guid()const { return guid_; }

	sinfg::GUID get_data_guid()const;

	//! Changes the editable flag. If set, the duck will not be able to be moved.
	void set_editable(bool x) { editable=x; }

	//! Retrieves the status of the editable flag
	bool get_editable()const { return editable; }

	//! \writeme
	void set_tangent(bool x) { tangent_=x; type_=TYPE_TANGENT; }

	//! \writeme
	bool get_tangent()const { return tangent_; }

	void set_connect_duck(const etl::handle<Duck>& x) { connect_duck=x; }
	void set_box_duck(const etl::handle<Duck>& x) { box_duck=x; }

	const etl::handle<Duck>& get_connect_duck()const { return connect_duck; }
	const etl::handle<Duck>& get_box_duck()const { return box_duck; }

	void set_value_desc(sinfgapp::ValueDesc x) { value_desc_=x; }

	sinfgapp::ValueDesc& get_value_desc() { return value_desc_; }

	void set_transform_stack(const sinfg::TransformStack& x) { transform_stack_=x; }

	const sinfg::TransformStack& get_transform_stack()const { return transform_stack_; }
	
	//! \writeme
	void set_type(Type x) { type_=x; }

	//! \writeme
	Type get_type()const { return type_; }

	//! Sets the scalar multiplier for the duck with respect to the origin
	void set_scalar(sinfg::Vector::value_type n) { scalar=n; }

	//! Retrieves the scalar value
	sinfg::Vector::value_type get_scalar()const { return scalar; }

	void set_shared_point(const etl::smart_ptr<sinfg::Point>&x) { shared_point=x; }

	//! Sets the location of the duck with respect to the origin
	void set_point(const sinfg::Point &x) { (shared_point?*shared_point:point)=x; }
		
	//! Returns the location of the duck
	sinfg::Point get_point()const { return shared_point?*shared_point:point; }
	
	sinfg::Point get_trans_point()const;
	
	void set_trans_point(const sinfg::Point &x);

	sinfg::Point get_sub_trans_point()const;
	void set_sub_trans_point(const sinfg::Point &x);
	sinfg::Point get_sub_trans_origin()const;

	//! Sets the origin point.
	void set_origin(const sinfg::Point &x);

	//! Sets the origin point as another duck
	void set_origin(const etl::handle<Duck> &x);

	//! Retrieves the origin location
	sinfg::Point get_origin()const;
	
	//! Retrieves the origin duck
	const etl::handle<Duck> & get_origin_duck() const;	

	//! Retrieves the origin location
	sinfg::Point get_trans_origin()const;

	void set_radius(bool r) { radius_=r; }
	bool is_radius()const { return radius_; }
	
	//! Sets the name of the duck
	void set_name(const sinfg::String &x);

	//! Retrieves the name of the duck
	sinfg::String get_name()const { return name; }
	
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

class DuckMap : public
#ifdef HASH_MAP_H
__gnu_cxx::hash_map<sinfg::GUID,etl::handle<studio::Duck>,sinfg::GUIDHash>
{
	typedef __gnu_cxx::hash_map<sinfg::GUID,etl::handle<studio::Duck>,sinfg::GUIDHash> PARENT_TYPE;
#else
std::map<sinfg::GUID,etl::handle<studio::Duck> >
{
	typedef std::map<sinfg::GUID,etl::handle<studio::Duck> > PARENT_TYPE;
#endif
public:
	void insert(const Duck::Handle& x) { operator[](x->get_guid())=x;  }
}; // END of class DuckMap

typedef std::list<Duck::Handle> DuckList;

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
