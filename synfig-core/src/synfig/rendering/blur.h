/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/blur.h
**	\brief Blur Header
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

#ifndef __SYNFIG_RENDERING_BLUR_H
#define __SYNFIG_RENDERING_BLUR_H

/* === H E A D E R S ======================================================= */

#include <cstring>
#include "task.h"
#include "primitive.h"
#include "surface.h"
#include "../matrix.h"
#include "../blur.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class BlurBase: public Primitive
{
public:
	typedef etl::handle<BlurBase> Handle;

private:
	::Blur::Type type;
	Vector size;
	Surface::Handle surface;
	Task::Handle task;

public:
	BlurBase(): type(::Blur::BOX) { }

	Type get_type() const { return type; }
	void set_type(Type x);

	const Vector& get_size() const { return size; }
	void set_size(const Vector &x);

	Surface::Handle get_surface() const { return surface; }
	void set_surface(const Surface::Handle &x);

	Task::Handle get_task() const { return task; }
	void set_task(const Task::Handle &x);

	void apply_common_data(const BlurBase &data);
	void changed_common_data();
};

class Blur: public BlurBase
{
public:
	typedef etl::handle<Blur> Handle;
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
