/* === S Y N F I G ========================================================= */
/*!	\file timepointsdelete.h
**	\brief Delete the Time Points Header
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#ifndef __SYNFIG_APP_ACTION_TIMEPOINTSDELETE_H
#define __SYNFIG_APP_ACTION_TIMEPOINTSDELETE_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfig/time.h>
#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class TimepointsDelete :
	public Super
{
private:

	//process all the value descriptions that are selected (or are in subselections)
	std::vector<synfig::Layer::Handle>	sel_layers;
	std::vector<synfig::Canvas::Handle>	sel_canvases;
	std::vector<synfigapp::ValueDesc>	sel_values;
	std::set<synfig::Time>				sel_times;

public:

	TimepointsDelete();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();
	virtual void perform();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
