/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animsharelist.cpp
**	\brief Implementation of the "Anim Share List" valuenode conversion.
**
**	This file is part of Synfig.
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "valuenode_animsharelist.h"
#include "valuenode_composite.h"
#include <synfig/animshare.h>
#include <synfig/exception.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>

#include <list>
#include <vector>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_AnimShareList, RELEASE_VERSION_1_4_0,
                   "animsharelist", N_("AnimShareList"))

/* === M E T H O D S ======================================================= */

ValueNode_AnimShareList::ValueNode_AnimShareList()
    : ValueNode_DynamicList(type_anim_share) {}

ValueNode_AnimShareList::~ValueNode_AnimShareList() {}

ValueNode_AnimShareList *
ValueNode_AnimShareList::create(const ValueBase &value,
                                etl::loose_handle<Canvas>) {
  // if the parameter is not a list type, return null
  if (value.get_type() != type_list)
    return nullptr;

  std::vector<ValueBase> list = value.get_list();

  synfig::info(">>> AnimShareList input count=%d", (int)list.size());

  for (const auto &item : list) {
    synfig::info(">>> AnimShareList item type_anim_share=%d",
                 item.get_type() == type_anim_share);
  }

  ValueNode_AnimShareList *value_node(new ValueNode_AnimShareList());

  if (!value.empty()) {
    Type &type(value.get_contained_type());
    if (type == type_anim_share) {
      std::vector<AnimShare> list(value.get_list_of(AnimShare()));
      std::vector<AnimShare>::const_iterator iter;

      for (iter = list.begin(); iter != list.end(); iter++) {
        value_node->add(ValueNode::Handle(ValueNode_Composite::create(*iter)));
      }
      value_node->set_loop(value.get_loop());
    } else {
      // We got a list of who-knows-what. We don't have any idea
      // what to do with it.
      return nullptr;
    }
  }

  return value_node;
}

ValueNode_AnimShareList::ListEntry
ValueNode_AnimShareList::create_list_entry(int index, Time time,
                                           Real /*origin*/) {
  ValueNode_AnimShareList::ListEntry ret;
  synfig::AnimShare inserted;
  int new_index;
  if (link_count()) {
    new_index = find_prev_valid_entry(index, time);
    ret.index = new_index;
  } else {
    ret.index = index;
  }
  ret.set_parent_value_node(this);
  ret.value_node = ValueNode_Composite::create(inserted);
  ret.value_node->set_parent_canvas(get_parent_canvas());
  return ret;
}

ValueBase ValueNode_AnimShareList::operator()(Time t) const {
  DEBUG_LOG("SYNFIG_DEBUG_VALUENODE_OPERATORS", "%s:%d operator()\n", __FILE__,
            __LINE__);

  std::vector<AnimShare> ret_list;

  std::vector<ListEntry>::const_iterator iter;
  bool rising;

  AnimShare curr;

  for (iter = list.begin(); iter != list.end(); ++iter) {

    float amount(iter->amount_at_time(t, &rising));
    assert(amount >= 0.0f);
    assert(amount <= 1.0f);

    curr = (*iter->value_node)(t).get(curr);

    if (amount > 1.0f - 0.0000001f)
      ret_list.push_back(curr);
  }

  return ValueBase(ret_list, get_loop());
}

String ValueNode_AnimShareList::link_local_name(int i) const {
  assert(i >= 0 && (unsigned)i < list.size());
  return strprintf(_("AnimShare %03d"), i + 1);
}

LinkableValueNode *ValueNode_AnimShareList::create_new() const {
  return new ValueNode_AnimShareList();
}

bool ValueNode_AnimShareList::check_type(Type &type) {
  return type == type_list;
}
void ValueNode_AnimShareList::clear() {
  while (!list.empty()) {
    ValueNode::Handle value_node = list.front().value_node;

    if (!value_node) {
      list.erase(list.begin());
      continue;
    }

    erase(value_node);
  }
}
