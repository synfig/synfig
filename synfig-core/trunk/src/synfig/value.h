/* === S Y N F I G ========================================================= */
/*!	\file value.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUE_H
#define __SYNFIG_VALUE_H

/* === H E A D E R S ======================================================= */

#include "segment.h"
#include "string.h"
#include <list>
#include <vector>
#include <ETL/trivial>
#include <ETL/handle>
#include "general.h"
#include "blinepoint.h"
#include "bone.h"
#include "exception.h"

#ifdef USE_HALF_TYPE
#include <OpenEXR/half.h>
#endif

#ifndef SYNFIG_NO_ANGLE
#include "angle.h"
#endif

#include <ETL/ref_count>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Canvas;
class Vector;
class Time;
class Segment;
class Gradient;
class BLinePoint;
class Color;
class Bone;
class ValueNode_Bone;
class Matrix;
class BoneWeightPair;

/*!	\class ValueBase
**	\todo writeme
*/
class ValueBase
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	//! \writeme
	enum Type
	{
		TYPE_NIL=0,			//!< Represents an empty value

		TYPE_BOOL,
		TYPE_INTEGER,
		TYPE_ANGLE,			//!< Angle

		// All types after this point are larger than 32 bits

		TYPE_TIME,			//!< Time
		TYPE_REAL,			//!< Real

		// All types after this point are larger than 64 bits

		TYPE_VECTOR,		//!< Vector
		TYPE_COLOR,			//!< Color
		TYPE_SEGMENT,		//!< Segment
		TYPE_BLINEPOINT,	//!< BLinePoint
		TYPE_MATRIX,		//!< Matrix
		TYPE_BONE_WEIGHT_PAIR,	//!< pair<Bone,Real>

		// All types after this point require construction/destruction

		TYPE_LIST,			//!< List
		TYPE_CANVAS,		//!< Canvas
		TYPE_STRING,		//!< String
		TYPE_GRADIENT,		//!< Color Gradient
		TYPE_BONE,			//!< Bone
		TYPE_VALUENODE_BONE,//!< ValueNode_Bone

		TYPE_END			//!< Not a valid type, used for sanity checks
	};

private:

	typedef std::vector<ValueBase> list_type;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

protected:

	Type type;
	void *data;
	etl::reference_counter ref_count;
	bool loop_;

	/*
 --	** -- C O N S T R U C T O R S -----------------------------------
	*/

public:

	//! \writeme
	ValueBase();

	//! \writeme
	template <typename T>
	ValueBase(const T &x, bool loop_=false):
		type(TYPE_NIL),data(0),ref_count(0),loop_(loop_)
		{ set(x); }

	//! \writeme
	ValueBase(Type x);

	//! \writeme
	~ValueBase();

	/*
 --	** -- O P E R A T O R S ---------------------------------------------------
	*/

public:

	//! \writeme
	template <class T> ValueBase& operator=(const T& x)
		{ set(x); return *this; }

	//! \writeme
	ValueBase& operator=(const ValueBase& x);

	//! \writeme
	bool operator==(const ValueBase& rhs)const;

	//! \writeme
	bool operator!=(const ValueBase& rhs)const { return !operator==(rhs); }

	//!	Constant index operator for when value is of type TYPE_LIST
	const ValueBase &operator[](int index)const
		{ assert(type==TYPE_LIST); assert(index>0); return get_list()[index]; }

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! \writeme
	void clear();

	//! \writeme
	bool get_loop()const { return loop_; }

	//! \writeme
	void set_loop(bool x) { loop_=x; }

	//! \writeme
	bool empty()const;

	//! \writeme
	Type get_contained_type()const;

	//! Returns true if the contained value is defined and valid.
	bool is_valid()const;

	//!	Returns a string containing the name of the type
	String type_name()const { return type_name(type); }

	//! Returns the type of the contained value
	const Type & get_type()const { return type; }

	//! Checks the type of the parameter against itself. Returns true if they are of the same type.
	template <class T> bool
	same_type_as(const T &x)const
	{
		const Type testtype(get_type(x));

		return same_type_as(type, testtype);
	}

	bool same_type_as(const Type testtype)const
	{
		return same_type_as(type, testtype);
	}

	//! Compares two types.  Returns true if they are the same type.
	static bool same_type_as(const Type type1, const Type type2)
	{
		if (type1 == type2) return true;
		if ((type1 == TYPE_REAL || type1 == TYPE_TIME) &&
			(type2 == TYPE_REAL || type2 == TYPE_TIME))
			return true;
		return false;
	}


	// === GET MEMBERS ========================================================
	template <typename T>
	const T &get(const T& x __attribute__ ((unused)))const
	{
#ifdef _DEBUG
		if (!is_valid())
			printf("%s:%d !is_valid()\n", __FILE__, __LINE__);
		else if (!same_type_as(x))
			printf("%s:%d !'%s'.same_type_as('%s')\n", __FILE__, __LINE__, type_name(type).c_str(), type_name(get_type(x)).c_str());
#endif
		assert(is_valid() && same_type_as(x));
		return *static_cast<const T*>(data);
	}
	float get(const float &)const { return get(Real()); }
	etl::loose_handle<Canvas> get(const etl::handle<Canvas>&)const
		{ return get(etl::loose_handle<Canvas>()); }
	etl::loose_handle<Canvas> get(Canvas*)const
		{ return get(etl::loose_handle<Canvas>()); }
	const char* get(const char*)const;
	const list_type& get_list()const { return get(list_type()); }

