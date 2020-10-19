/* === S Y N F I G ========================================================= */
/*!	\file transform.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_TRANSFORM_H
#define __SYNFIG_TRANSFORM_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include "vector.h"
#include <list>
#include "rect.h"
#include "guid.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Transform : public etl::shared_object
{
	GUID guid_;

public:
	typedef etl::handle<Transform> Handle;

protected:
	Transform(const GUID& guid):guid_(guid) { }

public:

	const GUID& get_guid()const { return guid_; }

	virtual ~Transform() { }
	virtual synfig::Vector perform(const synfig::Vector& x)const=0;
	virtual synfig::Vector unperform(const synfig::Vector& x)const=0;

	virtual synfig::Rect perform(const synfig::Rect& x)const;
	virtual synfig::Rect unperform(const synfig::Rect& x)const;

	virtual String get_string()const=0;

}; // END of class Transform

class TransformStack : public std::list<Transform::Handle>
{
public:
	GUID get_guid()const;

	synfig::Vector perform(const synfig::Vector& x)const;
	synfig::Vector unperform(const synfig::Vector& x)const;

	synfig::Rect perform(const synfig::Rect& x)const;
	synfig::Rect unperform(const synfig::Rect& x)const;

	void push(const Transform::Handle& x) { if(x)push_back(x); }
	void pop() { pop_back(); }
}; // END of class TransformStack

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
