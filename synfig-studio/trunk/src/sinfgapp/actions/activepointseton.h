/* === S I N F G =========================================================== */
/*!	\file activepointseton.h
**	\brief Template File
**
**	$Id: activepointseton.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_APP_ACTION_ACTIVEPOINTSETON_H
#define __SINFG_APP_ACTION_ACTIVEPOINTSETON_H

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

class ActivepointSetOn :
	public Super
{
private:

	ValueDesc value_desc;
	sinfg::ValueNode_DynamicList::Handle value_node;
	int index;
	sinfg::Activepoint activepoint;
	bool time_set;

	void calc_activepoint();

public:

	ActivepointSetOn();

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
