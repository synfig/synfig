/*!	\file test/app_layerparamreset.cpp
**	\brief Tests for synfigapp::Action LayerParamReset
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2025 Synfig authors
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

#include "test_base.h"

#include <synfig/canvas.h>
#include <synfig/general.h>
#include <synfig/transformation.h>
#include <synfig/valuenode_registry.h>

#include <synfigapp/actions/layerparamreset.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/instance.h>
#include <synfigapp/main.h>

/* === A U X I L I A R S =================================================== */

std::ostream& operator<<(std::ostream& os, const synfig::Transformation& t)
{
	os << '[' << t.offset << ',' << t.angle <<  ',' << t.skew_angle <<  ',' << t.scale << ']';
	return os;
}

static synfig::Layer::Handle
create_layer(const std::string& layer_name, etl::handle<synfigapp::Instance>& instance)
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("MyCanvas");
	instance = synfigapp::Instance::create(canvas, nullptr);
	synfig::Layer::Handle layer = synfig::Layer::create(layer_name);
	canvas->push_back(layer);
	return layer;
}

static synfigapp::Action::Handle
create_action(synfig::Layer::Handle& layer, const std::string& param_name, etl::handle<synfigapp::Instance>& instance)
{
	synfigapp::Action::Handle action = synfigapp::Action::create("LayerParamReset");
	action->set_param("value_desc", synfigapp::ValueDesc(layer, param_name));
	action->set_param("canvas", instance->get_canvas());
	action->set_param("canvas_interface", instance->find_canvas_interface(instance->get_canvas()));
	return action;
}

/* === T E S T S =========================================================== */

static void
test_synfigapp_layerparamreset_accepts_only_layerparam()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);

	synfigapp::Action::Handle action = synfigapp::Action::create("LayerParamReset");
	ASSERT(action->set_param("value_desc", synfigapp::ValueDesc(layer, "radius")));
	action->set_param("canvas", instance->get_canvas());
	action->set_param("canvas_interface", instance->find_canvas_interface(instance->get_canvas()));
	ASSERT(action->is_ready());

	auto value_node = synfig::ValueNodeRegistry::create("add", synfig::Real());
	layer->get_canvas()->add_value_node(value_node, "my_vn");
	ASSERT_FALSE(action->set_param("value_desc", synfigapp::ValueDesc(layer->get_canvas(), "my_vn")));
	ASSERT_FALSE(action->set_param("value_desc", synfigapp::ValueDesc(value_node, 0)));
	auto vn_const = synfig::ValueNode_Const::Handle::cast_static(synfig::ValueNode_Const::create(synfig::Real(3)));
	ASSERT_FALSE(action->set_param("value_desc", synfigapp::ValueDesc(vn_const)));
	auto vn_anim = synfig::ValueNode_Animated::Handle::cast_static(synfig::ValueNode_Animated::create(synfig::Real(3), synfig::Time(1.)));
	ASSERT_FALSE(action->set_param("value_desc", synfigapp::ValueDesc(vn_anim, synfig::Time(1.))));
}

static void
test_synfigapp_layerparamreset_resets_param_real()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);
	const synfig::Real initial_value = layer->get_param("radius").get(synfig::Real());
	const synfig::Real new_value = initial_value + 5.5;

	ASSERT(layer->set_param("radius", new_value));
	ASSERT_EQUAL(new_value, layer->get_param("radius").get(synfig::Real()));

	synfigapp::Action::Handle action = create_action(layer, "radius", instance);
	ASSERT_NO_EXCEPTION_THROWN(action->perform());

	ASSERT_EQUAL(initial_value, layer->get_param("radius").get(synfig::Real()));
}

