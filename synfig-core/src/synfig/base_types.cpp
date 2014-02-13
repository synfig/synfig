/* === S Y N F I G ========================================================= */
/*!	\file base_types.cpp
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

#include "base_types.h"

#endif

using namespace synfig;
using namespace types_namespace;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// TypeReal

namespace synfig {
namespace types_namespace {

class TypeReal: public TypeComparableGeneric<Real>
{
protected:
	virtual void initialize_vfunc(Description &description)
	{
		TypeComparableGeneric<Real>::initialize_vfunc(description);
		description.name = "real";
		description.local_name = N_("name");
	}

public:
	static TypeReal instance;

	virtual String to_string(const Real &x)
	{
		return etl::strprintf("Real (%f)", x);
	}
};

TypeReal TypeReal::instance;
Type& get_type(const Real*) { return TypeReal::instance; }

}} // END of namespaces types_namespace and synfig


namespace synfig {
	Type &type_real = TypeReal::instance;
}; // END of namespace synfig
