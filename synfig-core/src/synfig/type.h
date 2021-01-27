/* === S Y N F I G ========================================================= */
/*!	\file type.h
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TYPE_H
#define __SYNFIG_TYPE_H

/* === H E A D E R S ======================================================= */

#include <cassert>
#include <vector>
#include <map>
#include <typeinfo>
#include "string.h"

/* === M A C R O S ========================================================= */

//#define INITIALIZE_TYPE_BEFORE_USE

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
	class Type;

	template<typename T>
	class TypeAlias {
	public:
		typedef T AliasedType;
		Type &type;
		TypeAlias(Type &type): type(type) { }
	};
}

#include "real.h"
#include "string.h"
#include "angle.h"
#include <ETL/handle>

namespace synfig {

class Time;
class Color;
struct Segment;
class BLinePoint;
class Matrix3;
class BoneWeightPair;
class WidthPoint;
class DashItem;
class ValueBase;
class Canvas;
class Vector;
class Gradient;
class Bone;
class ValueNode_Bone;
class Transformation;
template<typename T> class WeightedValue;

namespace types_namespace
{
#define SYNFIG_DECLARE_TYPE_ALIAS(T) \
	TypeAlias< T > get_type_alias(T const&);
#define SYNFIG_IMPLEMENT_TYPE_ALIAS(T, Class) \
	TypeAlias< T > get_type_alias(T const&) { return TypeAlias< T >(Class::instance); }

	SYNFIG_DECLARE_TYPE_ALIAS(bool)
	SYNFIG_DECLARE_TYPE_ALIAS(int)
	SYNFIG_DECLARE_TYPE_ALIAS(Angle)
	SYNFIG_DECLARE_TYPE_ALIAS(Time)
	SYNFIG_DECLARE_TYPE_ALIAS(Real)
	SYNFIG_DECLARE_TYPE_ALIAS(float)
	SYNFIG_DECLARE_TYPE_ALIAS(Vector)
	SYNFIG_DECLARE_TYPE_ALIAS(Color)
	SYNFIG_DECLARE_TYPE_ALIAS(Segment)
	SYNFIG_DECLARE_TYPE_ALIAS(BLinePoint)
	SYNFIG_DECLARE_TYPE_ALIAS(Matrix3)
	SYNFIG_DECLARE_TYPE_ALIAS(BoneWeightPair)
	SYNFIG_DECLARE_TYPE_ALIAS(WidthPoint)
	SYNFIG_DECLARE_TYPE_ALIAS(DashItem)
	SYNFIG_DECLARE_TYPE_ALIAS(std::vector<ValueBase>)
	SYNFIG_DECLARE_TYPE_ALIAS(etl::loose_handle<Canvas>)
	SYNFIG_DECLARE_TYPE_ALIAS(etl::handle<Canvas>)
	SYNFIG_DECLARE_TYPE_ALIAS(Canvas*)
	SYNFIG_DECLARE_TYPE_ALIAS(String)
	SYNFIG_DECLARE_TYPE_ALIAS(const char*)
	SYNFIG_DECLARE_TYPE_ALIAS(Gradient)
	SYNFIG_DECLARE_TYPE_ALIAS(Bone)
	SYNFIG_DECLARE_TYPE_ALIAS(etl::handle<ValueNode_Bone>)
	SYNFIG_DECLARE_TYPE_ALIAS(etl::loose_handle<ValueNode_Bone>)
	SYNFIG_DECLARE_TYPE_ALIAS(ValueNode_Bone*)
	SYNFIG_DECLARE_TYPE_ALIAS(Transformation)

	template<typename T>
	TypeAlias< WeightedValue<T> > get_type_alias(WeightedValue<T> const&);
	template<typename T1, typename T2>
	TypeAlias< std::pair<T1, T2> > get_type_alias(std::pair<T1, T2> const&);
} // namespace types_namespace
} // namespace synfig

namespace synfig {

extern Type &type_nil;

typedef unsigned int TypeId;

/*!	\class Operation
**	\brief Provides methods to create operation with Values
*/
class Operation
{
public:
	typedef void* InternalPointer;
	typedef const void* ConstInternalPointer;

	enum OperationType {
		TYPE_NONE,
		TYPE_CREATE,
		TYPE_DESTROY,
		TYPE_SET,
		TYPE_PUT,
		TYPE_GET,
		TYPE_COPY,
		TYPE_EQUAL,
		TYPE_LESS,
		TYPE_TO_STRING,
	};