static void
test_synfigapp_layerparamreset_undoes_back_to_original_real()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);

	const synfig::Real initial_value = layer->get_param("radius").get(synfig::Real());
	const synfig::Real new_value = initial_value + 5.5;
	layer->set_param("radius", new_value);

	synfigapp::Action::Undoable::Handle action = synfigapp::Action::Undoable::Handle::cast_static(create_action(layer, "radius", instance));
	action->perform();

	ASSERT_NO_EXCEPTION_THROWN(action->undo());

	ASSERT_EQUAL(new_value, layer->get_param("radius").get(synfig::Real()));
}

static void
test_synfigapp_layerparamreset_resets_param_vector()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);
	const synfig::Vector initial_value = layer->get_param("origin").get(synfig::Vector());
	const synfig::Vector new_value = initial_value + synfig::Vector(5.5, -4.0);

	ASSERT(layer->set_param("origin", new_value));
	ASSERT_EQUAL(new_value, layer->get_param("origin").get(synfig::Vector()));

	synfigapp::Action::Handle action = create_action(layer, "origin", instance);
	ASSERT_NO_EXCEPTION_THROWN(action->perform());

	ASSERT_EQUAL(initial_value, layer->get_param("origin").get(synfig::Vector()));
}

static void
test_synfigapp_layerparamreset_undoes_back_to_original_vector()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);

	const synfig::Vector initial_value = layer->get_param("origin").get(synfig::Vector());
	const synfig::Vector new_value = initial_value + synfig::Vector(5.5, -4.0);
	layer->set_param("origin", new_value);

	synfigapp::Action::Undoable::Handle action = synfigapp::Action::Undoable::Handle::cast_static(create_action(layer, "origin", instance));
	action->perform();

	ASSERT_NO_EXCEPTION_THROWN(action->undo());

	ASSERT_EQUAL(new_value, layer->get_param("origin").get(synfig::Vector()));
}

static void
test_synfigapp_layerparamreset_resets_param_transformation()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("group", instance);
	const synfig::Transformation initial_value = layer->get_param("transformation").get(synfig::Transformation());
	const synfig::Transformation new_value(
		initial_value.offset + synfig::Vector(5.5, -4.0),
		initial_value.angle + synfig::Angle::rad(1.),
		initial_value.skew_angle + synfig::Angle::rad(0.5),
		initial_value.scale + synfig::Vector(0.5, -0.2)
	);

	ASSERT(layer->set_param("transformation", new_value));
	ASSERT_EQUAL(new_value, layer->get_param("transformation").get(synfig::Transformation()));

	synfigapp::Action::Handle action = create_action(layer, "transformation", instance);
	ASSERT_NO_EXCEPTION_THROWN(action->perform());

	ASSERT_EQUAL(initial_value, layer->get_param("transformation").get(synfig::Transformation()));
}

static void
test_synfigapp_layerparamreset_undoes_back_to_original_transformation()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("group", instance);

	const synfig::Transformation initial_value = layer->get_param("transformation").get(synfig::Transformation());
	const synfig::Transformation new_value(
		initial_value.offset + synfig::Vector(5.5, -4.0),
		initial_value.angle + synfig::Angle::rad(1.),
		initial_value.skew_angle + synfig::Angle::rad(0.5),
		initial_value.scale + synfig::Vector(0.5, -0.2)
	);

	layer->set_param("transformation", new_value);

	synfigapp::Action::Undoable::Handle action = synfigapp::Action::Undoable::Handle::cast_static(create_action(layer, "transformation", instance));
	action->perform();

	ASSERT_NO_EXCEPTION_THROWN(action->undo());

	ASSERT_EQUAL(new_value, layer->get_param("transformation").get(synfig::Transformation()));
}

