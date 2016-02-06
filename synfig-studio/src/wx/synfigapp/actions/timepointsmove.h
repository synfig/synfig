/* === S Y N F I G ========================================================= */
/*!	\file timepointsmove.h
**	\brief Move the Time Points Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#ifndef __SYNFIG_APP_ACTION_TIMEPOINTSMOVE_H
#define __SYNFIG_APP_ACTION_TIMEPOINTSMOVE_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfig/time.h>
#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfigapp/value_desc.h>

#include <vector>
#include <set>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class TimepointsMove :
	public Super
{
private:

	//process all the value descriptions that are selected (or are in subselections)
	std::vector<synfig::Layer::Handle>	sel_layers;
	std::vector<synfig::Canvas::Handle>	sel_canvases;
	std::vector<synfigapp::ValueDesc>	sel_values;
	std::set<synfig::Time>				sel_times;

	synfig::Time							timemove;

public:

	TimepointsMove();

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
