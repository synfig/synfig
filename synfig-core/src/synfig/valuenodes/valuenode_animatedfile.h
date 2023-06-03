/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animatedfile.h
**	\brief Header file for Valuenode_AnimatedFile.
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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
	typedef etl::handle<ValueNode_AnimatedFile> Handle;
	typedef etl::handle<const ValueNode_AnimatedFile> ConstHandle;

	static ValueNode_AnimatedFile* create(const ValueBase& x, Canvas::LooseHandle canvas=nullptr);
	virtual ~ValueNode_AnimatedFile();

	virtual ValueBase operator()(Time t) const override;

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

	String get_file_field(Time t, const String &field_name) const;

protected:
	LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i,ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual LinkableValueNode::Vocab get_children_vocab_vfunc() const override;

	virtual void get_values_vfunc(std::map<Time, ValueBase> &x) const override;

	virtual void on_changed() override;
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
