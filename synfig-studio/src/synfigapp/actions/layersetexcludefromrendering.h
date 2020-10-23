/* === S Y N F I G ========================================================= */
/*!	\file layersetexcludefromrendering.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#ifndef __SYNFIG_APP_ACTION_LAYERSETEXCLUDEFROMRENDERING_H
#define __SYNFIG_APP_ACTION_LAYERSETEXCLUDEFROMRENDERING_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfigapp/action.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class LayerSetExcludeFromRendering :
	public Undoable,
	public CanvasSpecific
{
private:

	synfig::Layer::Handle layer;
	bool new_state_set;
	bool old_state;
	bool new_state;

public:

	LayerSetExcludeFromRendering();

	static ParamVocab get_param_vocab();
	static bool is_candidate_for_exclude(const ParamList &x, bool new_state);
	synfig::String get_local_name()const;

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void perform();
	virtual void undo();
};

#define ACTION_LAYERSETEXCLUDEFROMRENDERING_DECLARE(class_name) \
	class class_name : public LayerSetExcludeFromRendering { \
	public: \
		static bool is_candidate(const ParamList &x); \
		ACTION_MODULE_EXT \
	}

ACTION_LAYERSETEXCLUDEFROMRENDERING_DECLARE(LayerSetExcludeFromRenderingOn);
ACTION_LAYERSETEXCLUDEFROMRENDERING_DECLARE(LayerSetExcludeFromRenderingOff);

#undef ACTION_LAYERSETEXCLUDEFROMRENDERING_DECLARE

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
