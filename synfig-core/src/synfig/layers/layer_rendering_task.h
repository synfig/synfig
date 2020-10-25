/* === S Y N F I G ========================================================= */
/*!	\file layer_rendering_task.h
**	\brief Layer_RenderingTask Headers
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

#ifndef __SYNFIG_LAYER_RENDERING_TASK_H
#define __SYNFIG_LAYER_RENDERING_TASK_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/rendering/renderer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Layer_RenderingTask
**	\brief Temporary class to support layers which yet not converted to new engine
*/
class Layer_RenderingTask : public Layer
{
private:
	//void put_sub_surface(Surface &dest, RectInt dest_rect, const RendDesc &renddesc, ProgressCallback *cb)const;

public:
	rendering::Task::List tasks;

	Layer_RenderingTask();

	virtual Rect get_bounding_rect()const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual bool accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const;
}; // END of class Layer_RenderingTask

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
