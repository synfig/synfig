/* === S Y N F I G ========================================================= */
/*!	\file layertotop.h
**	\brief Action to raise layer to top of the layer stack
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2023 Synfig Contributors
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

#ifndef LAYERTOTOP_H
#define LAYERTOTOP_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfigapp/action.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class LayerToTop : public Super
{
private:

	std::list<synfig::Layer::Handle> layers;

public:

	LayerToTop();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	bool set_param(const synfig::String& name, const Param &) override;
	bool is_ready()const override;

	void prepare() override;

	ACTION_MODULE_EXT
};

} // END of namespace action
} // END of namespace studio

/* === E N D =============================================================== */

#endif // LAYERTOTOP_H
