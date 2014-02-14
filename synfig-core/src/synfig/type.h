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
#include "string.h"
#include "general.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Type;

namespace types_namespace
{
	template<typename T>
	inline static synfig::Type& get_type()
	{
		Type& get_type(const T*);
		return get_type((const T*)NULL);
	}

	extern Type &type_nil;
}

typedef unsigned int TypeId;

/*!	\class Operation
**	\brief Provides methods to create operation with Values
*/
class Operation
{
public:
	enum OperationType {
		TYPE_NONE,
		TYPE_COMPARE,
	};

	typedef void* (*BinaryFunc)(const void*, const void*);
	typedef bool (*CompareFunc)(const void*, const void*);

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
			return return_type < other.return_type ? true
				 : other.return_type < return_type ? false
				 : type_a < other.type_a ? true
				 : other.type_a < type_a ? false
				 : type_b < other.type_b;
		}

		static Description comparison(TypeId type_a, TypeId type_b)
			{ return Description(TYPE_COMPARE, 0, type_a, type_b); }
		static Description comparison(TypeId type)
			{ return Description(TYPE_COMPARE, 0, type); }
	};

	class Convert
	{
	public:
		template<typename ReturnType, typename Func>
		static void* operation0(const void*, const void*) { return new ReturnType(Func()); }
		template<typename ReturnType, typename Func, typename TypeA>
		static void* operation1(const void *a, const void*) { return new ReturnType(Func(*(const TypeA*)a)); }
		template<typename ReturnType, typename Func, typename TypeA, typename TypeB>
		static void* operation2(const void *a, const void *b) { return new ReturnType(Func(*(const TypeA*)a, *(const TypeB*)b)); }
		template<typename TypeA, typename TypeB>
		static bool comparison(const void *a, const void *b) { return *(const TypeA*)a == *(const TypeB*)b; }
	private:
		Convert() { }
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
	struct Description
	{
		String version;
		String name;
		String local_name;
		std::vector<String> aliases;
	};

	enum { NIL = 0 };

private:
	class OperationBookBase
	{
	private:
		static OperationBookBase *first, *last;
		OperationBookBase *previous, *next;

		OperationBookBase(const OperationBookBase &): previous(NULL), next(NULL) { }
		OperationBookBase& operator= (const OperationBookBase &) { return *this; }

	protected:
		OperationBookBase():
			previous(last), next(NULL)
		{
			(previous == NULL ? first : previous->next) = last = this;
		}

	public:
		virtual void remove_type(TypeId identifier) = 0;
		virtual ~OperationBookBase()
		{
			(previous == NULL ? first : previous->next) = next;
			(next     == NULL ? last  : next->previous) = previous;
		}

		static void remove_type_from_all_books(TypeId identifier)
		{
			for(OperationBookBase *book = first; book != NULL; book = book->next)
				book->remove_type(identifier);
		}
	};

	template<typename T>
	class OperationBook
	{
	public:
		typedef std::pair<TypeId, T> Entry;
		typedef std::map<Operation::Description, Entry > Map;

		static OperationBook instance;
		Map map;

		virtual void remove_type(TypeId identifier)
		{
			for(typename Map::iterator i = map.begin(); i != map.end();)
				if (i->second.first == identifier)
					map.erase(i++); else ++i;
		}
	};

	static Type *first, *last;
	static TypeId last_identifier;

	static std::vector<Type*> typesById;
	static std::map<String, Type*> typesByName;

	Type *previous, *next;
	bool initialized;
	Description private_description;

public:
	const TypeId identifier;
	const Description &description;

protected:
	Type(TypeId):
		previous(last),
		next(NULL),
		initialized(false),
		identifier(NIL),
		description(private_description)
	{
		(previous == NULL ? first : previous->next) = last = this;
	}

	Type():
		previous(last),
		next(NULL),
		initialized(false),
		identifier(++last_identifier),
		description(private_description)
	{
		assert(last_identifier != 0);
		(previous == NULL ? first : previous->next) = last = this;
	}

private:
	// lock default copy constructor
	Type(const Type &):
		previous(NULL), next(NULL),
		initialized(false),
		identifier(0), description(private_description) { }
	// lock default assignment
	Type& operator= (const Type &) { return *this; }

	void register_type()
	{
		// register id
		if (typesById.size() <= identifier) typesById.resize(identifier + 1, NULL);
		assert(typesById[identifier] == NULL);
		typesById[identifier] = this;

		// register names
		assert(!typesByName.count(description.name));
		typesByName[description.name] = this;
		for(std::vector<String>::const_iterator i = description.aliases.begin(); i != description.aliases.end(); ++i)
		{
			assert(!typesByName.count(*i));
			typesByName[*i] = this;
		}
	}

	void unregister_type()
	{
		// unregister operations
		OperationBookBase::remove_type_from_all_books(identifier);

		// unregister id
		if (typesById.size() > identifier) typesById[identifier] = NULL;

		// unregister names
		typesByName.erase(description.name);
		for(std::vector<String>::const_iterator i = description.aliases.begin(); i != description.aliases.end(); ++i)
			typesByName.erase(*i);
	}

protected:
	template<typename T>
	void register_operation(const Operation::Description &description, T func)
	{
		typedef typename OperationBook<T>::Entry Entry;
		typedef typename OperationBook<T>::Map Map;
		Map &map = OperationBook<T>::instance.map;
		assert(!map.count(description) || map[description].first == identifier);
		map[description] = Entry(identifier, func);
	}

	virtual void initialize_vfunc(Description &description)
	{
		description.version = "0.0";
	}

public:
	virtual void* create() = 0;
	virtual void assign(void *dest, const void *src) = 0;
	virtual void destroy(const void *data) = 0;
	virtual String to_string(const void *data) = 0;

	void initialize()
	{
		if (initialized) return;
		initialize_vfunc(private_description);
		register_type();
		initialized = true;
	}

	virtual ~Type()
	{
		if (initialized) unregister_type();
		(previous == NULL ? first : previous->next) = next;
		(next     == NULL ? last  : next->previous) = previous;
	}

	static void initialize_all()
	{
		for(Type *type = first; type != NULL; type = type->next)
			type->initialize();
	}

	template<typename T>
	static T get_operation(const Operation::Description &description)
	{
		typedef typename OperationBook<T>::Map Map;
		typename Map::const_iterator i = OperationBook<T>::instance.map.find(description);
		return i == OperationBook<T>::instance.map.end() ? NULL : i->second.second;
	}

	template<typename T>
	static T get_operation_by_type(const Operation::Description &description, T)
		{ return get_operation<T>(description); }

	static Operation::CompareFunc get_comparison(const Operation::Description &description)
		{ return get_operation<Operation::CompareFunc>(description); }

	template<typename T>
	inline static Type& get_type()
	{
		return types_namespace::get_type<T>();
	}

	template<typename T>
	inline static const TypeId& get_type_id()
		{ return get_type<T>().identifier; }

	template<typename T>
	inline static Type& get_type_by_pointer(const T *x)
		{ return get_type<T>(); }

	template<typename T>
	inline static Type& get_type_by_reference(const T &x)
		{ return get_type<T>(); }

	static Type* try_get_type_by_id(TypeId id)
		{ return id < typesById.size() ? typesById[id] : NULL; }

	static Type* try_get_type_by_name(const String &name)
	{
		std::map<String, Type*>::const_iterator i = typesByName.find(name);
		return i == typesByName.end() ? NULL : i->second;
	}

	static Type& get_type_by_id(TypeId id)
		{ assert(try_get_type_by_id(id) != NULL); return *try_get_type_by_id(id); }

	static Type& get_type_by_name(const String &name)
		{ assert(try_get_type_by_name(name) != NULL); return *try_get_type_by_name(name); }

protected:
	template<typename ReturnType, typename Func>
	void register_operation0(Operation::OperationType operation_type)
	{
		register_operation(
			Operation::Description( operation_type,
								    get_type<ReturnType>() ),
			&Operation::Convert::operation0<ReturnType, Func> );
	}

	template<typename ReturnType, typename Func, typename TypeA>
	void register_operation1(Operation::OperationType operation_type)
	{
		register_operation(
			Operation::Description( operation_type,
									get_type_id<ReturnType>(),
									get_type_id<TypeA>() ),
			&Operation::Convert::operation1<ReturnType, Func, TypeA> );
	}

	template<typename ReturnType, typename Func, typename TypeA, typename TypeB>
	void register_operation2(Operation::OperationType operation_type)
	{
		register_operation(
			Operation::Description( operation_type,
									get_type_id<ReturnType>(),
									get_type_id<TypeA>(),
									get_type_id<TypeB>() ),
			&Operation::Convert::operation2<ReturnType, Func, TypeA, TypeB> );
	}

	template<typename TypeA, typename TypeB>
	void register_comparison(Operation::OperationType operation_type = Operation::TYPE_COMPARE)
	{
		register_operation(
			Operation::Description( operation_type,
									TypeId(NIL),
									get_type_id<TypeA>(),
									get_type_id<TypeB>() ),
			&Operation::Convert::comparison<TypeA, TypeB> );
	}

	void register_comparison(Operation::CompareFunc func)
		{ register_operation(Operation::Description::comparison(identifier), func); }
}; // END of class Type


template<typename T>
Type::OperationBook<T> Type::OperationBook<T>::instance;


template<typename T>
class TypeGeneric: public Type
{
public:
	typedef T Base;
	typedef TypeGeneric Parent;
	virtual void* create() { return new Base(); }
	virtual void assign(void *dest, const void *src) { *(Base*)dest = *(const Base*)src; }
	virtual void destroy(const void *data) { delete (const Base*)data; }
	virtual String to_string(const T &x) = 0;
	virtual String to_string(const void *data) { return to_string(*(const Base*)data); }
}; // END of class TypeGeneric


template<typename T>
class TypeComparableGeneric: public TypeGeneric<T>
{
protected:
	typedef TypeComparableGeneric Parent;
	virtual void initialize_vfunc(Type::Description &description)
	{
		TypeGeneric<T>::initialize_vfunc(description);
		Type::register_comparison<T, T>();
	}
}; // END of class TypeComparableGeneric


}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
