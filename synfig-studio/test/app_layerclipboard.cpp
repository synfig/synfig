// SPDX-License-Identifier: GPL-2.0-or-later
#include "test_base.h"
#include <synfig/canvas.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/valuenodes/valuenode_bonelink.h>
#include <synfig/valuenodes/valuenode_staticlist.h>
#include <synfigapp/main.h>
#include <synfigapp/layerclipboard.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/instance.h>
#include <list>

using namespace synfig;

struct SkeletonFixture
{
	Canvas::Handle canvas = Canvas::create();
	Layer::Handle skeleton = Layer::create("skeleton");
	Layer::Handle follower = Layer::create("circle");
	ValueNode_StaticList::Handle bones;
	ValueNode_Bone::Handle root, child, tip;

	explicit SkeletonFixture(Canvas::Handle owner = Canvas::create())
	{
		canvas = owner;
		std::vector<Bone> values(3);
		for (size_t i = 0; i < values.size(); ++i) {
			values[i].set_name("clipboard bone " + std::to_string(i));
			values[i].set_parent(ValueNode_Bone_Root::create(Bone()));
		}
		ValueBase value;
		value.set_list_of(values);
		skeleton->set_param("bones", value);
		bones = ValueNode_StaticList::create(value, canvas);
		root = ValueNode_Bone::Handle::cast_dynamic(bones->get_link(0));
		child = ValueNode_Bone::Handle::cast_dynamic(bones->get_link(1));
		tip = ValueNode_Bone::Handle::cast_dynamic(bones->get_link(2));
		ASSERT(root && child && tip)
		// Set the actual node links, rather than a detached vector of Bone values.
		ASSERT(child->set_link("parent", ValueNode_Const::create(root)))
		ASSERT(tip->set_link("parent", ValueNode_Const::create(child)))
		ASSERT(skeleton->connect_dynamic_param("bones", bones))
		canvas->push_back(skeleton);
		auto link = ValueNode_BoneLink::create(Vector(3, 5));
		ASSERT(link->set_link("bone", ValueNode_Const::create(tip)))
		ASSERT(follower->connect_dynamic_param("origin", link))
		canvas->push_back(follower);
	}
};

static std::list<Layer::Handle> copy_layers(const std::list<Layer::Handle>& source, Canvas::Handle canvas)
{
	GUID guid;
	return synfigapp::clone_layers_for_clipboard(source, canvas, guid);
}

static ValueNode_Bone::Handle bone_at(Layer::Handle skeleton, int index)
{
	auto list = ValueNode_StaticList::Handle::cast_dynamic(skeleton->dynamic_param_list().at("bones"));
	return ValueNode_Bone::Handle::cast_dynamic(list->get_link(index));
}

static ValueNode_Bone::Handle parent_of(ValueNode_Bone::Handle bone)
{
	return (*bone->get_link("parent"))(0).get(ValueNode_Bone::Handle());
}

static void test_clipboard_keeps_cloned_parent_chain()
{
	SkeletonFixture source;
	auto clipboard = copy_layers({source.skeleton, source.follower}, nullptr);
	auto root = bone_at(clipboard.front(), 0);
	auto child = bone_at(clipboard.front(), 1);
	auto tip = bone_at(clipboard.front(), 2);
	ASSERT(root != source.root && child != source.child && tip != source.tip)
	ASSERT(parent_of(child) == root)
	ASSERT(parent_of(tip) == child)
	ASSERT(parent_of(source.child) == source.root)
	ASSERT(parent_of(source.tip) == source.child)
}

static void test_paste_keeps_follower_on_pasted_skeleton()
{
	SkeletonFixture source;
	auto clipboard = copy_layers({source.skeleton, source.follower}, nullptr);
	auto destination = Canvas::create();
	auto pasted = copy_layers(clipboard, destination);
	for (const auto& layer : pasted)
		destination->push_back(layer);
	auto root = bone_at(pasted.front(), 0);
	auto child = bone_at(pasted.front(), 1);
	auto tip = bone_at(pasted.front(), 2);
	auto link = ValueNode_BoneLink::Handle::cast_dynamic(pasted.back()->dynamic_param_list().at("origin"));
	ASSERT((*link->get_link("bone"))(0).get(ValueNode_Bone::Handle()) == tip)
	ASSERT(parent_of(child) == root)
	ASSERT(parent_of(tip) == child)
	ASSERT(root != source.root && root != bone_at(clipboard.front(), 0))
	ASSERT(parent_of(source.child) == source.root)
}

