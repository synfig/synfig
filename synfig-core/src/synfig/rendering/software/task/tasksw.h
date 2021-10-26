/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/tasksw.h
**	\brief TaskSW Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TASKSW_H
#define __SYNFIG_RENDERING_TASKSW_H

/* === H E A D E R S ======================================================= */

#include "../../task.h"
#include "../surfacesw.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskSW: public Mode
{
public:
	typedef SurfaceSW TargetSurface;
	typedef Task::LockReadGeneric<TargetSurface> LockRead;
	typedef Task::LockWriteGeneric<TargetSurface> LockWrite;

	SYNFIG_EXPORT static ModeToken mode_token;
	virtual Surface::Token::Handle get_mode_target_token() const
		{ return TargetSurface::token.handle(); }
	virtual bool get_mode_allow_source_as_target() const
		{ return true; }
	virtual bool get_mode_allow_simultaneous_write() const
		{ return true; }
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
