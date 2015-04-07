/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/transformationstack.h
**	\brief TransformationStack Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TRANSFORMATIONSTACK_H
#define __SYNFIG_RENDERING_TRANSFORMATIONSTACK_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include "transformation.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TransformationStack: public Transformation
{
public:
	typedef etl::handle<TransformationStack> Handle;
	typedef std::vector<Transformation::Handle> Stack;

private:
	Stack stack;

protected:
	virtual TransformedPoint transform_vfunc(const Point &x) const;

public:
	const Stack& get_stack() const
		{ return stack; }
	int get_count() const
		{ return get_stack().size(); }
	bool empty() const
		{ return get_stack().empty(); }
	const Transformation::Handle& get(int index) const
		{ return get_stack()[index]; }

	void insert(int index, const Transformation::Handle &x);
	void remove(int index);
	void remove(const Transformation::Handle &x);
	void clear();

	void add(const Transformation::Handle &x)
		{ insert(get_count(), x); }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