	typedef InternalPointer	(*CreateFunc)	();
	typedef void			(*DestroyFunc)	(ConstInternalPointer);
	typedef void			(*CopyFunc)		(InternalPointer dest, ConstInternalPointer src);
	typedef bool			(*EqualFunc)	(ConstInternalPointer, ConstInternalPointer);
	typedef bool			(*LessFunc)		(ConstInternalPointer, ConstInternalPointer);
	typedef InternalPointer	(*BinaryFunc)	(ConstInternalPointer, ConstInternalPointer);
	typedef String			(*ToStringFunc)	(ConstInternalPointer);

	template<typename T>
	class GenericFuncs
	{
	public:
		typedef void 		(*SetFunc)		(InternalPointer dest, const T &src);
		typedef void 		(*PutFunc)		(T &dest, ConstInternalPointer src);
		typedef const T&	(*GetFunc)		(ConstInternalPointer);
	private:
		GenericFuncs() { }
	};

	class DefaultFuncs
	{
	public:
		template<typename Inner>
		static InternalPointer create()
			{ return new Inner(); }
		template<typename Inner>
		static void destroy(ConstInternalPointer x)
			{ return delete (Inner*)x; }
		template<typename Inner, typename Outer>
		static void set(InternalPointer dest, const Outer &src)
			{ *(Inner*)dest = src; }
		template<typename Inner, typename Outer>
		static void put(Outer &dest, ConstInternalPointer src)
			{ dest = static_cast<const Outer&>(*(Inner*)src); }
		template<typename Inner, typename Outer>
		static const Outer& get(ConstInternalPointer x)
			{ return static_cast<const Outer&>(*(Inner*)x); }
		template<typename Inner>
		static void copy(InternalPointer dest, ConstInternalPointer src)
			{ *(Inner*)dest = *(Inner*)src; }
		template<typename Inner>
		static bool equal(ConstInternalPointer a, ConstInternalPointer b)
			{ return *(Inner*)a == *(Inner*)b; }
		template<typename Inner>
		static bool less(ConstInternalPointer a, ConstInternalPointer b)
			{ return *(Inner*)a < *(Inner*)b; }
		template<typename Inner, String (*Func)(const Inner&)>
		static String to_string(ConstInternalPointer x)
			{ return Func(*(const Inner*)x); }
	private:
		DefaultFuncs() { }
	};

	struct Description
	{
		OperationType operation_type;
		TypeId return_type;
		TypeId type_a;
		TypeId type_b;

		Description(OperationType operation_type = TYPE_NONE, TypeId return_type = 0, TypeId type_a = 0, TypeId type_b = 0):
			operation_type(operation_type), return_type(return_type), type_a(type_a), type_b(type_b) { }

		bool operator < (const Description &other) const
		{
			return operation_type < other.operation_type ? true
				 : other.operation_type < operation_type ? false
				 : return_type < other.return_type ? true
				 : other.return_type < return_type ? false
				 : type_a < other.type_a ? true
				 : other.type_a < type_a ? false
				 : type_b < other.type_b;
		}

		bool operator > (const Description &other) const { return other < *this; }
		bool operator != (const Description &other) const { return *this < other || other < *this; }
		bool operator == (const Description &other) const { return !(*this != other); }

		inline static Description get_create(TypeId type)
			{ return Description(TYPE_CREATE, type); }
		inline static Description get_destroy(TypeId type)
			{ return Description(TYPE_DESTROY, 0, type); }
		inline static Description get_set(TypeId type)
			{ return Description(TYPE_SET, 0, type); }
		inline static Description get_put(TypeId type)
			{ return Description(TYPE_PUT, 0, 0, type); }
		inline static Description get_get(TypeId type)
			{ return Description(TYPE_GET, 0, type); }
		inline static Description get_copy(TypeId type_a, TypeId type_b)
			{ return Description(TYPE_COPY, 0, type_a, type_b); }
		inline static Description get_copy(TypeId type)
			{ return get_copy(type, type); }
		inline static Description get_equal(TypeId type_a, TypeId type_b)
			{ return Description(TYPE_EQUAL, 0, type_a, type_b); }
		inline static Description get_equal(TypeId type)
			{ return get_equal(type, type); }
		inline static Description get_less(TypeId type_a, TypeId type_b)
			{ return Description(TYPE_LESS, 0, type_a, type_b); }
		inline static Description get_less(TypeId type)
			{ return get_less(type, type); }
		inline static Description get_to_string(TypeId type)
			{ return Description(TYPE_TO_STRING, 0, type); }
		inline static Description get_binary(OperationType operation_type, TypeId return_type, TypeId type_a, TypeId type_b)
			{ return Description(operation_type, return_type, type_a, type_b); }
	};

private:
	Operation() { }
}; // END of class Operation


/*!	\class Type
**	\brief Class for the Type of Values of Synfig
*/
class Type
{
public:
	enum { NIL = 0 };
	typedef Operation::InternalPointer InternalPointer;
	typedef Operation::ConstInternalPointer ConstInternalPointer;