static void test_unselected_bones_and_exported_nodes_remain_shared()
{
	SkeletonFixture source;
	auto follower_only = copy_layers({source.follower}, nullptr);
	auto link = ValueNode_BoneLink::Handle::cast_dynamic(follower_only.front()->dynamic_param_list().at("origin"));
	ASSERT((*link->get_link("bone"))(0).get(ValueNode_Bone::Handle()) == source.tip)
	auto original_link = source.follower->dynamic_param_list().at("origin");
	source.canvas->add_value_node(original_link, "shared follower");
	auto shared = copy_layers({source.skeleton, source.follower}, nullptr);
	ASSERT(shared.back()->dynamic_param_list().at("origin") == original_link)
	ASSERT((*ValueNode_BoneLink::Handle::cast_dynamic(original_link)->get_link("bone"))(0).get(ValueNode_Bone::Handle()) == source.tip)
	ASSERT(parent_of(source.tip) == source.child)
}

static void test_animated_follower_and_repeated_pastes()
{
	SkeletonFixture source;
	auto animated = ValueNode_Animated::create(type_bone_valuenode);
	animated->new_waypoint(0.0, ValueBase(source.root));
	animated->new_waypoint(1.0, ValueBase(source.tip));
	auto original_link = ValueNode_BoneLink::Handle::cast_dynamic(source.follower->dynamic_param_list().at("origin"));
	ASSERT(original_link->set_link("bone", animated))
	auto clipboard = copy_layers({source.follower, source.skeleton}, nullptr);
	auto destination = Canvas::create();
	auto first = copy_layers(clipboard, destination);
	auto second = copy_layers(clipboard, destination);
	for (const auto& pasted : {first, second}) {
		auto link = ValueNode_BoneLink::Handle::cast_dynamic(pasted.front()->dynamic_param_list().at("origin"));
		ASSERT((*link->get_link("bone"))(0.0).get(ValueNode_Bone::Handle()) == bone_at(pasted.back(), 0))
		ASSERT((*link->get_link("bone"))(1.0).get(ValueNode_Bone::Handle()) == bone_at(pasted.back(), 2))
	}
	ASSERT(bone_at(first.back(), 0) != bone_at(second.back(), 0))
	ASSERT((*animated)(0.0).get(ValueNode_Bone::Handle()) == source.root)
	ASSERT((*animated)(1.0).get(ValueNode_Bone::Handle()) == source.tip)
}

static void test_embedded_group_keeps_internal_skeleton_links()
{
	auto document = Canvas::create();
	SkeletonFixture source(Canvas::create_inline(document));
	auto group = Layer::create("group");
	ASSERT(group->set_param("canvas", source.canvas))
	document->push_back(group);
	auto clipboard = copy_layers({group}, nullptr);
	auto pasted = copy_layers(clipboard, Canvas::create());
	auto nested = pasted.front()->get_param("canvas").get(Canvas::Handle());
	ASSERT(nested && nested != source.canvas)
	ASSERT_EQUAL(2, nested->size())
	auto tip = bone_at(nested->front(), 2);
	auto link = ValueNode_BoneLink::Handle::cast_dynamic(nested->back()->dynamic_param_list().at("origin"));
	ASSERT((*link->get_link("bone"))(0).get(ValueNode_Bone::Handle()) == tip)
	ASSERT(parent_of(tip) == bone_at(nested->front(), 1))
	ASSERT(parent_of(source.tip) == source.child)
}

static void test_regular_layers_keep_values_and_selection_order()
{
	auto circle = Layer::create("circle");
	auto rectangle = Layer::create("rectangle");
	ASSERT(circle->set_param("radius", Real(5.5)))
	auto clipboard = copy_layers({rectangle, circle}, nullptr);
	auto pasted = copy_layers(clipboard, Canvas::create());
	ASSERT_EQUAL("rectangle", pasted.front()->get_name())
	ASSERT_EQUAL("circle", pasted.back()->get_name())
	ASSERT(circle->set_param("radius", Real(9.0)))
	ASSERT_APPROX_EQUAL(5.5, pasted.back()->get_param("radius").get(Real()))
	ASSERT(pasted.back() != circle && pasted.back() != clipboard.back())
}

