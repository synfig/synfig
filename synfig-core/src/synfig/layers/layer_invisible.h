/* === S Y N F I G ========================================================= */
/*!	\file layer_invisible.h
**	\brief Invisible Layer Class Implementation
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#ifndef __SYNFIG_LAYER_INVISIBLE_H
#define __SYNFIG_LAYER_INVISIBLE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_Invisible
**	\brief Base class for invisible layers
*/
class Layer_Invisible : public Layer
{
protected:
	//! Default constructor. Not used directly.
	Layer_Invisible();

protected:
	virtual rendering::Task::Handle build_rendering_task_vfunc(Context context)const;
}; // END of class Layer_Invisible

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
