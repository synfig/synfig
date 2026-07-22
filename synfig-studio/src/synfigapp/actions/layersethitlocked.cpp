/* === S Y N F I G ========================================================= */
/*!	\file layersethitlocked.cpp
**	\brief synfigapp action LayerSetHitLocked
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2022 Synfig contributors
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

#include "layersethitlocked.h"
#include <synfigapp/canvasinterface.h>

#include <synfigapp/localization.h>

#endif

using namespace etl;
using namespace synfig;
using namespace synfigapp;
using namespace Action;

/* === M A C R O S ========================================================= */

#define ACTION_LAYERSETHITLOCKED_IMPLEMENT(class_name, local_name, task) \
	ACTION_INIT(Action::class_name); \
	ACTION_SET_NAME(Action::class_name, #class_name); \
	ACTION_SET_LOCAL_NAME(Action::class_name, local_name); \
	ACTION_SET_TASK(Action::class_name, "sethitlocked_" #task); \
	ACTION_SET_CATEGORY(Action::class_name, Action::CATEGORY_LAYER); \
	ACTION_SET_PRIORITY(Action::class_name, 0); \
	ACTION_SET_VERSION(Action::class_name, "0.0"); \
	bool Action::class_name::is_candidate(const ParamList& x) \
		{ return is_candidate_for_hitlock(x, task); }

ACTION_LAYERSETHITLOCKED_IMPLEMENT(
		LayerSetHitLockedOn, N_("Disable Layer Selection"), true);

ACTION_LAYERSETHITLOCKED_IMPLEMENT(
		LayerSetHitLockedOff, N_("Enable Layer Selection"), false);

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Action::LayerSetHitLocked::LayerSetHitLocked()
	: new_state_set(false),
	  old_state(false),
	  new_state(false)
{
}

synfig::String
Action::LayerSetHitLocked::get_local_name() const
{
	if (!layer)
		return _("Toggle Layer Locking");

	return strprintf("%s '%s'",
					 new_state
					 ? _("Disable layer selection - ")
					 : _("Enable layer selection - "),
					 layer->get_non_empty_description().c_str());
}

Action::ParamVocab
Action::LayerSetHitLocked::get_param_vocab()
{
	ParamVocab ret(Action::CanvasSpecific::get_param_vocab());

	ret.push_back(ParamDesc("layer", Param::TYPE_LAYER)
		.set_local_name(_("Layer"))
	);

	ret.push_back(ParamDesc("new_state", Param::TYPE_BOOL)
		.set_local_name(_("New State"))
		.set_desc(_("The new state of the layer selection lock"))
		.set_optional(true)
	);

	return ret;
}

bool
Action::LayerSetHitLocked::is_candidate_for_hitlock(const ParamList& x, bool new_state)
{
	if (!candidate_check(get_param_vocab(), x))
		return false;

	Layer::Handle l(x.find("layer")->second.get_layer());

	if (l->is_hit_locked() == new_state)
		return false;
	else
		return true;
}

bool
Action::LayerSetHitLocked::set_param(const synfig::String& name, const Action::Param& param)
{
	if (name == "layer" && param.get_type() == Param::TYPE_LAYER)
	{
		layer = param.get_layer();
		if (layer && !new_state_set)
			new_state = !layer->is_hit_locked();
		return true;
	}

	if(name == "new_state" && param.get_type() == Param::TYPE_BOOL)
	{
		new_state = param.get_bool();
		new_state_set = true;
		return true;
	}

	return Action::CanvasSpecific::set_param(name, param);
}

bool
Action::LayerSetHitLocked::is_ready() const
{
	if (!layer)
		return false;
	return Action::CanvasSpecific::is_ready();
}

void
Action::LayerSetHitLocked::perform()
{
	Canvas::Handle subcanvas(layer->get_canvas());

	// Find the iterator for the layer
	Canvas::iterator iter = find(subcanvas->begin(), subcanvas->end(), layer);

	// If we couldn't find the layer in the canvas, then bail
	if (*iter != layer)
		throw Error(_("This layer doesn't exist anymore."));

	old_state = layer->is_hit_locked();

	if (new_state == old_state) {
		set_dirty(false);
		return;
	} else {
		set_dirty();
	}

	layer->set_hit_locked(new_state);

	if (get_canvas_interface())
		get_canvas_interface()->signal_layer_hit_locked_changed()(layer, new_state);
	else
		synfig::warning("CanvasInterface not set on action");
}

void
Action::LayerSetHitLocked::undo()
{
	if (new_state == old_state) {
		set_dirty(false);
		return;
	} else {
		set_dirty();
	}

	layer->set_hit_locked(old_state);

	if (get_canvas_interface())
		get_canvas_interface()->signal_layer_hit_locked_changed()(layer, old_state);
	else
		synfig::warning("CanvasInterface not set on action");
}