static void
test_synfigapp_layerparamreset_removes_animation_on_resetting()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);
	const synfig::Real initial_value = layer->get_param("radius").get(synfig::Real());

	const synfig::Real new_value = initial_value + 5.5;
	auto animation = synfig::ValueNode_Animated::create(synfig::Real(new_value), synfig::Time(1.0));
	animation->add(synfig::Waypoint(initial_value, synfig::Time(1000.)));
	ASSERT(layer->connect_dynamic_param("radius", animation.get()));

	synfigapp::Action::Handle action = create_action(layer, "radius", instance);
	ASSERT_NO_EXCEPTION_THROWN(action->perform());

	ASSERT_EQUAL(0, layer->dynamic_param_list().count("radius"));
	ASSERT_EQUAL(initial_value, layer->get_param("radius").get(synfig::Real()));
}

static void
test_synfigapp_layerparamreset_restores_animation_on_undoing()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);
	const synfig::Real initial_value = layer->get_param("radius").get(synfig::Real());

	const synfig::Real new_value = initial_value + 5.5;
	auto animation = synfig::ValueNode_Animated::create(synfig::Real(new_value), synfig::Time(1.0));
	animation->add(synfig::Waypoint(initial_value, synfig::Time(1000.)));
	ASSERT(layer->connect_dynamic_param("radius", animation.get()));

	synfigapp::Action::Undoable::Handle action = synfigapp::Action::Undoable::Handle::cast_static(create_action(layer, "radius", instance));
	action->perform();

	ASSERT_NO_EXCEPTION_THROWN(action->undo());

	ASSERT_EQUAL(1, layer->dynamic_param_list().count("radius"));
	auto it = layer->dynamic_param_list().find("radius");
	ASSERT(it != layer->dynamic_param_list().end());
	ASSERT(animation == it->second);
}

static void
test_synfigapp_layerparamreset_removes_link_on_resetting()
{
	synfig::Layer::Handle layer1, layer2;
	etl::handle<synfigapp::Instance> instance;

	layer1 = create_layer("circle", instance);
	layer2 = create_layer("circle", instance);
	const synfig::Real initial_value = layer1->get_param("radius").get(synfig::Real());
	const synfig::Real new_value = initial_value + 5.5;

	auto value_node = synfig::ValueNodeRegistry::create("reference", new_value);
	ASSERT(layer1->connect_dynamic_param("radius", value_node.get()));
	ASSERT(layer2->connect_dynamic_param("radius", value_node.get()));

	ASSERT_EQUAL(1, layer1->dynamic_param_list().count("radius"));
	ASSERT_EQUAL(1, layer2->dynamic_param_list().count("radius"));
	ASSERT_EQUAL(2, value_node->parent_count());

	synfigapp::Action::Handle action = create_action(layer1, "radius", instance);
	ASSERT_NO_EXCEPTION_THROWN(action->perform());

	ASSERT_EQUAL(0, layer1->dynamic_param_list().count("radius"));
	ASSERT_EQUAL(initial_value, layer1->get_param("radius").get(synfig::Real()));

	ASSERT_EQUAL(1, layer2->dynamic_param_list().count("radius"));
	auto it = layer2->dynamic_param_list().find("radius");
	ASSERT(it != layer2->dynamic_param_list().end());
	ASSERT(value_node == it->second);
}

static void
test_synfigapp_layerparamreset_restores_link_on_undoing()
{
	synfig::Layer::Handle layer1, layer2;
	etl::handle<synfigapp::Instance> instance;

	layer1 = create_layer("circle", instance);
	layer2 = create_layer("circle", instance);
	const synfig::Real initial_value = layer1->get_param("radius").get(synfig::Real());
	const synfig::Real new_value = initial_value + 5.5;

	auto value_node = synfig::ValueNodeRegistry::create("reference", new_value);
	layer1->connect_dynamic_param("radius", value_node.get());
	layer2->connect_dynamic_param("radius", value_node.get());

	synfigapp::Action::Undoable::Handle action = synfigapp::Action::Undoable::Handle::cast_static(create_action(layer1, "radius", instance));
	action->perform();

	ASSERT_NO_EXCEPTION_THROWN(action->undo());

	ASSERT_EQUAL(1, layer1->dynamic_param_list().count("radius"));
	auto it1 = layer1->dynamic_param_list().find("radius");
	ASSERT(it1 != layer1->dynamic_param_list().end());
	ASSERT(value_node == it1->second);

	ASSERT_EQUAL(1, layer2->dynamic_param_list().count("radius"));
	auto it2 = layer2->dynamic_param_list().find("radius");
	ASSERT(it2 != layer2->dynamic_param_list().end());
	ASSERT(value_node == it2->second);
}

