/* === S Y N F I G ========================================================= */
/*!	\file value.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include "angle.h"
#include "segment.h"
#include "string.h"
#include <list>
#include <vector>
#include <ETL/trivial>
#include <ETL/handle>
#include "general.h"
#include "blinepoint.h"
#include "bone.h"
#include "widthpoint.h"
#include "dashitem.h"
#include "exception.h"
#include "interpolation.h"

#ifdef USE_HALF_TYPE
#include <OpenEXR/half.h>
#endif


#include <ETL/ref_count>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Canvas;
class Vector;
class Time;
struct Segment;
class Gradient;
class BLinePoint;
class WidthPoint;
class DashItem;
class Color;
class Bone;
class ValueNode_Bone;
class Matrix;
class BoneWeightPair;

/*!	\class ValueBase
**	\brief Base class for the Values of Synfig
*/
class ValueBase
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:

	//! This enum lists all the types of values
	enum Type
	{
		TYPE_NIL=0,			//!< Represents an empty value

		TYPE_BOOL,			//!< Boolean value (1 or 0)
		TYPE_INTEGER,		//!< Integer value -1, 0, 1, etc.
		TYPE_ANGLE,			//!< Angle value (Real number internally)

		// All types after this point are larger than 32 bits

		TYPE_TIME,			//!< Time value
		TYPE_REAL,			//!< Real value (floating point number)

		// All types after this point are larger than 64 bits

		TYPE_VECTOR,		//!< Vector value (Real, Real) Points are Vectors too
		TYPE_COLOR,			//!< Color (Real, Real, Real, Real)
		TYPE_SEGMENT,		//!< Segment Point and Vector
		TYPE_BLINEPOINT,	//!< BLinePoint Origin (Point) 2xTangents (Vector) Width (Real), Origin (Real) Split Tangent (Boolean)
		TYPE_MATRIX,		//!< Matrix
		TYPE_BONE_WEIGHT_PAIR,	//!< pair<Bone,Real>
		TYPE_WIDTHPOINT,	//!< WidthPoint Position (Real), Width (Real), 2xSide Type (int enum)
		TYPE_DASHITEM,		//!< DashItem Offset (Real distance), Length (Real distance), 2xSide Type (int enum)

		// All types after this point require construction/destruction

		TYPE_LIST,			//!< List of any of above
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
	//! The type of value
	Type type;
	//! Pointer to hold the data of the value
	void *data;
	//! Counter of Value Nodes that refers to this Value Base
	//! Value base can only be destructed if the ref_count is not greater than 0
	//!\see etl::reference_counter
	etl::reference_counter ref_count;
	//! For Values with loop option like TYPE_LIST
	bool loop_;
	//! For Values of Constant Value Nodes
	bool static_;
	//! Parameter interpolation
	Interpolation interpolation_;

	/*
 --	** -- C O N S T R U C T O R S -----------------------------------
	*/

public:

	//! Default constructor
	ValueBase();

	//! Template constructor for any type
	template <typename T>
	ValueBase(const T &x, bool loop_=false, bool static_=false):
		type(TYPE_NIL),data(0),ref_count(0),loop_(loop_), static_(static_),
		interpolation_(INTERPOLATION_UNDEFINED)
		{ set(x); }

	//! Copy constructor. The data is not copied, just the type.
	ValueBase(Type x);

	//! Default destructor
	~ValueBase();

	/*
 --	** -- O P E R A T O R S ---------------------------------------------------
	*/

public:

	//! Template for the operator assignation operator for non ValueBase classes
	//! \see set()
	template <class T> ValueBase& operator=(const T& x)
		{ set(x); return *this; }

	//!Operator asignation for ValueBase classes. Does a exact copy of \x
	ValueBase& operator=(const ValueBase& x);

	//! Eqaul than operator. Segment, Gradient and Bline Points cannot be compared.
	bool operator==(const ValueBase& rhs)const;

	//! Not equal than operator.
	bool operator!=(const ValueBase& rhs)const { return !operator==(rhs); }

	//!	Constant index operator for when value is of type TYPE_LIST
	const ValueBase &operator[](int index)const
		{ assert(type==TYPE_LIST); assert(index>0); return get_list()[index]; }

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! Deletes the data only if the ref count is zero
	void clear();

	//! Gets the loop option.
	bool get_loop()const { return loop_; }

	//! Sets the loop option.
	void set_loop(bool x) { loop_=x; }

	//! Gets the static option.
	bool get_static()const { return static_; }

	//! Sets the static option.
	void set_static(bool x) { static_=x; }

	//! Gets the interpolation.
	Interpolation get_interpolation()const { return interpolation_; }

	//! Sets the interpolation.
	void set_interpolation(Interpolation x) { interpolation_=x; }
	
	//! Create independent copy from existing ValueBase object
	void copy(const ValueBase& x);

	//! Copies properties (static, interpolation, etc) from other ValueBase object.
	void copy_properties_of(const ValueBase& x);

	//! True if the Value is not valid or is type LIST and is empty
	bool empty()const;

	//! Gets the contained type in the Value Base
	Type get_contained_type()const;

	//! Returns true if the contained value is defined and valid.
	bool is_valid()const;

	//!	Returns a string containing the name of the type. Used for sif files
	String type_name()const { return type_name(type); }

	//! Returns the type of the contained value
	const Type & get_type()const { return type; }

	//! Checks the type of the parameter against itself. Returns true if they are of the same type.
	//! Template for any class
	template <class T> bool
	same_type_as(const T &x)const
	{
		const Type testtype(get_type(x));

		return same_type_as(type, testtype);
	}
	//! Checks the type of the parameter against itself. Returns true if they are of the same type.
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
	//! Template to get the ValueBase class data by casting the type
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
	//! Gets the Real part of the data
	float get(const float &)const { return get(Real()); }
	//! Gets the Canvas Handle part of the data based on Canvas Handle type
	etl::loose_handle<Canvas> get(const etl::handle<Canvas>&)const
		{ return get(etl::loose_handle<Canvas>()); }
	//! Gets the Canvas Handle part of the data based on Canvas pointer type
	etl::loose_handle<Canvas> get(Canvas*)const
		{ return get(etl::loose_handle<Canvas>()); }
	//! Gets the data as char pointer based on char pointer
	const char* get(const char*)const;
	//! Gets the data as List Type
	const list_type& get_list()const { return get(list_type()); }

#ifdef _DEBUG
	String get_string() const;
#endif	// _DEBUG
	// ========================================================================



	// === PUT MEMBERS ========================================================
	//! Put template for any class
	template <typename T>
	void put(T* x)const
	{
		assert(same_type_as(*x));
		*x=*static_cast<const T*>(data);
	}
	//! Put for float values
	void put(float* x)const { *x=get(Real()); }
	//! Put for char values (Not defined??)
	void put(char** x)const;
	// ========================================================================



	// === SET MEMBERS ========================================================
	//! Set template for any class
	template <typename T> void set(const T& x) { _set(x); }
	//! Set for float
	void set(const float &x) { _set(Real(x)); }
	//! Set for List Type
	void set(const list_type &x);
	//! Set for char string
	void set(const char* x);
	//! Set for char string
	void set(char* x);
	//! Set for Canvas pointer
	void set(Canvas*x);
	//! Set for Canvas handle
	void set(etl::loose_handle<Canvas> x);
	//! Set for Canvas handle
	void set(etl::handle<Canvas> x);
	//! Set template for standar vector
	template <class T> void set(const std::vector<T> &x)
		{ _set(list_type(x.begin(),x.end())); }
	//! Set template for standar list
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

	//!	Returns a the corresponding Type of the described type.
	//! Notice that this is used in the loadcanvas. It should keep all
	//! all type names used in previous sif files
	static Type ident_type(const String &str);


	// === GET TYPE MEMBERS ===================================================
	static Type get_type(bool) { return TYPE_BOOL; }
	static Type get_type(int) { return TYPE_INTEGER; }
	static Type get_type(const Angle&) { return TYPE_ANGLE; }
	static Type get_type(const Time&) { return TYPE_TIME; }
	static Type get_type(const Real&) { return TYPE_REAL; }
	static Type get_type(const float&) { return TYPE_REAL; }
	static Type get_type(const Vector&) { return TYPE_VECTOR; }
	static Type get_type(const Color&) { return TYPE_COLOR; }
	static Type get_type(const Segment&) { return TYPE_SEGMENT; }
	static Type get_type(const BLinePoint&) { return TYPE_BLINEPOINT; }
	static Type get_type(const Matrix&) {return TYPE_MATRIX;}
	static Type get_type(const BoneWeightPair&) {return TYPE_BONE_WEIGHT_PAIR;}
	static Type get_type(const WidthPoint&) { return TYPE_WIDTHPOINT; }
	static Type get_type(const DashItem&) { return TYPE_DASHITEM; }
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
	//! I wonder why are those casting operators disabled...
	operator const list_type&()const { return get_list(); }
	operator const Angle&()const { return get(Angle()); }
	//operator const Color&()const { return get(Color()); }
	operator const Real&()const { return get(Real()); }
	//operator const Time&()const { return get(Time()); }

	operator const Vector&()const {  return get(Vector()); }
	operator const BLinePoint&()const {  return get(BLinePoint()); }
	operator const Matrix&()const { return get(Matrix()); }
	operator const WidthPoint&()const {  return get(WidthPoint()); }
	operator const DashItem&()const {  return get(DashItem()); }
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


	//! Cast operator template to obtain the standard list from the TYPE LIST
	template <class T>
	operator std::list<T>()const
	{
		assert(type==TYPE_LIST);
		std::list<T> ret(get_list().begin(),get_list().end());
		return ret;
	}
	//! Cast operator template to obtain the standard vector from the TYPE LIST
	template <class T>
	operator std::vector<T>()const
	{
		assert(type==TYPE_LIST);
		std::vector<T> ret(get_list().begin(),get_list().end());
		return ret;
	}


private:
	//! Internal set template. Takes in consideration the reference counter
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
**	\brief Template for all the valid Value Types
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

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
