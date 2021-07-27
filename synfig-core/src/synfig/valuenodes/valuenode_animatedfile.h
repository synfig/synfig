/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animatedfile.h
**	\brief Header file for Valuenode_AnimatedFile.
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#ifndef __SYNFIG_VALUENODE_ANIMATEDFILE_H
#define __SYNFIG_VALUENODE_ANIMATEDFILE_H

/* === H E A D E R S ======================================================= */

#include <synfig/canvas.h>

#include <synfig/valuenodes/valuenode_animatedinterface.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/** \class ValueNode_AnimatedFile */
class ValueNode_AnimatedFile : public LinkableValueNode, public ValueNode_AnimatedInterfaceConst
{
public:
	typedef std::shared_ptr<ValueNode_AnimatedFile> Handle;
	typedef std::shared_ptr<const ValueNode_AnimatedFile> ConstHandle;

private:
	class Internal;

	Internal *internal;

	String current_filename;
	ValueNode::RHandle filename;
	std::map<String, String> filefields;

	explicit ValueNode_AnimatedFile(Type &t);

	void load_file(const String &filename, bool force = false);
	void file_changed();

public:
	~ValueNode_AnimatedFile();

	virtual String get_name()const;
	virtual String get_local_name()const;

	using synfig::LinkableValueNode::get_link_vfunc;
	using synfig::LinkableValueNode::set_link_vfunc;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const;

	static bool check_type(Type &type);
	static ValueNode_AnimatedFile* create(const ValueBase &x);
	virtual Vocab get_children_vocab_vfunc() const;

	virtual ValueBase operator()(Time t) const;
	virtual void get_values_vfunc(std::map<Time, ValueBase> &x) const;

	String get_file_field(Time t, const String &field_name) const;

protected:
	LinkableValueNode* create_new() const;

	virtual void on_changed();
	virtual bool set_link_vfunc(int i, ValueNode::Handle x);
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