#ifdef _DEBUG
	String get_string() const;
#endif	// _DEBUG
	// ========================================================================



	// === PUT MEMBERS ========================================================
	template <typename T>
	void put(T* x)const
	{
		assert(same_type_as(*x));
		*x=*static_cast<const T*>(data);
	}
	void put(float* x)const { *x=get(Real()); }
	void put(char** x)const;
	// ========================================================================



	// === SET MEMBERS ========================================================
	template <typename T> void set(const T& x) { _set(x); }
	void set(const float &x) { _set(Real(x)); }
	void set(const list_type &x);
	void set(const char* x);
	void set(char* x);
	void set(Canvas*x);
	void set(etl::loose_handle<Canvas> x);
	void set(etl::handle<Canvas> x);
	template <class T> void set(const std::vector<T> &x)
		{ _set(list_type(x.begin(),x.end())); }
	template <class T> void set(const std::list<T> &x)
		{ _set(list_type(x.begin(),x.end())); }
	// ========================================================================


	/*
 --	** -- S T A T I C   F U N C T I O N S -------------------------------------
	*/

public:

	//!	Returns a string containing the name of the given Type
	static String type_name(Type id);

	//!	Returns a string containing the translated name of the given Type
	static String type_local_name(Type id);

	//!	Returns a the corresponding Type of the described type
	static Type ident_type(const String &str);


	// === GET TYPE MEMBERS ===================================================
	static Type get_type(bool) { return TYPE_BOOL; }
	static Type get_type(int) { return TYPE_INTEGER; }
	static Type get_type(const Time&) { return TYPE_TIME; }
	static Type get_type(const Real&) { return TYPE_REAL; }
	static Type get_type(const float&) { return TYPE_REAL; }
	static Type get_type(const Vector&) { return TYPE_VECTOR; }
	static Type get_type(const Color&) { return TYPE_COLOR; }
	static Type get_type(const Segment&) { return TYPE_SEGMENT; }
	static Type get_type(const BLinePoint&) { return TYPE_BLINEPOINT; }
	static Type get_type(const Matrix&) {return TYPE_MATRIX;}
	static Type get_type(const BoneWeightPair&) {return TYPE_BONE_WEIGHT_PAIR;}
	static Type get_type(const String&) { return TYPE_STRING; }
	static Type get_type(const Gradient&) { return TYPE_GRADIENT; }
	static Type get_type(const Bone&) { return TYPE_BONE; }
	static Type get_type(const etl::handle<ValueNode_Bone>&) { return TYPE_VALUENODE_BONE; }
	static Type get_type(const etl::loose_handle<ValueNode_Bone>&) { return TYPE_VALUENODE_BONE; }
	static Type get_type(Canvas*) { return TYPE_CANVAS; }
	static Type get_type(const etl::handle<Canvas>&)
		{ return TYPE_CANVAS; }
	static Type get_type(const etl::loose_handle<Canvas>&)
		{ return TYPE_CANVAS; }
	static Type get_type(const list_type&) { return TYPE_LIST; }
	template <class T> static Type get_type(const std::vector<T> &/*x*/)
		{ return TYPE_LIST; }
	template <class T> static Type get_type(const std::list<T> &/*x*/)
		{ return TYPE_LIST; }
	// ========================================================================


	/*
 --	** -- C A S T   O P E R A T O R S -----------------------------------------
	*/

