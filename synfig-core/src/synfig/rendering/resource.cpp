/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/resource.cpp
**	\brief Resource
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

#include "resource.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Resource::Id Resource::last_id = 0;

Resource::Storage::Storage(): refcount(0) { }

Resource::Storage::~Storage() { }

void
Resource::Storage::ref() const
{
	++refcount;
}

bool
Resource::Storage::unref_inactive() const
{
	return --refcount > (int)resources.size();
}

bool
Resource::Storage::unref() const
{
	if (--refcount > (int)resources.size())
	{
		refcount = 0;
		const_cast<Storage*>(this)->resources.clear();
		#ifdef ETL_SELF_DELETING_SHARED_OBJECT
		delete this;
		#endif
		return false;
	}
	return true;
}

int
Resource::Storage::count() const
{
	return refcount;
}


void
Resource::set_alternative(Handle other) const
{
	assert(other);

	if (alternatives && other->alternatives)
	{
		if (alternatives != other->alternatives)
		{
			if (other->alternatives->resources.size() > alternatives->resources.size())
				{ other->set_alternative(Handle(const_cast<Resource*>(this))); }
			else
			{
				// merge
				Storage::Handle other_alternatives = other->alternatives;
				while(!other->alternatives->resources.empty())
				{
					alternatives->resources.push_back(
						other->alternatives->resources.back() );
					other_alternatives->resources.pop_back();
					other->alternatives = alternatives;
				}
			}
		}
	}
	else
	if (alternatives)
	{
		// add
		other->alternatives = alternatives;
		alternatives->resources.push_back(other);
	}
	else
	if (other->alternatives)
		{ other->set_alternative(Handle(const_cast<Resource*>(this))); }
	else
	{
		// new storage
		alternatives = std::make_shared<Storage>();
		alternatives->resources.push_back(Handle(const_cast<Resource*>(this)));
		alternatives->resources.push_back(other);
	}
}

void
Resource::unset_alternative() const
{
	if (alternatives)
	{
		List &resources = alternatives->resources;
		for(List::iterator i = resources.begin(); i != resources.end(); ++i)
			if (i->get() == this)
				{ resources.erase(i); break; }
		alternatives.reset();
	}
}

/* === E N T R Y P O I N T ================================================= */
