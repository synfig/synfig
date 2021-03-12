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

#include "base_types.h"

#include <vector>
#include <list>
#include "interpolation.h"

#include <ETL/ref_count>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

// TODO: remove following predeclarations
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
	typedef std::vector<ValueBase> List;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

protected:
	//! The type of value
	Type *type;
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
		type(&type_nil),data(0),ref_count(0),loop_(loop_), static_(static_),
		interpolation_(INTERPOLATION_UNDEFINED)
	{
#ifdef INITIALIZE_TYPE_BEFORE_USE
		type->initialize();
#endif
		set(x);
	}

	template <typename T>
	ValueBase(const std::vector<T> &x, bool loop_=false, bool static_=false):
		type(&type_nil),data(0),ref_count(0),loop_(loop_), static_(static_),
		interpolation_(INTERPOLATION_UNDEFINED)
	{
#ifdef INITIALIZE_TYPE_BEFORE_USE
		type->initialize();
#endif
		set_list_of(x);
	}

	//! Copy constructor. The data is not copied, just the type.
	ValueBase(Type &x);

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

	//! Equal than operator. Segment, Gradient and Bline Points cannot be compared.
	bool operator==(const ValueBase& rhs)const;

	//! Not equal than operator.
	bool operator!=(const ValueBase& rhs)const { return !operator==(rhs); }

	//! todo writeme
	bool operator<(const ValueBase& rhs)const;

	bool operator>(const ValueBase& rhs)const
		{ return rhs < *this; }
	bool operator<=(const ValueBase& rhs)const
		{ return !(*this < rhs); }
	bool operator>=(const ValueBase& rhs)const
		{ return !(rhs < *this); }

	//!	Constant index operator for when value is of type TYPE_LIST
	const ValueBase &operator[](int index)const
		{ assert(type==&type_list); assert(index>0); return get_list()[index]; }

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
	Type& get_contained_type()const;

	//! Returns true if the contained value is defined and valid.
	bool is_valid()const;

	//!	Returns a string containing the name of the type. Used for sif files
	String type_name()const { return type->description.name; }

	//! Returns the type of the contained value
	Type& get_type()const { return *type; }

	template<typename T>
	inline static bool can_get(const TypeId type, const T &x)
		{ return _can_get(type, types_namespace::get_type_alias(x)); }
	template<typename T>
	inline static bool can_get(const Type& type, const T &x) { return can_get(type.identifier, x); }
	template<typename T>
	inline static bool can_set(const TypeId type, const T &x)
		{ return _can_set(type, types_namespace::get_type_alias(x)); }
	template<typename T>
	inline static bool can_set(const Type& type, const T &x) { return can_set(type.identifier, x); }
	template<typename T>
	inline static bool can_put(const TypeId type, const T &x)
		{ return _can_put(type, types_namespace::get_type_alias(x)); }
	template<typename T>
	inline static bool can_put(const Type& type, const T &x) { return can_put(type.identifier, x); }
	inline static bool can_copy(const TypeId dest, const TypeId src)
		{ return NULL != Type::get_operation<Operation::CopyFunc>(Operation::Description::get_copy(dest, src)); }
	inline static bool can_copy(const Type& dest, const Type& src) { return can_copy(dest.identifier, src.identifier); }

	template<typename T> inline bool can_get(const T &x) const { return is_valid() && can_get(*type, x); }
	template<typename T> inline bool can_set(const T &x) const { return can_set(*type, x); }
	template<typename T> inline bool can_put(const T &x) const { return is_valid() && can_put(*type, x); }
	bool can_copy_from(const TypeId type) const { return can_copy(this->type->identifier, type); }
	bool can_copy_from(const Type& type) const { return can_copy(*this->type, type); }
	bool can_copy_to(const TypeId type) const { return can_copy(type, this->type->identifier); }
	bool can_copy_to(const Type& type) const { return can_copy(type, *this->type); }

	template<typename T> bool same_type_as(const T&x) const { return can_get(x) && can_set(x) && can_put(x); }

	// === GET MEMBERS ========================================================
	//! Template to get the ValueBase class data by casting the type
	template <typename T>
	inline const T &get(const T &x)const { return (const T&)_get(types_namespace::get_type_alias(x)); }

	//! Gets the data as List Type
	const List& get_list()const { return get(List()); }

	template<typename T>
	std::vector<T> get_list_of(const T &x)const
	{
		const List &list = get_list();
		std::vector<T> out_list;
		out_list.reserve(list.size());
		for(List::const_iterator i = list.begin(); i != list.end(); ++i)
			if (i->can_get(x))
				out_list.push_back(i->get(x));
		return out_list;
	}

	template<typename T>
	void set_list_of(const std::vector<T> &list)
	{
		*this = List(list.begin(), list.end());
	}

	String get_string() const;
	// ========================================================================

	//! Put template for any class
	template <typename T>
	inline void put(T* x)const { _put(types_namespace::get_type_alias(*x), x); }

	//! Set template for any class
	template <typename T>
	inline void set(const T& x) { _set(x); }


	/*
 --	** -- S T A T I C   F U N C T I O N S -------------------------------------
	*/

