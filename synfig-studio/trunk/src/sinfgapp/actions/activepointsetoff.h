/* === S I N F G =========================================================== */
/*!	\file activepointsetoff.h
**	\brief Template File
**
**	$Id: activepointsetoff.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_APP_ACTION_ACTIVEPOINTSETOFF_H
#define __SINFG_APP_ACTION_ACTIVEPOINTSETOFF_H

/* === H E A D E R S ======================================================= */

#include <sinfgapp/action.h>
#include <sinfgapp/value_desc.h>
#include <sinfg/valuenode_dynamiclist.h>

#include <list>
#include <set>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

namespace Action {

class ActivepointSetOff :
	public Super
{
private:

	ValueDesc value_desc;
	sinfg::ValueNode_DynamicList::Handle value_node;
	int index;
	bool time_set;
	sinfg::Activepoint activepoint;

	void calc_activepoint();

public:

	ActivepointSetOff();

	static ParamVocab get_param_vocab();
	static bool is_canidate(const ParamList &x);

	virtual bool set_param(const sinfg::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void prepare();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