static void
test_synfigapp_layerparamreset_disconnects_exported_id_on_resetting()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);
	auto canvas = layer->get_canvas();
	const synfig::Real initial_value = layer->get_param("radius").get(synfig::Real());
	const synfig::Real new_value = initial_value + 5.5;
	{
		auto value_node = synfig::ValueNodeRegistry::create("reference", new_value);
		layer->get_canvas()->add_value_node(value_node, "my-valuenode");
	}

	ASSERT(layer->connect_dynamic_param("radius", canvas->find_value_node("my-valuenode", false)));

	ASSERT_EQUAL(1, layer->dynamic_param_list().count("radius"));

	synfigapp::Action::Handle action = create_action(layer, "radius", instance);
	ASSERT_NO_EXCEPTION_THROWN(action->perform());

	ASSERT_EQUAL(0, layer->dynamic_param_list().count("radius"));
	ASSERT_EQUAL(initial_value, layer->get_param("radius").get(synfig::Real()));

	ASSERT(canvas->find_value_node("my-valuenode", false));
}

static void
test_synfigapp_layerparamreset_restores_exported_id_on_undoing()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("circle", instance);
	auto canvas = layer->get_canvas();
	const synfig::Real initial_value = layer->get_param("radius").get(synfig::Real());
	const synfig::Real new_value = initial_value + 5.5;
	{
		auto value_node = synfig::ValueNodeRegistry::create("reference", new_value);
		layer->get_canvas()->add_value_node(value_node, "my-valuenode");
	}

	layer->connect_dynamic_param("radius", canvas->find_value_node("my-valuenode", false));

	synfigapp::Action::Undoable::Handle action = synfigapp::Action::Undoable::Handle::cast_static(create_action(layer, "radius", instance));
	action->perform();

	ASSERT_NO_EXCEPTION_THROWN(action->undo());

	ASSERT_EQUAL(1, layer->dynamic_param_list().count("radius"));
	auto it1 = layer->dynamic_param_list().find("radius");
	ASSERT(it1 != layer->dynamic_param_list().end());
	ASSERT(canvas->find_value_node("my-valuenode", false) == it1->second);
}

static void
test_synfigapp_layerparamreset_keeps_composite_valuenode_on_resetting()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("group", instance);
	const synfig::Transformation initial_value = layer->get_param("transformation").get(synfig::Transformation());
	const synfig::Transformation new_value(
		initial_value.offset + synfig::Vector(5.5, -4.0),
		initial_value.angle + synfig::Angle::rad(1.),
		initial_value.skew_angle + synfig::Angle::rad(0.5),
		initial_value.scale + synfig::Vector(0.5, -0.2)
	);

	{
		auto value_node = synfig::ValueNodeRegistry::create("composite", new_value);
		layer->connect_dynamic_param("transformation", value_node.get());
	}
	ASSERT_EQUAL(1, layer->dynamic_param_list().count("transformation"));
	ASSERT_EQUAL(new_value, (*layer->dynamic_param_list().find("transformation")->second)(0.0).get(synfig::Transformation()));

	synfigapp::Action::Handle action = create_action(layer, "transformation", instance);
	ASSERT_NO_EXCEPTION_THROWN(action->perform());

	ASSERT_EQUAL(1, layer->dynamic_param_list().count("transformation"));
	ASSERT_EQUAL("composite", layer->dynamic_param_list().find("transformation")->second->get_name());
	ASSERT_EQUAL(initial_value, (*layer->dynamic_param_list().find("transformation")->second)(0.0).get(synfig::Transformation()));
}

