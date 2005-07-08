/* === S Y N F I G ========================================================= */
/*!	\file valuelink.h
**	\brief ValueBase Link Header
**
**	$Id: valuelink.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2004 Adrian Bentley
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

#ifndef __SYNFIG_VALUELINK_H
#define __SYNFIG_VALUELINK_H

/* === H E A D E R S ======================================================= */
#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

/* NOTE: DO NOT USE THE INDEX BASED INTERFACE... THINGS WILL CHANGE
*/
class ValueBaseLink : public synfig::LinkableValueNode
{
	typedef std::vector<ValueNode::Handle> list_type;
	list_type	list;

protected:
	//stuff I don't want
	virtual bool set_link_vfunc(int i,synfig::ValueNode::Handle x) {return false;}
	virtual LinkableValueNode* create_new()const {return 0;}
	
	//new stuff I need
	list_type::const_iterator findlink(synfig::ValueNode::Handle x) const;
	list_type::iterator findlink(synfig::ValueNode::Handle x);
	
public: //linkable interface
	
	//stuff I do want	
	virtual synfig::ValueNode::LooseHandle get_link_vfunc(int i)const;
	virtual int link_count()const;
	
	//I have to support the thing because it's too much work otherwise
	virtual synfig::String link_local_name(int i)const;
	virtual synfig::String link_name(int i)const;

public:
	ValueBaseLink();
	virtual ~ValueBaseLink();

	//don't want
	virtual int get_link_index_from_name(const synfig::String &name)const;

	//new add and subtract stuff
	virtual void add(synfig::ValueNode::Handle v);
	virtual void remove(synfig::ValueNode::Handle v);

};
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
