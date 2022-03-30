/* === S Y N F I G ========================================================= */
/*!	\file node.cpp
**	\brief Test Node class
**
**	\legal
**	Copyright (c) 2022 Synfig contributors
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

#include <synfig/node.h>

#include "test_base.h"

using namespace etl;
using namespace synfig;

struct NodeX : public Node
{
	NodeX() : Node() {}
	
	String get_string() const override
	{
		return "NodeX";
	}

	// used for testing get_times()
	time_set x_times;
	
protected:
	void get_times_vfunc(time_set &set) const override
	{
		set = x_times;
	}
};

void initial_node_guid_is_not_zero() {
	NodeX node;

	ASSERT_NOT_EQUAL(GUID::zero(), node.get_guid());
}

void non_initialized_nodes_have_different_guid() {
	NodeX node1, node2;

	ASSERT(node1.get_guid() != node2.get_guid());
}

void set_node_guid_works() {
	NodeX node;
	
	const GUID initial_guid = node.get_guid();
	GUID new_guid;

	ASSERT(new_guid != initial_guid);
	
	node.set_guid(new_guid);

	ASSERT(node.get_guid() == new_guid);
}

void set_node_guid_first_time_does_not_emit_signal_guid_changed() {
	NodeX node;

	GUID new_guid;

	ASSERT_SIGNAL_NOT_EMITTED((&node),signal_guid_changed,,GUID,void, node.set_guid(new_guid));
}

void set_node_guid_emits_signal_guid_changed() {
	NodeX node;

	GUID new_guid1, new_guid2;

	node.set_guid(new_guid1);

	ASSERT_SIGNAL_EMITTED((&node),signal_guid_changed,,GUID,void, node.set_guid(new_guid2));
}

void set_same_node_guid_does_not_emit_signal_guid_changed() {
	NodeX node;

	GUID new_guid;
	node.set_guid(new_guid);

	ASSERT_SIGNAL_NOT_EMITTED((&node),signal_guid_changed,,GUID,void, node.set_guid(new_guid));
}

void set_node_guid_does_not_emit_signal_changed() {
	NodeX node;
	
	GUID new_guid;

	ASSERT_SIGNAL_NOT_EMITTED((&node),signal_changed,,,void, node.set_guid(new_guid));
}

void find_node_works() {
	NodeX node;
	
	ASSERT(&node == find_node(node.get_guid()));
}

void find_node_works_when_node_have_new_guid() {
	NodeX node;
	
	GUID new_guid;
	node.set_guid(new_guid);
	
	ASSERT(&node == find_node(new_guid));
}

void find_node_returns_null_on_not_found() {
	NodeX node;
	
	ASSERT_EQUAL(nullptr, find_node(GUID::zero()));
	
	GUID not_used_guid;

	ASSERT_EQUAL(nullptr, find_node(not_used_guid));
}

void deleting_node_emits_signal() {
	NodeX *node = new NodeX();
	
	ASSERT_SIGNAL_EMITTED(node,signal_deleted,,,void, delete node);
}

void deleting_node_does_not_emit_signal_changed() {
	NodeX *node = new NodeX();
	
	ASSERT_SIGNAL_NOT_EMITTED(node,signal_changed,,,void, delete node);
}

void initial_node_has_no_parent() {
	NodeX node;
	
	ASSERT_EQUAL(0, node.parent_count());
}

void adding_child_node_increases_its_parent_count() {
	NodeX parent_node1, parent_node2, child_node;
	
	parent_node1.add_child(&child_node);
	
	ASSERT_EQUAL(1, child_node.parent_count());

	parent_node2.add_child(&child_node);
	
	ASSERT_EQUAL(2, child_node.parent_count());
}

void adding_child_node_adds_itself_as_parent_to_child() {
	NodeX parent_node, child_node;
	
	parent_node.add_child(&child_node);
	
	ASSERT_EQUAL(1, child_node.parent_set.count(&parent_node));
}

void adding_child_node_does_not_increase_its_parent_count_if_already_its_parent() {
	NodeX parent_node, child_node;
	
	parent_node.add_child(&child_node);

	parent_node.add_child(&child_node);
	
	ASSERT_EQUAL(1, child_node.parent_count());
	ASSERT_EQUAL(1, child_node.parent_set.count(&parent_node));
}

void removing_child_node_decreases_its_parent_count() {
	NodeX parent_node1, parent_node2, child_node;
	
	parent_node1.add_child(&child_node);

	parent_node1.remove_child(&child_node);

	ASSERT_EQUAL(0, child_node.parent_count());

	parent_node1.add_child(&child_node);
	parent_node2.add_child(&child_node);

	parent_node1.remove_child(&child_node);

	ASSERT_EQUAL(1, child_node.parent_count());

	parent_node2.remove_child(&child_node);

	ASSERT_EQUAL(0, child_node.parent_count());
}

void removing_child_node_removes_itself_as_parent_from_child() {
	NodeX parent_node, child_node;
	
	parent_node.add_child(&child_node);

	parent_node.remove_child(&child_node);

	ASSERT_EQUAL(0, child_node.parent_set.count(&parent_node));
}

void removing_child_node_does_not_decrease_its_parent_count_if_not_its_parent() {
	NodeX parent_node, child_node;
	
	parent_node.remove_child(&child_node);

	ASSERT_EQUAL(0, child_node.parent_count());
}

void deleting_node_removes_it_as_parent_from_its_children() {
	NodeX *parent_node = new NodeX();
	
	NodeX child_node;
	
	parent_node->add_child(&child_node);
	
	ASSERT_EQUAL(1, child_node.parent_count());

	delete parent_node;

	ASSERT_EQUAL(0, child_node.parent_count());
	ASSERT_EQUAL(0, child_node.parent_set.count(parent_node));
}

void marking_node_as_changed_changes_the_last_time_changed() {
	NodeX node;

	auto last_time = node.get_time_last_changed();
	node.changed();

	ASSERT(last_time < node.get_time_last_changed());
}

void marking_node_as_changed_emits_signal_changed() {
	NodeX node;

	ASSERT_SIGNAL_EMITTED((&node),signal_changed,,,void, node.changed());
}

void marking_child_node_as_changed_emits_signal_changed() {
	NodeX parent_node, child_node;

	parent_node.add_child(&child_node);

	ASSERT_SIGNAL_EMITTED((&parent_node),signal_changed,,,void, child_node.changed());
}

void marking_child_node_as_changed_emits_signal_child_changed() {
	NodeX parent_node, child_node;

	parent_node.add_child(&child_node);

	ASSERT_SIGNAL_EMITTED((&parent_node),signal_child_changed,,const Node*,void, child_node.changed());
}

void get_times_is_cached() {
	NodeX node;

	node.x_times.insert(TimePoint(Time(3)));

	ASSERT_EQUAL(1, node.get_times().size());

	node.x_times.insert(TimePoint(Time(4)));

	ASSERT_EQUAL(1, node.get_times().size());
}

void marking_node_as_changed_updates_times_cache() {
	NodeX node;

	node.x_times.insert(TimePoint(Time(3)));

	ASSERT_EQUAL(1, node.get_times().size());

	node.x_times.insert(TimePoint(Time(4)));

	node.changed();
	ASSERT_EQUAL(2, node.get_times().size());
}

int main() {

	TEST_SUITE_BEGIN()
		TEST_FUNCTION(initial_node_guid_is_not_zero);
		TEST_FUNCTION(non_initialized_nodes_have_different_guid);
		TEST_FUNCTION(set_node_guid_works);
		TEST_FUNCTION(set_node_guid_first_time_does_not_emit_signal_guid_changed);
		TEST_FUNCTION(set_node_guid_emits_signal_guid_changed);
		TEST_FUNCTION(set_same_node_guid_does_not_emit_signal_guid_changed);
		TEST_FUNCTION(set_node_guid_does_not_emit_signal_changed);

		TEST_FUNCTION(find_node_works);
		TEST_FUNCTION(find_node_works_when_node_have_new_guid);
		TEST_FUNCTION(find_node_returns_null_on_not_found);

		TEST_FUNCTION(deleting_node_emits_signal);
		TEST_FUNCTION(deleting_node_does_not_emit_signal_changed);

		TEST_FUNCTION(initial_node_has_no_parent);
		TEST_FUNCTION(adding_child_node_increases_its_parent_count);
		TEST_FUNCTION(adding_child_node_adds_itself_as_parent_to_child);
		TEST_FUNCTION(adding_child_node_does_not_increase_its_parent_count_if_already_its_parent);
		TEST_FUNCTION(removing_child_node_decreases_its_parent_count);
		TEST_FUNCTION(removing_child_node_removes_itself_as_parent_from_child);
		TEST_FUNCTION(removing_child_node_does_not_decrease_its_parent_count_if_not_its_parent);
		TEST_FUNCTION(deleting_node_removes_it_as_parent_from_its_children);

		TEST_FUNCTION(marking_node_as_changed_changes_the_last_time_changed);
		TEST_FUNCTION(marking_node_as_changed_emits_signal_changed);
		TEST_FUNCTION(marking_child_node_as_changed_emits_signal_changed);
		TEST_FUNCTION(marking_child_node_as_changed_emits_signal_child_changed);

		TEST_FUNCTION(get_times_is_cached);
		TEST_FUNCTION(marking_node_as_changed_updates_times_cache);
	TEST_SUITE_END()

	return tst_exit_status;
}
