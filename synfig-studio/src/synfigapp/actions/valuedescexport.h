/* === S Y N F I G ========================================================= */
/*!	\file valuedescexport.h
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_APP_ACTION_VALUEDESCEXPORT_H
#define __SYNFIG_APP_ACTION_VALUEDESCEXPORT_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfigapp/value_desc.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class ValueDescExport :
	public Super
{
private:
	synfig::GUID guid;
	ValueDesc value_desc;
	synfig::String name;

	void scan_canvas(synfig::Canvas::Handle prev_canvas, synfig::Canvas::Handle new_canvas, synfig::Canvas::Handle canvas);
	void scan_layer(synfig::Canvas::Handle prev_canvas, synfig::Canvas::Handle new_canvas, synfig::Layer::Handle layer);
	void scan_linkable_value_node(synfig::Canvas::Handle prev_canvas, synfig::Canvas::Handle new_canvas, synfig::LinkableValueNode::Handle linkable_value_node);

public:

	ValueDescExport();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
