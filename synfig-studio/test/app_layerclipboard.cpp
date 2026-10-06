// SPDX-License-Identifier: GPL-2.0-or-later
#include "test_base.h"
#include <synfig/canvas.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_bonelink.h>
#include <synfig/valuenodes/valuenode_staticlist.h>
#include <synfigapp/main.h>
#include <list>

using namespace synfig;

struct SkeletonFixture
{
	Canvas::Handle canvas = Canvas::create();
	Layer::Handle skeleton = Layer::create("skeleton");
	Layer::Handle follower = Layer::create("circle");
	ValueNode_StaticList::Handle bones;
	ValueNode_Bone::Handle root, child, tip;

	SkeletonFixture()
	{
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
	std::list<Layer::Handle> result;
	for (const auto& layer : source)
		result.push_back(layer->clone(canvas, guid));
	return result;
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
	TEST_SUITE_END();
	return tst_exit_status;
}
