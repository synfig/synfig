/* === S Y N F I G ========================================================= */
/*!	\file layermakebline.h
**	\brief Template File
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

#ifndef __SYNFIG_APP_ACTION_LAYERMAKEBLINE_H
#define __SYNFIG_APP_ACTION_LAYERMAKEBLINE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfigapp/action.h>
#include <list>
#include <synfig/guid.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class LayerMakeBLine :
	public Super
{
private:
	synfig::Layer::Handle layer;

public:
	static ParamVocab get_param_vocab();
	static bool is_candidate_for_make_bline(const ParamList &x, const char **possible_layer_names, size_t possible_layer_names_count);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	void prepare_make_bline(const synfig::String &bline_layer_name, bool put_new_layer_behind);
};

#define ACTION_LAYERMAKEBLINE_DECLARE(class_name) \
	class class_name : public LayerMakeBLine { \
	private: \
		static const char *possible_layer_names__[]; \
	public: \
		static bool is_candidate(const ParamList &x); \
		virtual void prepare(); \
		ACTION_MODULE_EXT \
	}

ACTION_LAYERMAKEBLINE_DECLARE(LayerMakeOutline);
ACTION_LAYERMAKEBLINE_DECLARE(LayerMakeAdvancedOutline);
ACTION_LAYERMAKEBLINE_DECLARE(LayerMakeRegion);
ACTION_LAYERMAKEBLINE_DECLARE(LayerMakeCurveGradient);
ACTION_LAYERMAKEBLINE_DECLARE(LayerMakePlant);

#undef ACTION_LAYERMAKEBLINE_DECLARE

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
