/* === S Y N F I G ========================================================= */
/*!	\file action_groupaddlayers.h
**	\brief Template File
**
**	$Id: groupremove.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SYNFIG_APP_ACTION_GROUPREMOVE_H
#define __SYNFIG_APP_ACTION_GROUPREMOVE_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfigapp/action.h>
#include <set>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Instance;

namespace Action {

class GroupRemove :
	public Undoable,
	public CanvasSpecific
{
private:

	// List contains the layers
	std::set<synfig::Layer::Handle> layer_list;
	
	// The name of the group to remove
	synfig::String group;

public:

	GroupRemove();

	static ParamVocab get_param_vocab();
	static bool is_canidate(const ParamList &x);

	virtual bool set_param(const synfig::String& name, const Param &);
	virtual bool is_ready()const;

	virtual void perform();
	virtual void undo();

	ACTION_MODULE_EXT
};

}; // END of namespace action
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
