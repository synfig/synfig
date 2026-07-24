/* === S Y N F I G ========================================================= */
/*!	\file layerupgrade.h
**	\brief Upgrade a deprecated layer to its current replacement
**
**	\legal
**	Copyright (C) 2026 Synfig Contributors
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

#ifndef __SYNFIG_APP_ACTION_LAYERUPGRADE_H
#define __SYNFIG_APP_ACTION_LAYERUPGRADE_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <synfig/layer.h>
#include <synfigapp/action.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

// Replaces a deprecated layer with its current equivalent, carrying over
// parameters, animated (dynamic) links, description, z-depth, group and
// active state. Driven by a deprecated->current name map, so adding a new
// upgrade pair needs no code changes beyond the map (see get_target_layer_name).
class LayerUpgrade :
	public Super
{
private:
	std::list<synfig::Layer::Handle> layers;

	void prepare_upgrade_layer(const synfig::Layer::Handle &layer, const synfig::String &target_name);

	// Returns the current layer name for a deprecated one, or empty if unknown.
	static synfig::String get_target_layer_name(const synfig::String &source_layer_name);

public:

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();

	ACTION_MODULE_EXT
};

}; // END of namespace Action
}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