public:

	operator const list_type&()const { return get_list(); }
	//operator const Color&()const { return get(Color()); }
	operator const Real&()const { return get(Real()); }
	//operator const Time&()const { return get(Time()); }

	operator const Vector&()const {  return get(Vector()); }
	operator const BLinePoint&()const {  return get(BLinePoint()); }
	operator const Matrix&()const { return get(Matrix()); }
	//operator const int&()const {  return get(int()); }
	//operator const String&()const {  return get(String()); }
	//operator const char *()const {  return get(String()).c_str(); }
	operator const Segment&()const { return get(Segment()); }
	operator const Bone&()const { return get(Bone()); }


	/*
 --	** -- O T H E R -----------------------------------------------------------
	*/

public:

#ifdef USE_HALF_TYPE
	half get(const half &)const { return get(Real()); }
	void put(half*x)const { *x=get(Real()); }
	void set(const half &x) { _set(Real(x)); }
	static Type get_type(const half&) { return TYPE_REAL; }
	operator half()const { return get(Real()); }
#endif

#ifndef SYNFIG_NO_ANGLE
	operator const Angle&()const { return get(Angle()); }
	static Type get_type(const Angle&) { return TYPE_ANGLE; }
#endif

	template <class T>
	operator std::list<T>()const
	{
		assert(type==TYPE_LIST);
		std::list<T> ret(get_list().begin(),get_list().end());
		return ret;
	}
	template <class T>
	operator std::vector<T>()const
	{
		assert(type==TYPE_LIST);
		std::vector<T> ret(get_list().begin(),get_list().end());
		return ret;
	}


private:

	template <typename T> void
	_set(const T& x)
	{
		const Type newtype(get_type(x));

		assert(newtype!=TYPE_NIL);

		if(newtype==type)
		{
			if(ref_count.unique())
			{
				*reinterpret_cast<T*>(data)=x;
				return;
			}
		}

		clear();

		type=newtype;
		ref_count.reset();

//		if (type == TYPE_BONE && &x == 0)
//			data = 0;
//		else
		data=new T(x);
	}
}; // END of class ValueBase


/*!	\class Value
**	\todo writeme
*/
template <class T>
class Value : public ValueBase
{
public:
	Value(const T &x):ValueBase(x)
	{
	}

	Value(const ValueBase &x):ValueBase(x)
	{
		if(!x.same_type_as(T()))
			throw Exception::BadType("Value<T>(ValueBase): Type Mismatch");
	}

	Value()
	{
	}

	T get()const { return ValueBase::get(T()); }

	void put(T* x)const	{ ValueBase::put(x); }

	void set(const T& x) { ValueBase::operator=(x); }

	Value<T>& operator=(const T& x) { set(x); return *this; }

	Value<T>& operator=(const Value<T>& x) { return ValueBase::operator=(x); }

	Value<T>& operator=(const ValueBase& x)
	{
		if(!x.same_type_as(T()))
			throw Exception::BadType("Value<T>(ValueBase): Type Mismatch");
		return ValueBase::operator=(x);
	}

}; // END of class Value

/*
template <>
class Value< std::list<CT> > : public ValueBase
{
public:
	Value(const T &x):ValueBase(x)
	{
	}
	Value(const ValueBase &x):ValueBase(x)
	{
		if(!x.same_type_as(T()))
			throw Exception::BadType("Value<T>(ValueBase): Type Mismatch");
	}
	Value()
	{
	}

	T get()const { return ValueBase::get(T()); }

	void put(T* x)const	{ ValueBase::put(x); }

	void set(const T& x) { ValueBase::operator=(x); }

	Value<T>& operator=(const T& x) { set(x); return *this; }

	Value<T>& operator=(const Value<T>& x) { return ValueBase::operator=(x); }

	Value<T>& operator=(const ValueBase& x)
	{
		if(!x.same_type_as(T()))
			throw Exception::BadType("Value<T>(ValueBase): Type Mismatch");
		return ValueBase::operator=(x);
	}

}; // END of class Value
*/

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
