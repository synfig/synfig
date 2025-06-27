/* === S Y N F I G ========================================================= */
/*!	\file layersethitlocked.h
**	\brief Header for synfigapp action LayerSetHitLocked
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

/* === S T A R T =========================================================== */

#ifndef SYNFIG_APP_ACTION_LAYERSETHITLOCKED_H
#define SYNFIG_APP_ACTION_LAYERSETHITLOCKED_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfigapp/action.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class LayerSetHitLocked :
	public Undoable,
	public CanvasSpecific
{
private:

	synfig::Layer::Handle layer;
	bool new_state_set;
	bool old_state;
	bool new_state;

public:

	LayerSetHitLocked();

	static ParamVocab get_param_vocab();
	static bool is_candidate_for_hitlock(const ParamList& x, bool new_state);
	synfig::String get_local_name() const override;

	bool set_param(const synfig::String& name, const Param& param) override;
	bool is_ready() const override;

	void perform() override;
	void undo() override;
};

#define ACTION_LAYERSETHITLOCKED_DECLARE(class_name) \
	class class_name : public LayerSetHitLocked { \
	public: \
		static bool is_candidate(const ParamList &x); \
		ACTION_MODULE_EXT \
	}

ACTION_LAYERSETHITLOCKED_DECLARE(LayerSetHitLockedOn);
ACTION_LAYERSETHITLOCKED_DECLARE(LayerSetHitLockedOff);

#undef ACTION_LAYERSETHITLOCKED_DECLARE

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif // SYNFIG_APP_ACTION_LAYERSETHITLOCKED_H