	struct Description
	{
		String version;
		String name;
		String local_name;
		std::vector<String> aliases;
	};

private:
	class OperationBookBase
	{
	protected:
		static OperationBookBase *first, *last;
		OperationBookBase *previous, *next;
		bool initialized;

		OperationBookBase(const OperationBookBase &): previous(NULL), next(NULL), initialized(false) { }
		OperationBookBase& operator= (const OperationBookBase &) { return *this; }

		OperationBookBase();

	public:
		virtual void remove_type(TypeId identifier) = 0;
		virtual void set_alias(OperationBookBase *alias) = 0;
		virtual ~OperationBookBase();

		static void remove_type_from_all_books(TypeId identifier);

		void initialize();
		void deinitialize();
		static void initialize_all();
		static void deinitialize_all();
	};

	template<typename T>
	class OperationBook : public OperationBookBase
	{
	public:
		typedef std::pair<Type*, T> Entry;
		typedef std::map<Operation::Description, Entry> Map;

		static OperationBook instance;

	private:
		Map map;
		Map *map_alias;

		OperationBook(): map_alias(&map) { }

	public:
		inline Map& get_map()
		{
#ifdef INITIALIZE_TYPE_BEFORE_USE
			if (!OperationBookBase::initialized) OperationBookBase::initialize_all();
#endif
			return *map_alias;
		}

		inline const Map& get_map() const
		{
#ifdef INITIALIZE_TYPE_BEFORE_USE
			if (!OperationBookBase::initialized) OperationBookBase::initialize_all();
#endif
			return *map_alias;
		}

		virtual void set_alias(OperationBookBase *alias)
		{
			map_alias = alias == NULL ? &map : ((OperationBook<T>*)alias)->map_alias;
			if (map_alias != &map)
			{
				map_alias->insert(map.begin(), map.end());
				map.clear();
			}
		}

		virtual void remove_type(TypeId identifier)
		{
			Map &map = get_map();
			for(typename Map::iterator i = map.begin(); i != map.end();)
				if (i->second.first->identifier == identifier)
					map.erase(i++); else ++i;
		}

		~OperationBook() {
			while(!map.empty())
				map.begin()->second.first->deinitialize();
		}
	};

	static Type *first, *last;
	static TypeId last_identifier;

	static struct StaticData {
		std::vector<Type*> typesById;
		std::map<String, Type*> typesByName;
		~StaticData() { deinitialize_all(); }
	} staticData;

	Type *previous, *next;
	bool initialized;
	TypeId private_identifier;
	Description private_description;

	Type *clone_prev, *clone_next;

public:
	const TypeId &identifier;
	const Description &description;

protected:
	explicit Type(TypeId);
	Type();

private:
	// lock default copy constructor
	Type(const Type &):
		previous(NULL), next(NULL),
		initialized(false),
		private_identifier(0),
		clone_prev(NULL),
		clone_next(NULL),
		identifier(private_identifier),
		description(private_description)
	{ assert(false); }
	// lock default assignment
	Type& operator= (const Type &) { assert(false); return *this; }

	void register_type();
	void unregister_type();

private:
	template<typename T>
	void register_operation(const Operation::Description &description, T func)
	{
		typedef typename OperationBook<T>::Entry Entry;
		typedef typename OperationBook<T>::Map Map;
		Map &map = OperationBook<T>::instance.get_map();
		assert(!map.count(description) || map[description].first == this);
		map[description] = Entry(this, func);
	}

protected:
	static String local_n(const char *x);

