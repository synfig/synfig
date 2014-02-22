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

Type &type_nil = types_namespace::TypeNil::instance;
}

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */
