/* === S Y N F I G ========================================================= */
/*!	\file layerduplicate.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_APP_ACTION_LAYERDUPLICATE_H
#define __SYNFIG_APP_ACTION_LAYERDUPLICATE_H

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

class LayerDuplicate :
	public Super
{
private:

	synfig::GUID guid;
	std::list<synfig::Layer::Handle> layers;

public:

	LayerDuplicate();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();

	void export_dup_nodes(synfig::Layer::Handle, synfig::Canvas::Handle, int &);
	void replace_valuenodes(const std::map<synfig::Layer::Handle, synfig::Layer::Handle>& cloned_layer_map, const std::map<synfig::ValueNode::RHandle, synfig::ValueNode::RHandle>& cloned_valuenode_map);

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