	virtual void initialize_vfunc(Description &description)
	{
		description.version = "0.0";
	}

	virtual void deinitialize_vfunc(Description & /* description */) { }

public:
	virtual ~Type();

	void initialize();
	void deinitialize();

	inline bool operator== (const Type &other) const { return private_identifier == other.private_identifier; }
	inline bool operator!= (const Type &other) const { return private_identifier != other.private_identifier; }

	static void initialize_all();
	static void deinitialize_all();

	inline Type* get_next() const { return next; }
	inline static Type* get_first() { return first; }

	template<typename T>
	static T get_operation(const Operation::Description &description)
	{
		typedef typename OperationBook<T>::Map Map;
		const Map &map = OperationBook<T>::instance.get_map();
		typename Map::const_iterator i = map.find(description);
		return i == map.end() ? NULL : i->second.second;
	}

	template<typename T>
	static T get_operation_by_type(const Operation::Description &description, T)
		{ return get_operation<T>(description); }

	template<typename T>
	inline static Type& get_type()
	{
		return types_namespace::get_type_alias(T()).type;
	}

	template<typename T>
	inline static const TypeId& get_type_id()
		{ return get_type<T>().identifier; }

	template<typename T>
	inline static Type& get_type_by_pointer(const T *)
		{ return get_type<T>(); }

	template<typename T>
	inline static Type& get_type_by_reference(const T &)
		{ return get_type<T>(); }

	static Type* try_get_type_by_id(TypeId id)
		{ return id < staticData.typesById.size() ? staticData.typesById[id] : NULL; }

	static Type* try_get_type_by_name(const String &name)
	{
		std::map<String, Type*>::const_iterator i = staticData.typesByName.find(name);
		return i == staticData.typesByName.end() ? NULL : i->second;
	}

	static Type& get_type_by_id(TypeId id)
		{ assert(try_get_type_by_id(id) != NULL); return *try_get_type_by_id(id); }

	static Type& get_type_by_name(const String &name)
		{ assert(try_get_type_by_name(name) != NULL); return *try_get_type_by_name(name); }

private:
	inline void register_create(TypeId type, Operation::CreateFunc func)
		{ register_operation(Operation::Description::get_create(type), func); }
	inline void register_destroy(TypeId type, Operation::DestroyFunc func)
		{ register_operation(Operation::Description::get_destroy(type), func); }
	template<typename T>
	inline void register_set(TypeId type, typename Operation::GenericFuncs<T>::SetFunc func)
		{ register_operation(Operation::Description::get_set(type), func); }
	template<typename T>
	inline void register_put(TypeId type, typename Operation::GenericFuncs<T>::PutFunc func)
		{ register_operation(Operation::Description::get_put(type), func); }
	template<typename T>
	inline void register_get(TypeId type, typename Operation::GenericFuncs<T>::GetFunc func)
		{ register_operation(Operation::Description::get_get(type), func); }

protected:
	inline void register_copy(TypeId type_a, TypeId type_b, Operation::CopyFunc func)
		{ register_operation(Operation::Description::get_copy(type_a, type_b), func); }
	inline void register_copy(TypeId type, Operation::CopyFunc func)
		{ register_operation(Operation::Description::get_copy(type), func); }
	inline void register_equal(TypeId type_a, TypeId type_b, Operation::EqualFunc func)
		{ register_operation(Operation::Description::get_equal(type_a, type_b), func); }
	inline void register_equal(TypeId type, Operation::EqualFunc func)
		{ register_operation(Operation::Description::get_equal(type), func); }
	inline void register_less(TypeId type_a, TypeId type_b, Operation::LessFunc func)
		{ register_operation(Operation::Description::get_less(type_a, type_b), func); }
	inline void register_less(TypeId type, Operation::EqualFunc func)
		{ register_operation(Operation::Description::get_less(type), func); }
	inline void register_to_string(TypeId type, Operation::ToStringFunc func)
		{ register_operation(Operation::Description::get_to_string(type), func); }
	inline void register_binary(Operation::OperationType operation_type, TypeId type_return, TypeId type_a, TypeId type_b, Operation::BinaryFunc func)
		{ register_operation(Operation::Description::get_binary(operation_type, type_return, type_a, type_b), func); }
	inline void register_binary(const Operation::Description &description, Operation::BinaryFunc func)
		{ register_operation(description, func); }