public:
	//!	Returns a the corresponding Type of the described type.
	//! Notice that this is used in the loadcanvas. It should keep all
	//! all type names used in previous sif files
	static Type& ident_type(const String &str);


	// === GET TYPE MEMBERS ===================================================
	template<typename T>
	static Type& get_type(const T&) { return Type::get_type<T>(); }

	// TODO: remove this, when removed all references in code
	static Type& get_type(const List &)
		{ return Type::get_type<List>(); }
	static Type& get_type(Canvas* const &)
		{ return Type::get_type<Canvas*>(); }
	static Type& get_type(ValueNode_Bone* const &)
		{ return Type::get_type<ValueNode_Bone*>(); }
	template <typename T> static Type& get_type(const T* &)
		{ int i[(int)1 - (int)sizeof(T)]; return type_nil; }
	template <typename T> static Type& get_type(const std::vector<T> &)
		{ int i[(int)1 - (int)sizeof(T)]; return type_nil; }
	template <typename T> static Type& get_type(const std::list<T> &)
		{ int i[(int)1 - (int)sizeof(T)]; return type_nil; }
	// ========================================================================


	/*
 --	** -- C A S T   O P E R A T O R S -----------------------------------------
	*/

public:
	//! I wonder why are those casting operators disabled...
	/*
	template<typename T>
	operator const T&() const
	{
		Type &t = Type::get_type<T>();
		Operation::GenericFuncs<T>::GetFunc func =
			Type::get_operation<Operation::GenericFuncs<T>::GetFunc>(
				Operation::Description::get_get(t.identifier) );
		assert(func != NULL);
		return func(data);
	}
	*/

	/*
 --	** -- O T H E R -----------------------------------------------------------
	*/

private:
	void create(Type &type);
	inline void create() { create(*type); }

	template <typename T>
	inline static bool _can_get(const TypeId type, const T &)
	{
		typedef typename T::AliasedType TT;
		return NULL !=
			Type::get_operation<typename Operation::GenericFuncs<TT>::GetFunc>(
				Operation::Description::get_get(type) );
	}

	template <typename T>
	inline static bool _can_put(const TypeId type, const T &)
	{
		typedef typename T::AliasedType TT;
		return NULL !=
			Type::get_operation<typename Operation::GenericFuncs<TT>::PutFunc>(
				Operation::Description::get_put(type) );
	}

	template <typename T>
	inline static bool _can_set(const TypeId type, const T &)
	{
		typedef typename T::AliasedType TT;
		return NULL !=
			Type::get_operation<typename Operation::GenericFuncs<TT>::SetFunc>(
				Operation::Description::get_set(type) );
	}

	template <typename T>
	const typename T::AliasedType& _get(const T &)const
	{
		typedef typename T::AliasedType TT;
#ifdef _DEBUG
		if (!is_valid())
			printf("%s:%d !is_valid()\n", __FILE__, __LINE__);
#endif
		assert(is_valid());
		typename Operation::GenericFuncs<TT>::GetFunc func =
			Type::get_operation<typename Operation::GenericFuncs<TT>::GetFunc>(
				Operation::Description::get_get(type->identifier) );
#ifdef _DEBUG
		if (func == NULL)
			printf("%s:%d %s get_func == NULL\n", __FILE__, __LINE__, type->description.name.c_str());
#endif
		assert(func !=  NULL);
		return func(data);
	}

	template <typename T>
	void _put(const T &, typename T::AliasedType *x)const
	{
		assert(is_valid());
		typedef typename T::AliasedType TT;
		typename Operation::GenericFuncs<TT>::PutFunc func =
			Type::get_operation<typename Operation::GenericFuncs<TT>::PutFunc>(
				Operation::Description::get_put(type->identifier) );
		assert(func !=  NULL);
		func(*x, data);
	}

	template<typename T>
	void __set(const T &alias, const typename T::AliasedType &x)
	{
		typedef typename T::AliasedType TT;
#ifdef INITIALIZE_TYPE_BEFORE_USE
		alias.type.initialize();
#endif

		Type &current_type = *type;
		if (current_type != type_nil)
		{
			typename Operation::GenericFuncs<TT>::SetFunc func =
				Type::get_operation<typename Operation::GenericFuncs<TT>::SetFunc>(
					Operation::Description::get_set(current_type.identifier) );
			if (func != NULL)
			{
				if (!ref_count.unique()) create(current_type);
				func(data, x);
				return;
			}
		}

		Type &new_type = alias.type;
		assert(new_type != current_type);
		assert(new_type != type_nil);

		typename Operation::GenericFuncs<TT>::SetFunc func =
			Type::get_operation<typename Operation::GenericFuncs<TT>::SetFunc>(
				Operation::Description::get_set(new_type.identifier) );
		assert(func != NULL);

		create(new_type);
		assert(*type != type_nil);
		func(data, x);
	}

	//! Internal set template. Takes in consideration the reference counter
	template <typename T>
	inline void _set(const T& x) { __set(types_namespace::get_type_alias(x), x); }

}; // END of class ValueBase


/*!	\class Value
**	\brief Template for all the valid Value Types
*/
template <class T>
class Value : public ValueBase
{
public:
	Value(const T &x):ValueBase(x) { }
	Value(const ValueBase &x):ValueBase(x) { }
	T get()const { return ValueBase::get(T()); }
	void put(T* x)const	{ ValueBase::put(x); }
	void set(const T& x) { ValueBase::operator=(x); }
	Value<T>& operator=(const T& x) { set(x); return *this; }
	Value<T>& operator=(const Value<T>& x) { return ValueBase::operator=(x); }
	Value<T>& operator=(const ValueBase& x) { return ValueBase::operator=(x); }

}; // END of class Value

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
