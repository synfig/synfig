/*!	\file test/gtest_app_layerduplicate.cpp
**	\brief Tests for synfigapp::Action LayerDuplicate (Google Test)
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2021 Synfig authors
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

#include <gtest/gtest.h>

#include <synfig/canvas.h>
#include <synfig/general.h>
#include <synfig/valuenode_registry.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfig/valuenodes/valuenode_bonelink.h>
#include <synfig/valuenodes/valuenode_staticlist.h>

#include <synfigapp/actions/layerduplicate.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/instance.h>
#include <synfigapp/main.h>

class LayerDuplicateTest : public ::testing::Test {
protected:
	static std::unique_ptr<synfigapp::Main> synfig_main_;

	static void SetUpTestSuite() {
		// test binaries are in `bin/test` folder, but for Windows they should be in `bin`
		// folder, because there is no RPATH on Windows, and it can't find required dll's
		const char* argv0 = ::testing::internal::GetArgvs()[0].c_str();
#ifdef _WIN32
		const std::string root_path = synfig::filesystem::Path::absolute_path(std::string(argv0) + "/../../");
#else
		const std::string root_path = synfig::filesystem::Path::absolute_path(std::string(argv0) + "/../../../");
#endif
		synfig_main_.reset(new synfigapp::Main(root_path));
	}

	static void TearDownTestSuite() {
		synfig_main_.reset();
	}
};

std::unique_ptr<synfigapp::Main> LayerDuplicateTest::synfig_main_;

// Check basic case of duplication: a single layer
TEST_F(LayerDuplicateTest, OneRegularLayer)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);
	auto layer = synfig::Layer::create("circle");
	layer->set_param("radius", 5.5);
	canvas->push_back(layer);

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	layer->set_param("radius", 3.0);

	EXPECT_EQ(2, canvas->size());
	// Is duplication placed as first layer?
	EXPECT_EQ("circle", canvas->front()->get_name());
	EXPECT_NEAR(5.5, canvas->front()->get_param("radius").get(synfig::Real()), 1e-4);
	EXPECT_EQ(3.0, canvas->back()->get_param("radius").get(synfig::Real()));
}

// Check if duplicated layer stays right above source layer
TEST_F(LayerDuplicateTest, PlaceDupAboveSourceLayer)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);
	auto layer1 = synfig::Layer::create("circle");
	layer1->set_param("radius", 1.0);
	canvas->push_back(layer1);
	auto layer2 = synfig::Layer::create("circle");
	layer2->set_param("radius", 2.0);
	canvas->push_back(layer2);
	auto layer3 = synfig::Layer::create("circle");
	layer3->set_param("radius", 3.0);
	canvas->push_back(layer3);

	// Canvas layers: 1 2 3

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer1);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	// Now: 1 1 2 3
	EXPECT_EQ(size_t(4), canvas->size());
	EXPECT_NEAR(1.0, canvas->front()->get_param("radius").get(synfig::Real()), 1e-4);

	action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer2);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	// Now: 1 1 2 2 3
	EXPECT_EQ(size_t(5), canvas->size());
	EXPECT_NEAR(2.0, (*std::next(canvas->begin(), 2))->get_param("radius").get(synfig::Real()), 1e-4);

	action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer3);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	// Now: 1 1 2 2 3 3
	EXPECT_EQ(size_t(6), canvas->size());
	EXPECT_NEAR(3.0, (*std::next(canvas->begin(), 4))->get_param("radius").get(synfig::Real()), 1e-4);
}

// Check if action duplicates more than one layer and place them right above first one if in a row
TEST_F(LayerDuplicateTest, TwoRegularLayersMustKeepOrder)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);
	auto layer1 = synfig::Layer::create("circle");
	layer1->set_param("radius", 1.0);
	canvas->push_back(layer1);
	auto layer2 = synfig::Layer::create("circle");
	layer2->set_param("radius", 2.0);
	canvas->push_back(layer2);
	auto layer3 = synfig::Layer::create("circle");
	layer3->set_param("radius", 3.0);
	canvas->push_back(layer3);

	// Canvas layers: 1 2 3

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer2);
	action->set_param("layer", layer3);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	// Now: 1 2 3 2 3
	EXPECT_EQ(size_t(5), canvas->size());
	auto layer_it = canvas->begin();
	EXPECT_NEAR(1.0, (*layer_it)->get_param("radius").get(synfig::Real()), 1e-4);
	++layer_it;
	EXPECT_NEAR(2.0, (*layer_it)->get_param("radius").get(synfig::Real()), 1e-4);
	++layer_it;
	EXPECT_NEAR(3.0, (*layer_it)->get_param("radius").get(synfig::Real()), 1e-4);
	++layer_it;
	EXPECT_NEAR(2.0, (*layer_it)->get_param("radius").get(synfig::Real()), 1e-4);
	++layer_it;
	EXPECT_NEAR(3.0, (*layer_it)->get_param("radius").get(synfig::Real()), 1e-4);
}

// Check if exported valuenode are not re-created (= linked, keeps same id) - but those not-exported are not linked
TEST_F(LayerDuplicateTest, RegularLayerWithExportedValuenode)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	synfig::ValueNode::Handle radius_vn = synfig::ValueNode_Const::create();
	canvas->add_value_node(radius_vn, "exported vn");

	synfig::ValueNode::Handle origin_vn = synfig::ValueNode_Const::create();

	auto layer = synfig::Layer::create("circle");
	EXPECT_TRUE(layer->connect_dynamic_param("radius", radius_vn));
	EXPECT_TRUE(layer->connect_dynamic_param("origin", origin_vn));
	canvas->push_back(layer);

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(2, canvas->size());

	// exported valuenodes keep linked
	auto dup_radius_it = canvas->front()->dynamic_param_list().find("radius");
	auto src_radius_it = canvas->back()->dynamic_param_list().find("radius");
	EXPECT_TRUE(dup_radius_it != canvas->front()->dynamic_param_list().end());
	EXPECT_TRUE(src_radius_it != canvas->back()->dynamic_param_list().end());
	EXPECT_TRUE(src_radius_it->second == dup_radius_it->second);
	EXPECT_EQ("exported vn", src_radius_it->second->get_id());
	EXPECT_EQ("exported vn", dup_radius_it->second->get_id());

	// not-exported valuenodes are NOT the same
	auto dup_origin_it = canvas->front()->dynamic_param_list().find("origin");
	auto src_origin_it = canvas->back()->dynamic_param_list().find("origin");
	EXPECT_TRUE(dup_origin_it != canvas->front()->dynamic_param_list().end());
	EXPECT_TRUE(src_origin_it != canvas->back()->dynamic_param_list().end());
	EXPECT_TRUE(src_origin_it->second != dup_origin_it->second);
}

TEST_F(LayerDuplicateTest, EncapsulatedLayerSolo)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	auto layer = synfig::Layer::create("group");
	auto child_canvas=synfig::Canvas::create_inline(canvas);
	layer->set_param("canvas", child_canvas);
	canvas->push_back(layer);

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(2, canvas->size());
}

TEST_F(LayerDuplicateTest, EncapsulatedLayerWithContents)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	auto layer = synfig::Layer::create("group");
	auto child_canvas=synfig::Canvas::create_inline(canvas);
	layer->set_param("canvas", child_canvas);
	canvas->push_back(layer);

	auto layer1 = synfig::Layer::create("circle");
	child_canvas->push_back(layer1);
	auto layer2 = synfig::Layer::create("rectangle");
	child_canvas->push_back(layer2);

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(2, canvas->size());
	EXPECT_EQ("group", canvas->front()->get_name());
	EXPECT_EQ("group", canvas->back()->get_name());

	auto dup_canvas = canvas->front()->get_param("canvas").get(canvas);
	auto src_canvas = canvas->back()->get_param("canvas").get(canvas);
	EXPECT_EQ(2, dup_canvas->size());
	EXPECT_EQ(2, src_canvas->size());
	EXPECT_EQ("circle", dup_canvas->front()->get_name());
	EXPECT_EQ("rectangle", dup_canvas->back()->get_name());
	EXPECT_EQ("circle", src_canvas->front()->get_name());
	EXPECT_EQ("rectangle", src_canvas->back()->get_name());

	EXPECT_FALSE(dup_canvas->front() == src_canvas->front());
	EXPECT_FALSE(dup_canvas->back() == src_canvas->back());
}

// Check if duplicated layer_duplicate has its (auto-exported) 'index' parameter renamed
TEST_F(LayerDuplicateTest, LayerDuplicateSolo)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	auto layer1 = synfig::Layer::create("duplicate");
	canvas->push_back(layer1);
	auto valuenode = layer1->dynamic_param_list().find("index")->second;
	canvas->add_value_node(valuenode, "Index 1");

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer1);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(2, canvas->size());
	EXPECT_EQ(2, canvas->value_node_list().size());
	EXPECT_NE("Index 1", canvas->front()->dynamic_param_list().find("index")->second->get_id());
}

// Check if duplicated layer whose parameter is linked to layer_duplicate index keeps linked to it too
TEST_F(LayerDuplicateTest, DupIsLinkedToSameLayerDuplicateIndexValuenode)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	auto layer1 = synfig::Layer::create("duplicate");
	canvas->push_back(layer1);
	auto valuenode = layer1->dynamic_param_list().find("index")->second;
	canvas->add_value_node(valuenode, "Index 1");

	auto layer2 = synfig::Layer::create("circle");
	canvas->push_back(layer2);
	EXPECT_TRUE(layer2->connect_dynamic_param("radius", valuenode));

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer2);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(3, canvas->size());
	auto dup_circle = *std::next(canvas->begin(), 1);
	auto src_circle = *std::next(canvas->begin(), 2);

	EXPECT_EQ("Index 1", dup_circle->dynamic_param_list().find("radius")->second->get_id());
	EXPECT_EQ("Index 1", src_circle->dynamic_param_list().find("radius")->second->get_id());
}

// Bug GH #1921
// Layer_Duplicate and a layer linked to it are Duplicated.
// Check if the latter is linked to the cloned Layer_Duplicate, not the source one.
TEST_F(LayerDuplicateTest, BothLayerDuplicateAndLinkedLayers)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	auto layer1 = synfig::Layer::create("duplicate");
	canvas->push_back(layer1);
	auto valuenode = layer1->dynamic_param_list().find("index")->second;
	canvas->add_value_node(valuenode, "Index 1");

	auto composite2 = synfig::ValueNodeRegistry::create("composite", synfig::Vector());
	composite2->set_link("x", valuenode);

	auto layer2 = synfig::Layer::create("translate");
	canvas->push_back(layer2);
	EXPECT_TRUE(layer2->connect_dynamic_param("origin", synfig::ValueNode::Handle::cast_static(composite2)));

	auto layer3 = synfig::Layer::create("circle");
	canvas->push_back(layer3);
	layer3->connect_dynamic_param("radius", valuenode);
	auto composite3 = composite2->clone(canvas);
	EXPECT_TRUE(layer3->connect_dynamic_param("origin", synfig::ValueNode::Handle::cast_static(composite3)));

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer1);
	action->set_param("layer", layer2);
	action->set_param("layer", layer3);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(6, canvas->size());
	EXPECT_EQ(2, canvas->value_node_list().size());
	EXPECT_EQ("duplicate", canvas->front()->get_name());

	// Check 'index' valuenode remapping
	auto dup_valuenode = canvas->front()->dynamic_param_list().find("index")->second;
	auto dup_translate = *std::next(canvas->begin(), 1);
	auto dup_circle = *std::next(canvas->begin(), 2);
	EXPECT_EQ(dup_valuenode->get_id(), dup_circle->dynamic_param_list().find("radius")->second->get_id());
	auto dup_circle_origin = dup_circle->dynamic_param_list().find("origin")->second;
	EXPECT_EQ("composite", dup_circle_origin->get_name());
	auto dup_circle_link_vn = dynamic_cast<synfig::LinkableValueNode*>(dup_circle_origin.get());
	EXPECT_TRUE(dup_circle_link_vn);
	EXPECT_EQ(dup_valuenode->get_id(), dup_circle_link_vn->get_link("x")->get_id());

	auto dup_translate_origin = dup_translate->dynamic_param_list().find("origin")->second;
	EXPECT_EQ("composite", dup_translate_origin->get_name());
	auto dup_translate_link_vn = dynamic_cast<synfig::LinkableValueNode*>(dup_translate_origin.get());
	EXPECT_TRUE(dup_translate_link_vn);
	EXPECT_EQ(dup_valuenode->get_id(), dup_translate_link_vn->get_link("x")->get_id());

	// what about source layers? they should remain linked to source 'index'
	auto src_valuenode = (*std::next(canvas->begin(), 3))->dynamic_param_list().find("index")->second;
	auto src_translate = *std::next(canvas->begin(), 4);
	auto src_circle = *std::next(canvas->begin(), 5);
	EXPECT_EQ(src_valuenode->get_id(), src_circle->dynamic_param_list().find("radius")->second->get_id());
	auto src_circle_origin = src_circle->dynamic_param_list().find("origin")->second;
	EXPECT_EQ("composite", src_circle_origin->get_name());
	auto src_circle_link_vn = dynamic_cast<synfig::LinkableValueNode*>(src_circle_origin.get());
	EXPECT_TRUE(src_circle_link_vn);
	EXPECT_EQ(src_valuenode->get_id(), src_circle_link_vn->get_link("x")->get_id());

	auto src_translate_origin = src_translate->dynamic_param_list().find("origin")->second;
	EXPECT_EQ("composite", src_translate_origin->get_name());
	auto src_translate_link_vn = dynamic_cast<synfig::LinkableValueNode*>(src_translate_origin.get());
	EXPECT_TRUE(src_translate_link_vn);
	EXPECT_EQ(src_valuenode->get_id(), src_translate_link_vn->get_link("x")->get_id());

	// Just to ensure
	EXPECT_NE(src_valuenode->get_id(), dup_valuenode->get_id());

	// now we check if undo removes the correct exported valuenode
	static_cast<synfigapp::Action::Undoable*>(action.get())->undo();

	EXPECT_EQ(3, canvas->size());
	EXPECT_EQ(1, canvas->value_node_list().size());
	EXPECT_EQ("Index 1", canvas->value_node_list().front()->get_id());
}

// When duplicating two groups (each one with a layer_duplicate) at same time,
// it used to try to create new Index with same names (wrong)
// and export them (error on second export)
// Check if recent fix works
TEST_F(LayerDuplicateTest, TwoGroupsWithLayerDuplicateEach)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	auto group_layer1 = synfig::Layer::create("group");
	auto child_canvas1=synfig::Canvas::create_inline(canvas);
	group_layer1->set_param("canvas", child_canvas1);
	canvas->push_back(group_layer1);

	auto duplicate_layer1 = synfig::Layer::create("duplicate");
	child_canvas1->push_back(duplicate_layer1);
	auto valuenode1 = duplicate_layer1->dynamic_param_list().find("index")->second;
	canvas->add_value_node(valuenode1, "some_index1");

	auto group_layer2 = synfig::Layer::create("group");
	auto child_canvas2=synfig::Canvas::create_inline(canvas);
	group_layer2->set_param("canvas", child_canvas2);
	canvas->push_back(group_layer2);

	auto duplicate_layer2 = synfig::Layer::create("duplicate");
	child_canvas2->push_back(duplicate_layer2);
	auto valuenode2 = duplicate_layer2->dynamic_param_list().find("index")->second;
	canvas->add_value_node(valuenode2, "some_index2");

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", group_layer1);
	action->set_param("layer", group_layer2);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(4, canvas->size());
	EXPECT_EQ(4, canvas->value_node_list().size());

	auto dup1_child_canvas = canvas->front()->get_param("canvas").get(canvas);
	auto dup2_child_canvas = (*std::next(canvas->begin(),1))->get_param("canvas").get(canvas);
	EXPECT_EQ(1, dup1_child_canvas->size());
	EXPECT_EQ(1, dup2_child_canvas->size());

	std::string new_id1 = dup1_child_canvas->front()->dynamic_param_list().find("index")->second->get_id();
	std::string new_id2 = dup2_child_canvas->front()->dynamic_param_list().find("index")->second->get_id();
	EXPECT_NE("some_index1", new_id1);
	EXPECT_NE("some_index2", new_id1);
	EXPECT_NE(new_id2, new_id1);
	EXPECT_NE("some_index1", new_id2);
	EXPECT_NE("some_index2", new_id2);
}

// Case: There are two sibling groups, each one has a layer_duplicate.
// Check if duplicating only the layer_duplicates (not the groups) work
TEST_F(LayerDuplicateTest, TwoLayerDuplicateInDifferentGroups)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	auto group_layer1 = synfig::Layer::create("group");
	auto child_canvas1=synfig::Canvas::create_inline(canvas);
	group_layer1->set_param("canvas", child_canvas1);
	canvas->push_back(group_layer1);

	auto duplicate_layer1 = synfig::Layer::create("duplicate");
	child_canvas1->push_back(duplicate_layer1);
	auto valuenode1 = duplicate_layer1->dynamic_param_list().find("index")->second;
	canvas->add_value_node(valuenode1, "some_index1");

	auto group_layer2 = synfig::Layer::create("group");
	auto child_canvas2=synfig::Canvas::create_inline(canvas);
	group_layer2->set_param("canvas", child_canvas2);
	canvas->push_back(group_layer2);

	auto duplicate_layer2 = synfig::Layer::create("duplicate");
	child_canvas2->push_back(duplicate_layer2);
	auto valuenode2 = duplicate_layer2->dynamic_param_list().find("index")->second;
	canvas->add_value_node(valuenode2, "some_index2");

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", duplicate_layer1);
	action->set_param("layer", duplicate_layer2);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(2, canvas->size());
	EXPECT_EQ(4, canvas->value_node_list().size());

	std::string new_id1 = child_canvas1->front()->dynamic_param_list().find("index")->second->get_id();
	std::string new_id2 = child_canvas2->front()->dynamic_param_list().find("index")->second->get_id();
	EXPECT_NE("some_index1", new_id1);
	EXPECT_NE("some_index2", new_id1);
	EXPECT_NE(new_id2, new_id1);
	EXPECT_NE("some_index1", new_id2);
	EXPECT_NE("some_index2", new_id2);
}

// User try to duplicate a group AND an inner layer_duplicate
// Check if it creates a new group and don't duplicate layer_duplicate
TEST_F(LayerDuplicateTest, EncapsulatedAndInnerLayerDuplicate)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	auto group_layer = synfig::Layer::create("group");
	auto child_canvas=synfig::Canvas::create_inline(canvas);
	group_layer->set_param("canvas", child_canvas);
	canvas->push_back(group_layer);

	auto duplicate_layer = synfig::Layer::create("duplicate");
	child_canvas->push_back(duplicate_layer);
	auto valuenode = duplicate_layer->dynamic_param_list().find("index")->second;
	canvas->add_value_node(valuenode, "some_index1");

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", group_layer);
	action->set_param("layer", duplicate_layer);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(2, canvas->size());
	EXPECT_EQ(2, canvas->value_node_list().size());

	// - dup_group
	//   - dup_layer_duplicate
	// - group
	//   - layer_duplicate
	auto dup_group = canvas->front();
	EXPECT_EQ("group", dup_group->get_name());
	auto dup_child_canvas = dup_group->get_param("canvas").get(canvas);
	EXPECT_EQ(1, dup_child_canvas->size());
	EXPECT_EQ(1, child_canvas->size());
	auto dup_layer_duplicate1 = dup_child_canvas->front();
	EXPECT_EQ("duplicate", dup_layer_duplicate1->get_name());

	std::string new_id1 = dup_layer_duplicate1->dynamic_param_list().find("index")->second->get_id();
	std::string id0 = duplicate_layer->dynamic_param_list().find("index")->second->get_id();
	EXPECT_NE("some_index1", new_id1);
	EXPECT_EQ("some_index1", id0);
}

TEST_F(LayerDuplicateTest, SkeletonWithBoneLink)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	// create bones for skeleton
	std::vector<synfig::Bone> bones(2);
	bones[0] = synfig::Bone(synfig::Point(0,0), synfig::Point(0.5,0.5));
	bones[0].set_parent(synfig::ValueNode_Bone_Root::create(synfig::Bone()));
	bones[0].set_name("my bone 0");

	bones[1] = synfig::Bone(synfig::Point(0.5,0.5), synfig::Point(1,1));
	bones[1].set_parent(synfig::ValueNode_Bone_Root::create(synfig::Bone()));
	bones[1].set_name("my bone 1");

	// create skeleton
	auto layer = synfig::Layer::create("skeleton");
	synfig::ValueBase param_bones;
	param_bones.set_list_of(bones);
	// Layer clone does not work as expected without `layer->set_param("bones", param_bones);` - bone valuenodes are not cloned if not dynamic
	layer->set_param("bones", param_bones);
	EXPECT_TRUE(layer->connect_dynamic_param("bones", synfig::ValueNode_StaticList::create(param_bones)));

	canvas->push_back(layer);

	// retrieve bone[0] value node for setting as bone[1] parent
	synfig::ValueNode_Bone::Handle bone0_vn;
	// it should be the line below, but find() is not static, as it depends on bone canvas
	//	bone0_vn = synfig::ValueNode_Bone::Handle::find("my bone 0");
	{
		const auto& bone_map = synfig::ValueNode_Bone::get_bone_map(canvas);
		for (auto iter =  bone_map.cbegin(); iter != bone_map.cend(); ++iter)
			if ((*iter->second->get_link("name"))(0).get(synfig::String()) == "my bone 0")
			{
				bone0_vn = iter->second;
			}
	}

	EXPECT_TRUE(bone0_vn);
	bones[1].set_parent(bone0_vn.get());

	EXPECT_EQ(2, bone0_vn->get_bone_map(canvas).size());

	// add circle and link its origin to bone[0]
	auto circle = synfig::Layer::create("circle");
	synfig::ValueNode_BoneLink::Handle bone_link = synfig::ValueNode_BoneLink::create(synfig::Vector(3,5));
	bone_link->set_link("bone", synfig::ValueNode_Const::create(bone0_vn));
	circle->connect_dynamic_param("origin", bone_link.get());
	canvas->push_back(circle);

	// duplicate current layers
	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer);
	action->set_param("layer", circle);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(4, canvas->size());
	EXPECT_EQ(4, synfig::ValueNode_Bone::get_bone_map(canvas).size());

	// check result layer list order
	EXPECT_EQ("skeleton", canvas->front()->get_name());
	EXPECT_EQ("circle", (*std::next(canvas->begin(),1))->get_name());
	EXPECT_EQ("circle", canvas->back()->get_name());
	EXPECT_TRUE(circle == canvas->back());
	EXPECT_FALSE(circle == *std::next(canvas->begin(), 1));

	// check if bones were duplicated too: they would be automatically with different names
	EXPECT_EQ("my bone 0", (*std::next(canvas->begin(),2))->get_param("bones").get_list_of(synfig::Bone())[0].get_name());
	EXPECT_NE("my bone 0", canvas->front()->get_param("bones").get_list_of(synfig::Bone())[0].get_name());

	// are the circles' bone links different?
	auto cloned_circle = (*std::next(canvas->begin(),1));
	auto cloned_origin_pair = cloned_circle->dynamic_param_list().find("origin");
	EXPECT_TRUE(cloned_origin_pair->second);
	auto cloned_origin = cloned_origin_pair->second;
	EXPECT_EQ("bone_link", cloned_origin->get_name());
	auto cloned_bone_link = synfig::ValueNode_BoneLink::Handle::cast_static(cloned_origin);
	EXPECT_FALSE(bone_link == cloned_bone_link);

	EXPECT_EQ("constant", bone_link->get_link("bone")->get_name());
	auto bone_link_const = synfig::ValueNode_Const::Handle::cast_static(bone_link->get_link("bone"));
	EXPECT_EQ("my bone 0", bone_link_const->get_value().get(synfig::ValueNode_Bone::Handle())->get_bone_name(synfig::Time()));

	// truth time : the new circle layer is linked to new skeleton bone?
	EXPECT_EQ("constant", cloned_bone_link->get_link("bone")->get_name());
	auto cloned_bone_link_const = synfig::ValueNode_Const::Handle::cast_static(cloned_bone_link->get_link("bone"));
	EXPECT_NE("my bone 0", cloned_bone_link_const->get_value().get(synfig::ValueNode_Bone::Handle())->get_bone_name(synfig::Time()));
}

TEST_F(LayerDuplicateTest, SkeletonWithAnimatedBoneLink)
{
	// same setup of previous test: test_synfigapp_layerduplicate_skeleton_with_animated_bone_link

	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	// create bones for skeleton
	std::vector<synfig::Bone> bones(2);
	bones[0] = synfig::Bone(synfig::Point(0,0), synfig::Point(0.5,0.5));
	bones[0].set_parent(synfig::ValueNode_Bone_Root::create(synfig::Bone()));
	bones[0].set_name("my bone 0");

	bones[1] = synfig::Bone(synfig::Point(0.5,0.5), synfig::Point(1,1));
	bones[1].set_parent(synfig::ValueNode_Bone_Root::create(synfig::Bone()));
	bones[1].set_name("my bone 1");

	// create skeleton
	auto layer = synfig::Layer::create("skeleton");
	synfig::ValueBase param_bones;
	param_bones.set_list_of(bones);
	// Layer clone does not work as expected without `layer->set_param("bones", param_bones);` - bone valuenodes are not cloned if not dynamic
	layer->set_param("bones", param_bones);
	EXPECT_TRUE(layer->connect_dynamic_param("bones", synfig::ValueNode_StaticList::create(param_bones)));

	canvas->push_back(layer);

	// retrieve bone[0] value node for setting as bone[1] parent
	synfig::ValueNode_Bone::Handle bone0_vn;
	// it should be the line below, but find() is not static, as it depends on bone canvas
	//	bone0_vn = synfig::ValueNode_Bone::Handle::find("my bone 0");
	{
		const auto& bone_map = synfig::ValueNode_Bone::get_bone_map(canvas);
		for (auto iter =  bone_map.cbegin(); iter != bone_map.cend(); ++iter)
			if ((*iter->second->get_link("name"))(0).get(synfig::String()) == "my bone 0")
			{
				bone0_vn = iter->second;
			}
	}

	EXPECT_TRUE(bone0_vn);
	bones[1].set_parent(bone0_vn.get());

	// retrieve bone[1] value node
	synfig::ValueNode_Bone::Handle bone1_vn;
	{
		const auto& bone_map = synfig::ValueNode_Bone::get_bone_map(canvas);
		for (auto iter =  bone_map.cbegin(); iter != bone_map.cend(); ++iter)
			if ((*iter->second->get_link("name"))(0).get(synfig::String()) == "my bone 1")
			{
				bone1_vn = iter->second;
			}
	}

	EXPECT_TRUE(bone1_vn);

	EXPECT_EQ(2, bone0_vn->get_bone_map(canvas).size());

	// NEW: Animated valuenode for bone link for origin!
	auto animated_vn = synfig::ValueNode_Animated::create(synfig::type_bone_valuenode);
	animated_vn->new_waypoint(0.0, synfig::ValueBase(bone0_vn));
	animated_vn->new_waypoint(1.0, synfig::ValueBase(bone1_vn));

	// add circle and link its origin to bone[0]
	auto circle = synfig::Layer::create("circle");
	synfig::ValueNode_BoneLink::Handle bone_link = synfig::ValueNode_BoneLink::create(synfig::Vector(3,5));
	bone_link->set_link("bone", animated_vn);
	circle->connect_dynamic_param("origin", bone_link.get());
	canvas->push_back(circle);

	// duplicate current layers
	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer);
	action->set_param("layer", circle);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	EXPECT_EQ(4, canvas->size());
	EXPECT_EQ(4, synfig::ValueNode_Bone::get_bone_map(canvas).size());

	// check result layer list order
	EXPECT_EQ("skeleton", canvas->front()->get_name());
	EXPECT_EQ("circle", (*std::next(canvas->begin(),1))->get_name());
	EXPECT_EQ("circle", canvas->back()->get_name());
	EXPECT_TRUE(circle == canvas->back());
	EXPECT_FALSE(circle == *std::next(canvas->begin(), 1));

	// check if bones were duplicated too: they would be automatically with different names
	EXPECT_EQ("my bone 0", (*std::next(canvas->begin(),2))->get_param("bones").get_list_of(synfig::Bone())[0].get_name());
	std::string cloned_bone0_name = canvas->front()->get_param("bones").get_list_of(synfig::Bone())[0].get_name();
	EXPECT_NE("my bone 0", cloned_bone0_name);
	std::string cloned_bone1_name = canvas->front()->get_param("bones").get_list_of(synfig::Bone())[1].get_name();
	EXPECT_NE("my bone 1", cloned_bone1_name);

	// are the circles' bone links different?
	auto cloned_circle = (*std::next(canvas->begin(),1));
	auto cloned_origin_pair = cloned_circle->dynamic_param_list().find("origin");
	EXPECT_TRUE(cloned_origin_pair->second);
	auto cloned_origin = cloned_origin_pair->second;
	EXPECT_EQ("bone_link", cloned_origin->get_name());
	auto cloned_bone_link = synfig::ValueNode_BoneLink::Handle::cast_static(cloned_origin);
	EXPECT_FALSE(bone_link == cloned_bone_link);

	EXPECT_EQ("animated", bone_link->get_link("bone")->get_name());
	auto bone_link_animated = synfig::ValueNode_Animated::Handle::cast_static(bone_link->get_link("bone"));

	EXPECT_EQ("my bone 0", (*bone_link_animated)(0.0).get(synfig::ValueNode_Bone::Handle())->get_bone_name(synfig::Time()));
	EXPECT_EQ("my bone 1", (*bone_link_animated)(1.0).get(synfig::ValueNode_Bone::Handle())->get_bone_name(synfig::Time()));

	// truth time : the new circle layer is linked to new skeleton bone?
	EXPECT_EQ("animated", cloned_bone_link->get_link("bone")->get_name());
	auto cloned_bone_link_animated = synfig::ValueNode_Animated::Handle::cast_static(cloned_bone_link->get_link("bone"));
	EXPECT_EQ(cloned_bone0_name, (*cloned_bone_link_animated)(0.0).get(synfig::ValueNode_Bone::Handle())->get_bone_name(synfig::Time()));
	EXPECT_EQ(cloned_bone1_name, (*cloned_bone_link_animated)(1.0).get(synfig::ValueNode_Bone::Handle())->get_bone_name(synfig::Time()));
}