	inline void register_create(Operation::CreateFunc func)
		{ register_create(identifier, func); }
	inline void register_destroy(Operation::DestroyFunc func)
		{ register_destroy(identifier, func); }
	template<typename T>
	inline void register_set(typename Operation::GenericFuncs<T>::SetFunc func)
		{ register_set<T>(identifier, func); }
	template<typename T>
	inline void register_put(typename Operation::GenericFuncs<T>::PutFunc func)
		{ register_put<T>(identifier, func); }
	template<typename T>
	inline void register_get(typename Operation::GenericFuncs<T>::GetFunc func)
		{ register_get<T>(identifier, func); }
	inline void register_copy(Operation::CopyFunc func)
		{ register_copy(identifier, func); }
	inline void register_equal(Operation::EqualFunc func)
		{ register_equal(identifier, func); }
	inline void register_less(Operation::EqualFunc func)
		{ register_less(identifier, func); }
	inline void register_to_string(Operation::ToStringFunc func)
		{ register_to_string(identifier, func); }

	template<typename Inner, typename Outer>
	inline void register_alias()
	{
		register_set<Outer> ( Operation::DefaultFuncs::set<Inner, Outer>      );
		register_put<Outer> ( Operation::DefaultFuncs::put<Inner, Outer>      );
		register_get<Outer> ( Operation::DefaultFuncs::get<Inner, Outer>      );
	}

	template<typename Inner, typename Outer, String (*Func)(const Inner&)>
	inline void register_all_but_compare()
	{
		register_create     ( Operation::DefaultFuncs::create<Inner>          );
		register_destroy    ( Operation::DefaultFuncs::destroy<Inner>         );
		register_copy       ( Operation::DefaultFuncs::copy<Inner>            );
		register_to_string  ( Operation::DefaultFuncs::to_string<Inner, Func> );
		register_alias<Inner, Outer>();
	}

	template<typename Inner, typename Outer, String (*Func)(const Inner&)>
	inline void register_all()
	{
		register_all_but_compare<Inner, Outer, Func>();
		register_equal      ( Operation::DefaultFuncs::equal<Inner>           );
		register_less       ( Operation::DefaultFuncs::less<Inner>            );
	}

	template<typename Outer, String (*Func)(const Outer&)>
	inline void register_all()
		{ register_all<Outer, Outer, Func>(); }
	template<typename Outer, String (*Func)(const Outer&)>
	inline void register_all_but_compare()
		{ register_all_but_compare<Outer, Outer, Func>(); }

private:
	template<typename T>
	static String _value_to_string(const T &alias, const typename T::AliasedType &x)
	{
		if (alias.type.identifier == NIL) {
			Operation::ToStringFunc to_string_func =
				Type::get_operation<Operation::ToStringFunc>(
					Operation::Description::get_to_string(alias.type.identifier) );
			return to_string_func(NULL);
		}

		typedef typename T::AliasedType TT;

		Operation::CreateFunc create_func =
			Type::get_operation<Operation::CreateFunc>(
				Operation::Description::get_create(alias.type.identifier) );
		typename Operation::GenericFuncs<TT>::SetFunc set_func =
			Type::get_operation<typename Operation::GenericFuncs<TT>::SetFunc>(
				Operation::Description::get_set(alias.type.identifier) );
		Operation::ToStringFunc to_string_func =
			Type::get_operation<Operation::ToStringFunc>(
				Operation::Description::get_to_string(alias.type.identifier) );
		Operation::DestroyFunc destroy_func =
			Type::get_operation<Operation::DestroyFunc>(
				Operation::Description::get_destroy(alias.type.identifier) );
		assert(create_func != NULL);
		assert(set_func != NULL);
		assert(to_string_func != NULL);
		assert(destroy_func != NULL);

		InternalPointer data = create_func();
		set_func(data, x);
		String res = to_string_func(data);
		destroy_func(data);
		return res;
	}

public:
	template<typename T>
	static String value_to_string(const T &x)
		{ return _value_to_string(types_namespace::get_type_alias(x), x); }

public:
	static bool subsys_init() {
		initialize_all();
		return true;
	}

	static bool subsys_stop() {
		deinitialize_all();
		return true;
	}
}; // END of class Type


template<typename T>
Type::OperationBook<T> Type::OperationBook<T>::instance;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
