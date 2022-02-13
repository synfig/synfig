/* === S Y N F I G ========================================================= */
/*!	\file type.cpp
**	\brief Template Header
**
**	\legal
**	......... ... 2014 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "general.h"
#include <synfig/localization.h>

#endif

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

Type::OperationBookBase *Type::OperationBookBase::first = NULL;
Type::OperationBookBase *Type::OperationBookBase::last = NULL;

SYNFIG_EXPORT Type *Type::first = NULL;
SYNFIG_EXPORT Type *Type::last = NULL;
TypeId Type::last_identifier = 0;
Type::StaticData Type::staticData;


Type::OperationBookBase::OperationBookBase():
	previous(last), next(NULL), initialized(false)
{
	(previous == NULL ? first : previous->next) = last = this;
}

Type::OperationBookBase::~OperationBookBase()
{
	(previous == NULL ? first : previous->next) = next;
	(next     == NULL ? last  : next->previous) = previous;
}

void Type::OperationBookBase::remove_type_from_all_books(TypeId identifier)
{
	for(OperationBookBase *book = first; book != NULL; book = book->next)
		book->remove_type(identifier);
}

void Type::OperationBookBase::initialize()
{
	if (initialized) return;
	std::string type_name(typeid(*this).name());
	for(OperationBookBase *book = first; book != NULL && book != this; book = book->next)
	{
		book->initialize();
		if (typeid(*book).name() == type_name)
		{
			set_alias(book);
			break;
		}
	}
	initialized = true;
}

void Type::OperationBookBase::deinitialize()
{
	if (!initialized) return;
	set_alias(NULL);
}

void Type::OperationBookBase::initialize_all()
{
	for(OperationBookBase *book = first; book != NULL; book = book->next)
		book->initialize();
}

void Type::OperationBookBase::deinitialize_all() {
	for(OperationBookBase *book = first; book != NULL; book = book->next)
		book->deinitialize();
}


Type::Type(TypeId):
	previous(last), next(NULL),
	initialized(false),
	private_identifier(NIL),
	clone_prev(NULL),
	clone_next(NULL),
	identifier(private_identifier),
	description(private_description)
{
	(previous == NULL ? first : previous->next) = last = this;
}

Type::Type():
	previous(last), next(NULL),
	initialized(false),
	private_identifier(++last_identifier),
	clone_prev(NULL),
	clone_next(NULL),
	identifier(private_identifier),
	description(private_description)
{
	assert(last_identifier != NIL);
	(previous == NULL ? first : previous->next) = last = this;
}

Type::~Type()
{
	deinitialize();
	(previous == NULL ? first : previous->next) = next;
	(next     == NULL ? last  : next->previous) = previous;
}

void Type::register_type()
{
	// register id
	if (staticData.typesById.size() <= identifier) staticData.typesById.resize(identifier + 1, NULL);
	assert(staticData.typesById[identifier] == NULL);
	staticData.typesById[identifier] = this;

	// register names
	staticData.typesByName[description.name] = this;
	for(std::vector<String>::const_iterator i = description.aliases.begin(); i != description.aliases.end(); ++i)
	{
		assert(!staticData.typesByName.count(*i) || staticData.typesByName[*i] == this);
		staticData.typesByName[*i] = this;
	}
}

void Type::unregister_type()
{
	// unregister operations
	OperationBookBase::remove_type_from_all_books(identifier);

	// unregister id
	if (staticData.typesById.size() > identifier) staticData.typesById[identifier] = NULL;

	// unregister names
	staticData.typesByName.erase(description.name);
	for(std::vector<String>::const_iterator i = description.aliases.begin(); i != description.aliases.end(); ++i)
		staticData.typesByName.erase(*i);
}

String Type::local_n(const char *x)
{
	return N_(x);
}

void Type::initialize()
{
	if (initialized) return;
	if (clone_prev != NULL) { clone_prev->initialize(); return; }

	// find clones
	if (clone_next == NULL) {
		std::string type_name(typeid(*this).name());
		for(Type *i = first; i != NULL; i = i->next)
		{
			if (i != this && typeid(*i).name() == type_name)
			{
				clone_prev = i;
				clone_next = clone_prev->clone_next;
				if (clone_prev != NULL) clone_prev->clone_next = this;
				if (clone_next != NULL) clone_next->clone_prev = this;
				i->initialize();
				private_identifier = clone_prev->identifier;
				private_description = clone_prev->private_description;
				return;
			}
		}
	}

	initialize_vfunc(private_description);
	register_type();
	initialized = true;
}

void Type::deinitialize()
{
	if (!initialized && clone_prev == NULL) return;

	Type *initialize_next = NULL;
	if (clone_prev == NULL)
	{
		unregister_type();
		deinitialize_vfunc(private_description);
		initialized = false;
		initialize_next = clone_next;
	}

	// unassign clone
	if (clone_prev != NULL) clone_prev->clone_next = clone_next;
	if (clone_next != NULL) clone_next->clone_prev = clone_prev;
	clone_prev = NULL;
	clone_next = NULL;

	if (initialize_next != NULL) initialize_next->initialize();
}

void Type::initialize_all()
{
	OperationBookBase::initialize_all();
	for(Type *type = first; type != NULL; type = type->next)
		type->initialize();
}

void Type::deinitialize_all()
{
	for(Type *type = first; type != NULL; type = type->next)
		type->deinitialize();
	OperationBookBase::deinitialize_all();
}


namespace synfig {
namespace types_namespace {
	class TypeNil: public Type
	{
	protected:
		TypeNil(): Type(NIL) { }

		static String to_string(ConstInternalPointer) { return "Nil"; }

		virtual void initialize_vfunc(Description &description)
		{
			Type::initialize_vfunc(description);
			description.name = "nil";
			description.local_name = local_n("nil");
			description.aliases.push_back("null");
			register_to_string(to_string);
		}
	public:
		SYNFIG_EXPORT static TypeNil instance;
	};

	TypeNil TypeNil::instance;
}

Type &type_nil = types_namespace::TypeNil::instance;
}

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