static void
test_synfigapp_layerparamreset_restores_original_composite_valuenode_on_resetting()
{
	synfig::Layer::Handle layer;
	etl::handle<synfigapp::Instance> instance;

	layer = create_layer("group", instance);

	const synfig::Transformation initial_value = layer->get_param("transformation").get(synfig::Transformation());
	const synfig::Transformation new_value(
		initial_value.offset + synfig::Vector(5.5, -4.0),
		initial_value.angle + synfig::Angle::rad(1.),
		initial_value.skew_angle + synfig::Angle::rad(0.5),
		initial_value.scale + synfig::Vector(0.5, -0.2)
	);

	synfig::ValueNode::LooseHandle vn_loose;
	{
		auto value_node = synfig::ValueNodeRegistry::create("composite", new_value);
		vn_loose = value_node;
		layer->connect_dynamic_param("transformation", value_node.get());
	}
	synfigapp::Action::Undoable::Handle action = synfigapp::Action::Undoable::Handle::cast_static(create_action(layer, "transformation", instance));
	action->perform();

	ASSERT_NO_EXCEPTION_THROWN(action->undo());

	ASSERT_EQUAL(1, layer->dynamic_param_list().count("transformation"));
	ASSERT_EQUAL("composite", layer->dynamic_param_list().find("transformation")->second->get_name());
	ASSERT_EQUAL(new_value, (*layer->dynamic_param_list().find("transformation")->second)(0.0).get(synfig::Transformation()));
	// Check if it is exactly the ORIGINAL valuenode
	ASSERT_EQUAL(vn_loose.get(), layer->dynamic_param_list().find("transformation")->second.get());
}

int main(int /*argc*/, const char* argv[])
{
// test binaries are in `bin/test` folder, but for Windows they should be in `bin`
// folder, because there is no RPATH on Windows, and it can't find required dll's
#ifdef _WIN32
	const std::string root_path = synfig::filesystem::Path::absolute_path(std::string(argv[0]) + "/../../");
#else
	const std::string root_path = synfig::filesystem::Path::absolute_path(std::string(argv[0]) + "/../../../");
#endif
	synfigapp::Main Main(root_path);

	TEST_SUITE_BEGIN();

	TEST_FUNCTION(test_synfigapp_layerparamreset_accepts_only_layerparam);
	TEST_FUNCTION(test_synfigapp_layerparamreset_resets_param_real);
	TEST_FUNCTION(test_synfigapp_layerparamreset_undoes_back_to_original_real);
	TEST_FUNCTION(test_synfigapp_layerparamreset_resets_param_vector);
	TEST_FUNCTION(test_synfigapp_layerparamreset_undoes_back_to_original_vector);
	TEST_FUNCTION(test_synfigapp_layerparamreset_resets_param_transformation);
	TEST_FUNCTION(test_synfigapp_layerparamreset_undoes_back_to_original_transformation);
	TEST_FUNCTION(test_synfigapp_layerparamreset_removes_animation_on_resetting);
	TEST_FUNCTION(test_synfigapp_layerparamreset_restores_animation_on_undoing);
	TEST_FUNCTION(test_synfigapp_layerparamreset_removes_link_on_resetting);
	TEST_FUNCTION(test_synfigapp_layerparamreset_restores_link_on_undoing);
	TEST_FUNCTION(test_synfigapp_layerparamreset_disconnects_exported_id_on_resetting);
	TEST_FUNCTION(test_synfigapp_layerparamreset_restores_exported_id_on_undoing);
	TEST_FUNCTION(test_synfigapp_layerparamreset_keeps_composite_valuenode_on_resetting);
	TEST_FUNCTION(test_synfigapp_layerparamreset_restores_original_composite_valuenode_on_resetting);

	TEST_SUITE_END();

	return tst_exit_status;
}
