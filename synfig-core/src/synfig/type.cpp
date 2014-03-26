/* === S Y N F I G ========================================================= */
/*!	\file type.cpp
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

#include "type.h"

#endif

using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

Type::OperationBookBase *Type::OperationBookBase::first = NULL;
Type::OperationBookBase *Type::OperationBookBase::last = NULL;

Type *Type::first = NULL;
Type *Type::last = NULL;
TypeId Type::last_identifier = 0;
std::vector<Type*> Type::typesById;
std::map<String, Type*> Type::typesByName;

namespace synfig {
namespace types_namespace {
	class TypeNil: public Type
	{
	protected:
		TypeNil(): Type(NIL) { }

		static String to_string(const InternalPointer) { return "Nil"; }

		virtual void initialize_vfunc(Description &description)
		{
			Type::initialize_vfunc(description);
			description.name = "nil";
			description.local_name = N_("nil");
			description.aliases.push_back("null");
			register_default(to_string);
		}
	public:
		static TypeNil instance;
	};

	TypeNil TypeNil::instance;
}

Type::OperationBookBase::OperationBookBase():
	previous(last), next(NULL), initialized(false), alias(NULL)
{
	(previous == NULL ? first : previous->next) = last = this;
	std::cout << "Type::OperationBookBase::OperationBookBase()" << " root anchor " << (long long)(void*)&first << std::endl;
}

Type::OperationBookBase::~OperationBookBase()
{
	(previous == NULL ? first : previous->next) = next;
	(next     == NULL ? last  : next->previous) = previous;
	std::cout << "Type::OperationBookBase::~OperationBookBase()" << " root anchor " << (long long)(void*)&first << std::endl;
}

void Type::OperationBookBase::initialize()
{
	if (initialized) return;
	alias = NULL;
	for(OperationBookBase *book = first; book != NULL && book != this; book = book->next)
	{
		if (book->alias != NULL) continue;
		if (std::string(typeid(*book).name()) == typeid(*this).name())
		{
			alias = book;
			move_entries_to_alias();
			std::cout << "find another instance of type " << typeid(*this).name() << std::endl;
			if (typeid(*book) != typeid(*this))
				std::cout << "types instances of " << typeid(*this).name() << " has different typeid" << std::endl;
		}
	}
	initialized = true;
}

void Type::OperationBookBase::deinitialize()
{
	if (!initialized) return;
	alias = NULL;
}

void Type::OperationBookBase::initialize_all()
{
	std::cout << "Type::OperationBookBase::initalize_all()" << " root anchor " << (long long)(void*)&first << std::endl;
	for(OperationBookBase *book = first; book != NULL; book = book->next)
		book->initialize();
}

void Type::OperationBookBase::deinitialize_all() {
	std::cout << "Type::OperationBookBase::deinitalize_all()" << " root anchor " << (long long)(void*)&first << std::endl;
	for(OperationBookBase *book = first; book != NULL; book = book->next)
		book->deinitialize();
}


Type::Type(TypeId):
	previous(last),
	next(NULL),
	initialized(false),
	identifier(NIL),
	description(private_description)
{
	(previous == NULL ? first : previous->next) = last = this;
}

Type::Type():
	previous(last),
	next(NULL),
	initialized(false),
	identifier(++last_identifier),
	description(private_description)
{
	assert(last_identifier != NIL);
	(previous == NULL ? first : previous->next) = last = this;
}

Type::~Type()
{
	if (initialized) unregister_type();
	(previous == NULL ? first : previous->next) = next;
	(next     == NULL ? last  : next->previous) = previous;
}

void Type::initialize_all()
{
	std::cout << "Type::initialize_all()" << " root anchor " << (long long)(void*)&first << std::endl;
	OperationBookBase::initialize_all();
	for(Type *type = first; type != NULL; type = type->next)
		type->initialize();
}

void Type::deinitialize_all()
{
	std::cout << "Type::deinitialize_all()" << " root anchor " << (long long)(void*)&first << std::endl;
	for(Type *type = first; type != NULL; type = type->next)
		type->deinitialize();
	OperationBookBase::deinitialize_all();
}

Type &type_nil = types_namespace::TypeNil::instance;

}

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
