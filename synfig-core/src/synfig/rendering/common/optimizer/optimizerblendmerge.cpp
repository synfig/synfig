/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/optimizer/optimizerblendmerge.cpp
**	\brief OptimizerBlendMerge
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

#include <synfig/general.h>
#include <synfig/localization.h>

#include "optimizerblendmerge.h"

#include "../task/taskblend.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


OptimizerBlendMerge::OptimizerBlendMerge()
{
	category_id = CATEGORY_ID_SPECIALIZED;
	depends_from = CATEGORY_COORDS;
	mode = MODE_REPEAT_PARENT;
	deep_first = true;
	for_task = true;
}

void
OptimizerBlendMerge::run(const RunParams& params) const
{
	//
	// merge blend (only for BLEND_COMPOSITE)
	//
	//  blendA(targetA)
	//  - taskB(targetB)
	//  - blendC(targetC)
	//    - none
	//    - taskD(targetD)
	//
	// converts to:
	//
	//  blendAC(targetA)
	//  - taskB(targetB)
	//  - taskD(targetD)
	//

	TaskBlend::Handle blend = TaskBlend::Handle::cast_dynamic(params.ref_task);
	if ( blend
	  && blend->blend_method == Color::BLEND_COMPOSITE )
	{
		TaskBlend::Handle sub_blend = TaskBlend::Handle::cast_dynamic(blend->sub_task_b());
		if ( sub_blend
		  && (!sub_blend->sub_task_a())
		  && blend->blend_method == sub_blend->blend_method )
		{
			TaskBlend::Handle new_blend = TaskBlend::Handle::cast_dynamic(blend->clone());
			new_blend->amount *= sub_blend->amount;
			new_blend->sub_task_b() = sub_blend->sub_task_b();
			apply(params, new_blend);
		}
	}
}

/* === E N T R Y P O I N T ================================================= */
