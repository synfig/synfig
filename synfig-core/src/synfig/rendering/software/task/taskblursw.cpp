/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskblursw.cpp
**	\brief TaskBlurSW
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include "../../common/task/taskblur.h"
#include "../../common/task/taskblend.h"
#include "tasksw.h"
#include "../function/blur.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskBlurSW: public TaskBlur, public TaskSW, public TaskInterfaceBlendToTarget
{
public:
	typedef etl::handle<TaskBlurSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual int get_target_subtask_index() const
		{ return 1; }
	virtual Color::BlendMethodFlags get_supported_blend_methods() const
		{ return Color::BLEND_METHODS_ALL & ~Color::BLEND_METHODS_STRAIGHT; }

	virtual bool run(RunParams&) const {
		if (!is_valid() || !sub_task() || !sub_task()->is_valid())
			return true;

		LockWrite la(this);
		LockRead lb(sub_task());
		if (!la || !lb)
			return false;

		Vector ppu = get_pixels_per_unit();
		Vector s = blur.size.multiply_coords(ppu);

		VectorInt offset = TaskList::calc_target_offset(*this, *sub_task());
		offset += target_rect.get_min();

		software::Blur::blur(
			software::Blur::Params(
				la->get_surface(), target_rect,
				lb->get_surface(), offset,
				blur.type, s,
				blend, blend_method, amount ));

		return false;
	}
};


Task::Token TaskBlurSW::token(
	DescReal<TaskBlurSW, TaskBlur>("BlurSW") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