static void test_embed_imported_canvas_preserves_bone_links()
{
	SkeletonFixture imported;
	imported.canvas->set_file_name("imported-skeleton.sif");
	auto document = Canvas::create();
	auto group = Layer::create("group");
	ASSERT(group->set_param("canvas", imported.canvas))
	document->push_back(group);
	auto instance = synfigapp::Instance::create(document, nullptr);
	auto embed = synfigapp::Action::create("LayerEmbed");
	ASSERT(embed->set_param("canvas", document))
	ASSERT(embed->set_param("canvas_interface", instance->find_canvas_interface(document)))
	ASSERT(embed->set_param("layer", group))
	ASSERT(instance->perform_action(embed))
	auto embedded = group->get_param("canvas").get(Canvas::Handle());
	ASSERT(embedded && embedded != imported.canvas)
	ASSERT_EQUAL(size_t(2), embedded->size())
	auto link = embedded->back()->dynamic_param_list().at("origin");
	auto tip = bone_at(embedded->front(), 2);
	ASSERT((*ValueNode_BoneLink::Handle::cast_dynamic(link)->get_link("bone"))(0).get(ValueNode_Bone::Handle()) == tip)
	ASSERT(parent_of(tip) == bone_at(embedded->front(), 1))
	ASSERT(instance->undo())
	ASSERT(group->get_param("canvas").get(Canvas::Handle()) == imported.canvas)
	ASSERT(instance->redo())
	ASSERT(group->get_param("canvas").get(Canvas::Handle()) == embedded)
	ASSERT(parent_of(imported.tip) == imported.child)
}

static void test_pasted_bones_move_follower_after_undo_and_redo()
{
	SkeletonFixture source;
	auto original_link = source.follower->dynamic_param_list().at("origin");
	Vector original_position = (*original_link)(0).get(Vector());
	auto clipboard = copy_layers({source.skeleton, source.follower}, nullptr);
	auto destination = Canvas::create();
	auto instance = synfigapp::Instance::create(destination, nullptr);
	auto pasted = copy_layers(clipboard, destination);
	{
		synfigapp::Action::PassiveGrouper group(instance.get(), "Paste");
		int depth = 0;
		for (const auto& layer : pasted) {
			auto add = synfigapp::Action::create("LayerAdd");
			ASSERT(add->set_param("canvas", destination))
			ASSERT(add->set_param("canvas_interface", instance->find_canvas_interface(destination)))
			ASSERT(add->set_param("new", layer))
			ASSERT(instance->perform_action(add))
			if (depth > 0) {
				auto move = synfigapp::Action::create("LayerMove");
				ASSERT(move->set_param("canvas", destination))
				ASSERT(move->set_param("canvas_interface", instance->find_canvas_interface(destination)))
				ASSERT(move->set_param("layer", layer))
				ASSERT(move->set_param("new_index", depth))
				ASSERT(instance->perform_action(move))
			}
			++depth;
		}
	}
	ASSERT_EQUAL(size_t(2), destination->size())
	ASSERT(destination->front() == pasted.front())
	ASSERT(instance->undo())
	ASSERT(destination->empty())
	ASSERT(instance->redo())
	ASSERT_EQUAL(size_t(2), destination->size())
	ASSERT(destination->front() == pasted.front())
	auto link = pasted.back()->dynamic_param_list().at("origin");
	Vector before = (*link)(0).get(Vector());
	auto root = bone_at(pasted.front(), 0);
	ASSERT(root->set_link("origin", ValueNode_Const::create(Vector(20, 30))))
	Vector after = (*link)(0).get(Vector());
	ASSERT_APPROX_EQUAL(20.0, after[0] - before[0])
	ASSERT_APPROX_EQUAL(30.0, after[1] - before[1])
	Vector unchanged = (*original_link)(0).get(Vector());
	ASSERT_APPROX_EQUAL(original_position[0], unchanged[0])
	ASSERT_APPROX_EQUAL(original_position[1], unchanged[1])
}

int main(int, const char* argv[])
{
#ifdef _WIN32
	const auto root_path = filesystem::Path::absolute_path(std::string(argv[0]) + "/../../");
#else
	const auto root_path = filesystem::Path::absolute_path(std::string(argv[0]) + "/../../../");
#endif
	synfigapp::Main main(root_path);
	TEST_SUITE_BEGIN()
		TEST_FUNCTION(test_clipboard_keeps_cloned_parent_chain)
		TEST_FUNCTION(test_paste_keeps_follower_on_pasted_skeleton)
		TEST_FUNCTION(test_unselected_bones_and_exported_nodes_remain_shared)
		TEST_FUNCTION(test_animated_follower_and_repeated_pastes)
		TEST_FUNCTION(test_embedded_group_keeps_internal_skeleton_links)
		TEST_FUNCTION(test_regular_layers_keep_values_and_selection_order)
		TEST_FUNCTION(test_embed_imported_canvas_preserves_bone_links)
		TEST_FUNCTION(test_pasted_bones_move_follower_after_undo_and_redo)
	TEST_SUITE_END();
	return tst_exit_status;
}
