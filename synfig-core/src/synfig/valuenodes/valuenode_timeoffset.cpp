#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valuenode_timeoffset.h"
#include <synfig/valuenode.h>
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#endif

using namespace synfig;

REGISTER_VALUENODE(ValueNode_TimeOffset, RELEASE_VERSION_CURRENT, "timeoffset", N_("Time Offset"))
// TODO: Replace RELEASE_VERSION_CURRENT with the release this node is introduced in.

ValueNode_TimeOffset::ValueNode_TimeOffset(Type &x):
	LinkableValueNode(x)
{
	init_children_vocab();
	set_link("link",   ValueNode_Const::create(x));
	set_link("offset", ValueNode_Const::create(Time(0)));
}

ValueNode_TimeOffset::ValueNode_TimeOffset(const ValueNode::Handle &x):
    LinkableValueNode(x->get_type())
{
    init_children_vocab();
    set_link("link",   x);
    set_link("offset", ValueNode_Const::create(Time(0)));
    if (!link_ || !offset_){
    	synfig::error(
        	"TimeOffset missing children: link=%p offset=%p",
        	link_.get(),
        	offset_.get());
	}
}

ValueNode_TimeOffset*
ValueNode_TimeOffset::create(const ValueBase& x, etl::loose_handle<Canvas>)
{
    return new ValueNode_TimeOffset(ValueNode_Const::create(x));   // now matches
}

ValueNode_TimeOffset*
ValueNode_TimeOffset::create_with_offset(const ValueNode::Handle &link, const Time &offset)
{
    ValueNode_TimeOffset *node = new ValueNode_TimeOffset(link);   // default offset = 0
    node->set_link("offset", ValueNode_Const::create(offset));    // then override it
    return node;
}

LinkableValueNode*
ValueNode_TimeOffset::create_new()const
{
	return new ValueNode_TimeOffset(get_type());
}

ValueNode_TimeOffset::~ValueNode_TimeOffset()
{
	unlink_all();
}

bool
ValueNode_TimeOffset::set_link_vfunc(int i, ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,   get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(offset_, type_time);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_TimeOffset::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	if(i==0) return link_;
	if(i==1) return offset_;

	return 0;
}

ValueBase
ValueNode_TimeOffset::operator()(Time t)const
{
	DEBUG_LOG("SYNFIG_DEBUG_VALUENODE_OPERATORS",
		"%s:%d operator()\n", __FILE__, __LINE__);
	if (!link_)
		return ValueBase();

	Time offset = offset_ ? (*offset_)(t).get(Time()) : Time(0);
	return (*link_)(t - offset);
}

bool
ValueNode_TimeOffset::check_type(Type &type)
{
	if(type != type_nil)
		return true;
	return false;
}

LinkableValueNode::Vocab
ValueNode_TimeOffset::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;

	ret.push_back(ParamDesc("link")
		.set_local_name(_("Link"))
		.set_description(_("The value node to time-shift"))
	);

	ret.push_back(ParamDesc("offset")
		.set_local_name(_("Offset"))
		.set_description(_("Amount of time added before evaluating Link"))
	);
	return ret;
}

// TODO: Inverse evaluation currently ignores the stored time offset.
// This is sufficient for evaluation, but interactive editing of
// time-shifted values would require mapping edits back through the
// offset before writing to the wrapped node.
LinkableValueNode::InvertibleStatus
ValueNode_TimeOffset::is_invertible(const Time& /*t*/, const ValueBase& target_value, int* link_index) const
{
	if (target_value.get_type() != get_link("link")->get_type())
		return INVERSE_ERROR_BAD_TYPE;
	if (link_index)
		*link_index = get_link_index_from_name("link");
	return INVERSE_OK;
}

ValueBase
ValueNode_TimeOffset::get_inverse(const Time& /*t*/, const ValueBase& target_value) const
{
	return target_value;
}
