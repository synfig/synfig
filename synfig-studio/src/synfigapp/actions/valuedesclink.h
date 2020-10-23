/* === S Y N F I G ========================================================= */
/*!	\file valuedesclink.h
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2010 Carlos López
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

#ifndef __SYNFIG_APP_ACTION_VALUEDESCLINK_H
#define __SYNFIG_APP_ACTION_VALUEDESCLINK_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfigapp/value_desc.h>
#include <list>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class ValueDescLink :
	public Super
{
private:
	friend class ValueDescLinkOpposite;
	//! List of Value Descriptions retrieved from the action Parameters list
	//! to be linked
	std::list<ValueDesc> value_desc_list;
	//! Selected value node from the Value Descriptions (maybe none)
	//! to which the others would link
	synfig::ValueNode::Handle link_value_node;
	//! If poison is true then Link is not possible (two exported value nodes found)
	bool poison;
	//! Used to monitorize the tie decision when selecting the link value node
	int status_level;
	//! Message to inform the status of the tie decision when selecting the link value node
	synfig::String status_message;
	//! Time where the value nodes are evaluated
	synfig::Time time;
	//! If true then link opposite is being called.
	bool link_opposite;
public:

	ValueDescLink();

	static ParamVocab get_param_vocab();
	static bool is_candidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();

	ACTION_MODULE_EXT
};

class ValueDescLinkOpposite :
	public ValueDescLink
{
public:

	ValueDescLinkOpposite();

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
