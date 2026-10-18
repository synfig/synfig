#ifndef SYNFIG_VALUENODE_TIMEOFFSET_H
#define SYNFIG_VALUENODE_TIMEOFFSET_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ValueNode_TimeOffset : public LinkableValueNode
{
	ValueNode::RHandle link_;
	ValueNode::RHandle offset_;

	ValueNode_TimeOffset(Type &x);
	ValueNode_TimeOffset(const ValueNode::Handle &x);
	
public:
	typedef etl::handle<ValueNode_TimeOffset> Handle;
	typedef etl::handle<const ValueNode_TimeOffset> ConstHandle;

	static ValueNode_TimeOffset* create(const ValueBase& x, etl::loose_handle<Canvas> canvas=nullptr);
	//! convenience constructor used by attach_shared_entries()
	static ValueNode_TimeOffset* create_with_offset(const ValueNode::Handle &link, const Time &offset);
	virtual ~ValueNode_TimeOffset();

	virtual String get_name() const override;
	virtual String get_local_name() const override;
	static bool check_type(Type &type);

	virtual ValueBase operator()(Time t) const override;

	virtual InvertibleStatus is_invertible(const Time& t, const ValueBase& target_value, int* link_index = nullptr) const override;
	virtual ValueBase get_inverse(const Time& t, const synfig::ValueBase &target_value) const override;

protected:
	LinkableValueNode* create_new() const override;

	virtual bool set_link_vfunc(int i, ValueNode::Handle x) override;
	virtual ValueNode::LooseHandle get_link_vfunc(int i) const override;

	virtual Vocab get_children_vocab_vfunc() const override;
}; // END of class ValueNode_TimeOffset

}; // END of namespace synfig

#endif
