/* === S Y N F I G ========================================================= */
/*!	\file layer_composite_fork.h
**	\brief Layer_CompositeFork Class Headers
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#ifndef __SYNFIG_LAYER_COMPOSITE_FORK_H
#define __SYNFIG_LAYER_COMPOSITE_FORK_H

/* === H E A D E R S ======================================================= */

#include "layer_composite.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_CompositeFork
**	\brief Base class for composite layers which also
**	 do manipulations with context
*/
class Layer_CompositeFork : public Layer_Composite
{
protected:
	//! Default constructor. Not used directly.
	explicit Layer_CompositeFork(Real amount=1.0, Color::BlendMethod blend_method=Color::BLEND_COMPOSITE);
	virtual rendering::Task::Handle build_composite_fork_task_vfunc(ContextParams context_params, rendering::Task::Handle sub_task)const;
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context)const;
}; // END of class Layer_Invisible

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
