/* === S I N F G =========================================================== */
/*!	\file valuenodedynamiclistinsert.h
**	\brief Template File
**
**	$Id: valuenodedynamiclistinsert.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_APP_ACTION_VALUENODEDYNAMICLISTINSERT_H
#define __SINFG_APP_ACTION_VALUENODEDYNAMICLISTINSERT_H

/* === H E A D E R S ======================================================= */

#include <sinfgapp/action.h>
#include <sinfg/activepoint.h>
#include <sinfg/valuenode_dynamiclist.h>
#include <sinfg/valuenode_bline.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class Instance;

namespace Action {

class ValueNodeDynamicListInsert :
	public Undoable,
	public CanvasSpecific
{
private:

	sinfg::ValueNode_DynamicList::Handle value_node;
	sinfg::ValueNode_BLine::Handle value_node_bline;
	sinfg::ValueNode_DynamicList::ListEntry list_entry;
	sinfg::ValueNode::Handle item;
	sinfg::Time time;
	sinfg::Real origin;
	int index;


public:

	ValueNodeDynamicListInsert();

	static ParamVocab get_param_vocab();
	static bool is_canidate(const ParamList &x);

	virtual bool set_param(const sinfg::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void perform();
	virtual void undo();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
