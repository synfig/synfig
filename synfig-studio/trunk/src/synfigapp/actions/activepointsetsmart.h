/* === S Y N F I G ========================================================= */
/*!	\file activepointsetsmart.h
**	\brief Template File
**
**	$Id: activepointsetsmart.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_APP_ACTION_ACTIVEPOINTSETSMART_H
#define __SYNFIG_APP_ACTION_ACTIVEPOINTSETSMART_H

/* === H E A D E R S ======================================================= */

#include <synfigapp/action.h>
#include <synfigapp/value_desc.h>
#include <synfig/valuenode_dynamiclist.h>

#include <list>
#include <set>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

namespace Action {

class ActivepointSetSmart :
	public Super
{
private:

	ValueDesc value_desc;
	synfig::ValueNode_DynamicList::Handle value_node;
	int index;
	synfig::Activepoint activepoint;
	bool time_set;

	void calc_activepoint();
	void enclose_activepoint(const synfig::Activepoint& activepoint);

	std::set<synfig::Time> times;

public:

	ActivepointSetSmart();

	static ParamVocab get_param_vocab();
	static bool is_canidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
