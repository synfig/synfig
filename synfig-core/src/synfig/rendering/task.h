/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/task.h
**	\brief Task Header
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

#ifndef __SYNFIG_RENDERING_TASK_H
#define __SYNFIG_RENDERING_TASK_H

/* === H E A D E R S ======================================================= */

#include "renderer.h"
#include "surface.h"
#include "transformation.h"
#include "blending.h"
#include "primitive.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Surface;

class Task: public etl::shared_object
{
public:
	typedef etl::handle<Task> Handle;

private:
	Transformation::Handle transformation;
	Blending::Handle blending;
	Primitive::Handle primitive;

	Handle branch;
	Handle next;

public:
	Task();

	const Transformation::Handle& get_transformation() const
		{ return transformation; }
	void set_transformation(const Transformation::Handle &x)
		{ transformation = x; }

	const Blending::Handle& get_blending() const
		{ return blending; }
	void set_blending(const Blending::Handle &x)
		{ blending = x; }

	const Primitive::Handle& get_primitive() const
		{ return primitive; }
	void set_primitive(const Primitive::Handle &x)
		{ primitive = x; }

	const Handle& get_branch() const
		{ return branch; }
	void set_branch(const Handle &x)
		{ branch = x; }

	const Handle& get_next() const
		{ return next; }
	void set_next(const Handle &x)
		{ next = x; }

	bool draw(
		const Renderer::Handle &renderer,
		const Renderer::Params &params,
		const Surface::Handle &surface ) const;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
