/*!	\file test/app_layerduplicate.cpp
**	\brief Tests for synfigapp::Action LayerDuplicate
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2021 Synfig authors
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

#include <synfig/canvas.h>
#include <synfig/general.h>
#include <synfig/valuenode_registry.h>

#include <synfigapp/actions/layerduplicate.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/instance.h>
#include <synfigapp/main.h>

#define ERROR_MESSAGE_TWO_VALUES(a, b) \
	std::cerr.precision(8); \
	std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - expected " << a << ", but got " << b << std::endl;

#define ASSERT(value) {\
	if (!(value)) { \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - not TRUE: " << #value << std::endl; \
		return true; \
	} \
}

#define ASSERT_FALSE(value) {\
	if (value) { \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - not FALSE: " << #value << std::endl; \
		return true; \
	} \
}

#define ASSERT_EQUAL(expected, value) {\
	if (expected != value) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_NOT_EQUAL(not_acceptable, value) {\
	if (not_acceptable == value) { \
		std::cerr.precision(8); \
		std::cerr << __FUNCTION__ << ":" << __LINE__ << std::endl << "\t - must not be equal" << std::endl; \
		return true; \
	} \
}

#define ASSERT_APPROX_EQUAL(expected, value) {\
	if (!synfig::approximate_equal(expected, value)) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_APPROX_EQUAL_MICRO(expected, value) {\
	if (std::abs(expected - value) > 1e-6) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define ASSERT_VECTOR_APPROX_EQUAL_MICRO(expected, value) {\
	if (std::abs(expected[0] - value[0]) > 2e-6 || std::abs(expected[1] - value[1]) > 2e-6) { \
		ERROR_MESSAGE_TWO_VALUES(expected, value) \
		return true; \
	} \
}

#define TEST_FUNCTION(function_name) {\
	bool fail = function_name(); \
	if (fail) { \
		synfig::error("%s FAILED", #function_name); \
		failures++; \
	} else { \
		successes++; \
	} \
}

// Check basic case of duplication: a single layer
static bool test_synfigapp_layerduplicate_one_regular_layer()
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

	ASSERT_EQUAL(2, canvas->size())
	// Is duplication placed as first layer?
	ASSERT_EQUAL("circle", canvas->front()->get_name())
	ASSERT_APPROX_EQUAL(5.5, canvas->front()->get_param("radius").get(synfig::Real()))
	ASSERT_EQUAL(3.0, canvas->back()->get_param("radius").get(synfig::Real()))
	return false;
}

// Check if duplicated layer stays right above source layer
static bool test_synfigapp_layerduplicate_place_dup_above_source_layer()
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
	ASSERT_EQUAL(size_t(4), canvas->size())
	ASSERT_APPROX_EQUAL(1.0, canvas->front()->get_param("radius").get(synfig::Real()))

	action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer2);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	// Now: 1 1 2 2 3
	ASSERT_EQUAL(size_t(5), canvas->size())
	ASSERT_APPROX_EQUAL(2.0, (*std::next(canvas->begin(), 2))->get_param("radius").get(synfig::Real()))

	action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer3);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	// Now: 1 1 2 2 3 3
	ASSERT_EQUAL(size_t(6), canvas->size())
	ASSERT_APPROX_EQUAL(3.0, (*std::next(canvas->begin(), 4))->get_param("radius").get(synfig::Real()))

	return false;
}

// Check if action duplicates more than one layer and place them right above first one if in a row
static bool test_synfigapp_layerduplicate_two_regular_layers_must_keep_order()
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
	ASSERT_EQUAL(size_t(5), canvas->size())
	auto layer_it = canvas->begin();
	ASSERT_APPROX_EQUAL(1.0, (*layer_it)->get_param("radius").get(synfig::Real()))
	++layer_it;
	ASSERT_APPROX_EQUAL(2.0, (*layer_it)->get_param("radius").get(synfig::Real()))
	++layer_it;
	ASSERT_APPROX_EQUAL(3.0, (*layer_it)->get_param("radius").get(synfig::Real()))
	++layer_it;
	ASSERT_APPROX_EQUAL(2.0, (*layer_it)->get_param("radius").get(synfig::Real()))
	++layer_it;
	ASSERT_APPROX_EQUAL(3.0, (*layer_it)->get_param("radius").get(synfig::Real()))

	return false;
}

// Check if exported valuenode are not re-created (= linked, keeps same id) - but those not-exported are not linked
static bool test_synfigapp_layerduplicate_regular_layer_with_exported_valuenode()
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	synfig::ValueNode::Handle radius_vn = synfig::ValueNode_Const::create();
	canvas->add_value_node(radius_vn, "exported vn");

	synfig::ValueNode::Handle origin_vn = synfig::ValueNode_Const::create();

	auto layer = synfig::Layer::create("circle");
	ASSERT(layer->connect_dynamic_param("radius", radius_vn))
	ASSERT(layer->connect_dynamic_param("origin", origin_vn))
	canvas->push_back(layer);

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	ASSERT_EQUAL(2, canvas->size())

	// exported valuenodes keep linked
	auto dup_radius_it = canvas->front()->dynamic_param_list().find("radius");
	auto src_radius_it = canvas->back()->dynamic_param_list().find("radius");
	ASSERT(dup_radius_it != canvas->front()->dynamic_param_list().end())
	ASSERT(src_radius_it != canvas->back()->dynamic_param_list().end())
	ASSERT(src_radius_it->second == dup_radius_it->second)
	ASSERT_EQUAL("exported vn", src_radius_it->second->get_id())
	ASSERT_EQUAL("exported vn", dup_radius_it->second->get_id())

	// not-exported valuenodes are NOT the same
	auto dup_origin_it = canvas->front()->dynamic_param_list().find("origin");
	auto src_origin_it = canvas->back()->dynamic_param_list().find("origin");
	ASSERT(dup_origin_it != canvas->front()->dynamic_param_list().end())
	ASSERT(src_origin_it != canvas->back()->dynamic_param_list().end())
	ASSERT(src_origin_it->second != dup_origin_it->second)
	return false;
}

static bool test_synfigapp_layerduplicate_encapsulated_layer_solo()
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

	ASSERT_EQUAL(2, canvas->size())
	return false;
}

static bool test_synfigapp_layerduplicate_encapsulated_layer_with_contents()
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

	ASSERT_EQUAL(2, canvas->size())
	ASSERT_EQUAL("group", canvas->front()->get_name())
	ASSERT_EQUAL("group", canvas->back()->get_name())

	auto dup_canvas = canvas->front()->get_param("canvas").get(canvas);
	auto src_canvas = canvas->back()->get_param("canvas").get(canvas);
	ASSERT_EQUAL(2, dup_canvas->size())
	ASSERT_EQUAL(2, src_canvas->size())
	ASSERT_EQUAL("circle", dup_canvas->front()->get_name())
	ASSERT_EQUAL("rectangle", dup_canvas->back()->get_name())
	ASSERT_EQUAL("circle", src_canvas->front()->get_name())
	ASSERT_EQUAL("rectangle", src_canvas->back()->get_name())

	ASSERT_NOT_EQUAL(dup_canvas->front(), src_canvas->front())
	ASSERT_NOT_EQUAL(dup_canvas->back(), src_canvas->back())
	return false;
}

// Check if duplicated layer_duplicate has its (auto-exported) 'index' parameter renamed
static bool test_synfigapp_layerduplicate_layer_duplicate_solo()
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

	ASSERT_EQUAL(2, canvas->size())
	ASSERT_EQUAL(2, canvas->value_node_list().size())
	ASSERT_NOT_EQUAL("Index 1", canvas->front()->dynamic_param_list().find("index")->second->get_id())
	return false;
}

// Check if duplicated layer whose parameter is linked to layer_duplicate index keeps linked to it too
static bool test_synfigapp_layerduplicate_dup_is_linked_to_same_layer_duplicate_index_valuenode()
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
	ASSERT(layer2->connect_dynamic_param("radius", valuenode))

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer2);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	ASSERT_EQUAL(3, canvas->size())
	auto dup_circle = *std::next(canvas->begin(), 1);
	auto src_circle = *std::next(canvas->begin(), 2);

	ASSERT_EQUAL("Index 1", dup_circle->dynamic_param_list().find("radius")->second->get_id())
	ASSERT_EQUAL("Index 1", src_circle->dynamic_param_list().find("radius")->second->get_id())
	return false;
}

// Bug GH #1921
// Layer_Duplicate and a layer linked to it are Duplicated.
// Check if the latter is linked to the cloned Layer_Duplicate, not the source one.
static bool test_synfigapp_layerduplicate_both_layer_duplicate_and_linked_layers()
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
	ASSERT(layer2->connect_dynamic_param("origin", etl::handle<synfig::ValueNode>::cast_static(composite2)))

	auto layer3 = synfig::Layer::create("circle");
	canvas->push_back(layer3);
	layer3->connect_dynamic_param("radius", valuenode);
	auto composite3 = composite2->clone(canvas);
	ASSERT(layer3->connect_dynamic_param("origin", etl::handle<synfig::ValueNode>::cast_static(composite3)))

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerDuplicate");
	action->set_param("layer", layer1);
	action->set_param("layer", layer2);
	action->set_param("layer", layer3);
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->perform();

	ASSERT_EQUAL(6, canvas->size())
	ASSERT_EQUAL(2, canvas->value_node_list().size())
	ASSERT_EQUAL("duplicate", canvas->front()->get_name())

	// Check 'index' valuenode remapping
	auto dup_valuenode = canvas->front()->dynamic_param_list().find("index")->second;
	auto dup_translate = *std::next(canvas->begin(), 1);
	auto dup_circle = *std::next(canvas->begin(), 2);
	ASSERT_EQUAL(dup_valuenode->get_id(), dup_circle->dynamic_param_list().find("radius")->second->get_id())
	auto dup_circle_origin = dup_circle->dynamic_param_list().find("origin")->second;
	ASSERT_EQUAL("composite", dup_circle_origin->get_name())
	auto dup_circle_link_vn = dynamic_cast<synfig::LinkableValueNode*>(dup_circle_origin.get());
	ASSERT(dup_circle_link_vn)
	ASSERT_EQUAL(dup_valuenode->get_id(), dup_circle_link_vn->get_link("x")->get_id())

	auto dup_translate_origin = dup_translate->dynamic_param_list().find("origin")->second;
	ASSERT_EQUAL("composite", dup_translate_origin->get_name())
	auto dup_translate_link_vn = dynamic_cast<synfig::LinkableValueNode*>(dup_translate_origin.get());
	ASSERT(dup_translate_link_vn)
	ASSERT_EQUAL(dup_valuenode->get_id(), dup_translate_link_vn->get_link("x")->get_id())

	// what about source layers? they should remain linked to source 'index'
	auto src_valuenode = (*std::next(canvas->begin(), 3))->dynamic_param_list().find("index")->second;
	auto src_translate = *std::next(canvas->begin(), 4);
	auto src_circle = *std::next(canvas->begin(), 5);
	ASSERT_EQUAL(src_valuenode->get_id(), src_circle->dynamic_param_list().find("radius")->second->get_id())
	auto src_circle_origin = src_circle->dynamic_param_list().find("origin")->second;
	ASSERT_EQUAL("composite", src_circle_origin->get_name())
	auto src_circle_link_vn = dynamic_cast<synfig::LinkableValueNode*>(src_circle_origin.get());
	ASSERT(src_circle_link_vn)
	ASSERT_EQUAL(src_valuenode->get_id(), src_circle_link_vn->get_link("x")->get_id())

	auto src_translate_origin = src_translate->dynamic_param_list().find("origin")->second;
	ASSERT_EQUAL("composite", src_translate_origin->get_name())
	auto src_translate_link_vn = dynamic_cast<synfig::LinkableValueNode*>(src_translate_origin.get());
	ASSERT(src_translate_link_vn)
	ASSERT_EQUAL(src_valuenode->get_id(), src_translate_link_vn->get_link("x")->get_id())

	// Just to ensure
	ASSERT_NOT_EQUAL(src_valuenode->get_id(), dup_valuenode->get_id())

	// now we check if undo removes the correct exported valuenode
	static_cast<synfigapp::Action::Undoable*>(action.get())->undo();

	ASSERT_EQUAL(3, canvas->size())
	ASSERT_EQUAL(1, canvas->value_node_list().size())
	ASSERT_EQUAL("Index 1", canvas->value_node_list().front()->get_id())
	return false;
}

// When duplicating two groups (each one with a layer_duplicate) at same time,
// it used to try to create new Index with same names (wrong)
// and export them (error on second export)
// Check if recent fix works
static bool test_synfigapp_layerduplicate_two_groups_with_layer_duplicate_each()
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

	ASSERT_EQUAL(4, canvas->size())
	ASSERT_EQUAL(4, canvas->value_node_list().size())

	auto dup1_child_canvas = canvas->front()->get_param("canvas").get(canvas);
	auto dup2_child_canvas = (*std::next(canvas->begin(),1))->get_param("canvas").get(canvas);
	ASSERT_EQUAL(1, dup1_child_canvas->size())
	ASSERT_EQUAL(1, dup2_child_canvas->size())

	std::string new_id1 = dup1_child_canvas->front()->dynamic_param_list().find("index")->second->get_id();
	std::string new_id2 = dup2_child_canvas->front()->dynamic_param_list().find("index")->second->get_id();
	ASSERT_NOT_EQUAL("some_index1", new_id1)
	ASSERT_NOT_EQUAL("some_index2", new_id1)
	ASSERT_NOT_EQUAL(new_id2, new_id1)
	ASSERT_NOT_EQUAL("some_index1", new_id2)
	ASSERT_NOT_EQUAL("some_index2", new_id2)

	return false;
}

// Case: There are two sibling groups, each one has a layer_duplicate.
// Check if duplicating only the layer_duplicates (not the groups) work
static bool test_synfigapp_layerduplicate_two_layer_duplicate_in_different_groups()
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

	ASSERT_EQUAL(2, canvas->size())
	ASSERT_EQUAL(4, canvas->value_node_list().size())

	std::string new_id1 = child_canvas1->front()->dynamic_param_list().find("index")->second->get_id();
	std::string new_id2 = child_canvas2->front()->dynamic_param_list().find("index")->second->get_id();
	ASSERT_NOT_EQUAL("some_index1", new_id1)
	ASSERT_NOT_EQUAL("some_index2", new_id1)
	ASSERT_NOT_EQUAL(new_id2, new_id1)
	ASSERT_NOT_EQUAL("some_index1", new_id2)
	ASSERT_NOT_EQUAL("some_index2", new_id2)

	return false;
}

// User try to duplicate a group AND an inner layer_duplicate
// Check if it creates a new group with two layer_duplicate (with different Index parameters)
static bool test_synfigapp_layerduplicate_encapsulated_and_inner_layer_duplicate()
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

	ASSERT_EQUAL(2, canvas->size())
	ASSERT_EQUAL(3, canvas->value_node_list().size())

	// - dup_group
	//   - dup_layer_duplicate1
	//   - dup_layer_duplicate2
	// - group
	//   - layer_duplicate
	auto dup_group = canvas->front();
	ASSERT_EQUAL("group", dup_group->get_name())
	auto dup_child_canvas = dup_group->get_param("canvas").get(canvas);
	auto dup_layer_duplicate1 = dup_child_canvas->front();
	ASSERT_EQUAL("duplicate", dup_layer_duplicate1->get_name())
	auto dup_layer_duplicate2 = *std::next(dup_child_canvas->begin(), 1);
	ASSERT_EQUAL("duplicate", dup_layer_duplicate2->get_name())

	std::string new_id1 = dup_layer_duplicate1->dynamic_param_list().find("index")->second->get_id();
	std::string new_id2 = dup_layer_duplicate2->dynamic_param_list().find("index")->second->get_id();
	ASSERT_NOT_EQUAL("some_index1", new_id1)
	ASSERT_NOT_EQUAL("some_index1", new_id2)
	ASSERT_NOT_EQUAL(new_id2, new_id1)

	return false;
}

int main()
{
	synfigapp::Main Main("");

	int successes = 0;
	int failures = 0;
	bool exception_thrown = false;

	try {
		TEST_FUNCTION(test_synfigapp_layerduplicate_one_regular_layer)
		TEST_FUNCTION(test_synfigapp_layerduplicate_place_dup_above_source_layer)
		TEST_FUNCTION(test_synfigapp_layerduplicate_two_regular_layers_must_keep_order)
		TEST_FUNCTION(test_synfigapp_layerduplicate_regular_layer_with_exported_valuenode)
		TEST_FUNCTION(test_synfigapp_layerduplicate_encapsulated_layer_solo)
		TEST_FUNCTION(test_synfigapp_layerduplicate_encapsulated_layer_with_contents)
		TEST_FUNCTION(test_synfigapp_layerduplicate_layer_duplicate_solo)
		TEST_FUNCTION(test_synfigapp_layerduplicate_dup_is_linked_to_same_layer_duplicate_index_valuenode)
		TEST_FUNCTION(test_synfigapp_layerduplicate_both_layer_duplicate_and_linked_layers)
		TEST_FUNCTION(test_synfigapp_layerduplicate_two_groups_with_layer_duplicate_each)
		TEST_FUNCTION(test_synfigapp_layerduplicate_two_layer_duplicate_in_different_groups)
		TEST_FUNCTION(test_synfigapp_layerduplicate_encapsulated_and_inner_layer_duplicate)
	} catch (...) {
		synfig::error("Some exception has been thrown.");
		exception_thrown = true;
	}

	if (exception_thrown)
		synfig::error("Test interrupted due to an execption thrown (%i errors and %i successfull tests until then)", failures, successes);
	else if (failures)
		synfig::error("Test finished with %i errors and %i successfull tests", failures, successes);
	else
		synfig::info("Success");

	return exception_thrown? 2 : (failures ? 1 : 0);
}
