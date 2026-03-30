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
#include <thread>

#include <gtest/gtest.h>

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

TEST(NodeTest, initial_node_guid_is_not_zero) {
	NodeX node;

	EXPECT_NE(GUID::zero(), node.get_guid());
}

TEST(NodeTest, non_initialized_nodes_have_different_guid) {
	NodeX node1, node2;

	EXPECT_TRUE(node1.get_guid() != node2.get_guid());
}

TEST(NodeTest, set_node_guid_works) {
	NodeX node;
	
	const GUID initial_guid = node.get_guid();
	GUID new_guid;

	EXPECT_TRUE(new_guid != initial_guid);
	
	node.set_guid(new_guid);

	EXPECT_TRUE(node.get_guid() == new_guid);
}

TEST(NodeTest, set_node_guid_first_time_does_not_emit_signal_guid_changed) {
	NodeX node;

	GUID new_guid;

	{
		bool signal_emitted = false;
		auto conn = (&node)->signal_guid_changed().connect([&](GUID) { signal_emitted = true; });
		node.set_guid(new_guid);
		EXPECT_FALSE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, set_node_guid_emits_signal_guid_changed) {
	NodeX node;

	GUID new_guid1, new_guid2;

	node.set_guid(new_guid1);

	{
		bool signal_emitted = false;
		auto conn = (&node)->signal_guid_changed().connect([&](GUID) { signal_emitted = true; });
		node.set_guid(new_guid2);
		EXPECT_TRUE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, set_same_node_guid_does_not_emit_signal_guid_changed) {
	NodeX node;

	GUID new_guid;
	node.set_guid(new_guid);

	{
		bool signal_emitted = false;
		auto conn = (&node)->signal_guid_changed().connect([&](GUID) { signal_emitted = true; });
		node.set_guid(new_guid);
		EXPECT_FALSE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, set_node_guid_does_not_emit_signal_changed) {
	NodeX node;
	
	GUID new_guid;

	{
		bool signal_emitted = false;
		auto conn = (&node)->signal_changed().connect([&]() { signal_emitted = true; });
		node.set_guid(new_guid);
		EXPECT_FALSE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, find_node_works) {
	NodeX node;
	
	EXPECT_TRUE(&node == find_node(node.get_guid()));
}

TEST(NodeTest, find_node_works_when_node_have_new_guid) {
	NodeX node;
	
	GUID new_guid;
	node.set_guid(new_guid);
	
	EXPECT_TRUE(&node == find_node(new_guid));
}

TEST(NodeTest, find_node_returns_null_on_not_found) {
	NodeX node;
	
	EXPECT_EQ(nullptr, find_node(GUID::zero()));
	
	GUID not_used_guid;

	EXPECT_EQ(nullptr, find_node(not_used_guid));
}

TEST(NodeTest, deleting_node_emits_signal) {
	NodeX *node = new NodeX();
	
	{
		bool signal_emitted = false;
		auto conn = node->signal_deleted().connect([&]() { signal_emitted = true; });
		delete node;
		EXPECT_TRUE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, deleting_node_does_not_emit_signal_changed) {
	NodeX *node = new NodeX();
	
	{
		bool signal_emitted = false;
		auto conn = node->signal_changed().connect([&]() { signal_emitted = true; });
		delete node;
		EXPECT_FALSE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, initial_node_has_no_parent) {
	NodeX node;
	
	EXPECT_EQ(0, node.parent_count());
}

TEST(NodeTest, adding_child_node_increases_its_parent_count) {
	NodeX parent_node1, parent_node2, child_node;
	
	parent_node1.add_child(&child_node);
	
	EXPECT_EQ(1, child_node.parent_count());

	parent_node2.add_child(&child_node);
	
	EXPECT_EQ(2, child_node.parent_count());
}

TEST(NodeTest, adding_child_node_adds_itself_as_parent_to_child) {
	NodeX parent_node, child_node;
	
	parent_node.add_child(&child_node);
	
	EXPECT_TRUE(child_node.is_child_of(&parent_node));
}

TEST(NodeTest, adding_child_node_does_not_increase_its_parent_count_if_already_its_parent) {
	NodeX parent_node, child_node;
	
	parent_node.add_child(&child_node);

	parent_node.add_child(&child_node);
	
	EXPECT_EQ(1, child_node.parent_count());
	EXPECT_TRUE(child_node.is_child_of(&parent_node));
}

TEST(NodeTest, removing_child_node_decreases_its_parent_count) {
	NodeX parent_node1, parent_node2, child_node;
	
	parent_node1.add_child(&child_node);

	parent_node1.remove_child(&child_node);

	EXPECT_EQ(0, child_node.parent_count());

	parent_node1.add_child(&child_node);
	parent_node2.add_child(&child_node);

	parent_node1.remove_child(&child_node);

	EXPECT_EQ(1, child_node.parent_count());

	parent_node2.remove_child(&child_node);

	EXPECT_EQ(0, child_node.parent_count());
}

TEST(NodeTest, removing_child_node_removes_itself_as_parent_from_child) {
	NodeX parent_node, child_node;
	
	parent_node.add_child(&child_node);

	parent_node.remove_child(&child_node);

	EXPECT_FALSE(child_node.is_child_of(&parent_node));
}

TEST(NodeTest, removing_child_node_does_not_decrease_its_parent_count_if_not_its_parent) {
	NodeX parent_node, child_node;
	
	parent_node.remove_child(&child_node);

	EXPECT_EQ(0, child_node.parent_count());
}

TEST(NodeTest, deleting_node_removes_it_as_parent_from_its_children) {
	NodeX *parent_node = new NodeX();
	
	NodeX child_node;
	
	parent_node->add_child(&child_node);
	
	EXPECT_EQ(1, child_node.parent_count());

	delete parent_node;

	EXPECT_EQ(0, child_node.parent_count());
	EXPECT_FALSE(child_node.is_child_of(parent_node));
}

TEST(NodeTest, marking_node_as_changed_changes_the_last_time_changed) {
	NodeX node;

	auto last_time = node.get_time_last_changed();
	// Issue if less than 1ms
	// https://developercommunity.visualstudio.com/t/stdthis-threadsleep-forstdchronomicroseconds999/1351851
	std::this_thread::sleep_for(std::chrono::microseconds(1000));
	node.changed();

	EXPECT_TRUE(last_time < node.get_time_last_changed());
}

TEST(NodeTest, marking_node_as_changed_emits_signal_changed) {
	NodeX node;

	{
		bool signal_emitted = false;
		auto conn = (&node)->signal_changed().connect([&]() { signal_emitted = true; });
		node.changed();
		EXPECT_TRUE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, marking_child_node_as_changed_emits_signal_changed) {
	NodeX parent_node, child_node;

	parent_node.add_child(&child_node);

	{
		bool signal_emitted = false;
		auto conn = (&parent_node)->signal_changed().connect([&]() { signal_emitted = true; });
		child_node.changed();
		EXPECT_TRUE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, marking_child_node_as_changed_emits_signal_child_changed) {
	NodeX parent_node, child_node;

	parent_node.add_child(&child_node);

	{
		bool signal_emitted = false;
		auto conn = (&parent_node)->signal_child_changed().connect([&](const Node*) { signal_emitted = true; });
		child_node.changed();
		EXPECT_TRUE(signal_emitted);
		conn.disconnect();
	}
}

TEST(NodeTest, get_times_is_cached) {
	NodeX node;

	node.x_times.insert(TimePoint(Time(3)));

	EXPECT_EQ(1, node.get_times().size());

	node.x_times.insert(TimePoint(Time(4)));

	EXPECT_EQ(1, node.get_times().size());
}

TEST(NodeTest, marking_node_as_changed_updates_times_cache) {
	NodeX node;

	node.x_times.insert(TimePoint(Time(3)));

	EXPECT_EQ(1, node.get_times().size());

	node.x_times.insert(TimePoint(Time(4)));

	node.changed();
	EXPECT_EQ(2, node.get_times().size());
}
