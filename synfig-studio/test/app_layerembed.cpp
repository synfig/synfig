/*!	\file test/app_layerembed.cpp
**	\brief Tests for synfigapp::Action LayerEmbed
**
**	\legal
**	Copyright (c) 2026 Synfig authors
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

#include <synfigapp/action_param.h>
#include <synfigapp/actions/layerembed.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/instance.h>
#include <synfigapp/main.h>

static synfigapp::Action::ParamList
make_layerembed_params(synfig::Canvas::Handle canvas, synfig::Layer::Handle layer)
{
	synfigapp::Action::ParamList params;
	params.add("canvas", canvas);
	params.add("layer", layer);
	return params;
}

static void
perform_layerembed(etl::handle<synfigapp::Instance> instance, synfig::Canvas::Handle canvas, synfig::Layer::Handle layer)
{
	synfigapp::Action::Handle action = synfigapp::Action::create("LayerEmbed");
	action->set_param("canvas", canvas);
	action->set_param("canvas_interface", instance->find_canvas_interface(canvas));
	action->set_param("layer", layer);
	ASSERT(action->is_ready())
	action->perform();
}

static void
test_synfigapp_layerembed_rejects_local_child_canvas()
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("main");

	synfig::Canvas::Handle child_canvas = canvas->new_child_canvas("child");
	synfig::Layer::Handle group_layer = synfig::Layer::create("group");
	group_layer->set_param("canvas", child_canvas);
	canvas->push_back(group_layer);

	ASSERT_FALSE(synfigapp::Action::LayerEmbed::is_candidate(make_layerembed_params(canvas, group_layer)))
}

static void
test_synfigapp_layerembed_embeds_external_child_canvas()
{
	synfig::Canvas::Handle canvas = synfig::Canvas::create();
	canvas->set_id("main");
	canvas->set_file_name("/tmp/anim.sif");
	auto instance = synfigapp::Instance::create(canvas, nullptr);

	synfig::Canvas::Handle external_root = synfig::Canvas::create();
	external_root->set_file_name("/tmp/exported.sif");
	synfig::Canvas::Handle external_child = external_root->new_child_canvas("canvas");

	synfig::Layer::Handle nested_group = synfig::Layer::create("group");
	nested_group->set_param("canvas", external_child);
	external_root->push_back(nested_group);

	synfig::Layer::Handle root_group = synfig::Layer::create("group");
	root_group->set_param("canvas", external_root);
	canvas->push_back(root_group);

	perform_layerembed(instance, canvas, root_group);

	synfig::Canvas::Handle embedded_root = root_group->get_param("canvas").get(canvas);
	ASSERT(embedded_root)
	ASSERT_NOT_EQUAL(external_root.get(), embedded_root.get())
	ASSERT_EQUAL(canvas.get(), embedded_root->get_root().get())
	ASSERT_EQUAL("exported", embedded_root->get_id())
	ASSERT_EQUAL(1, embedded_root->size())

	synfig::Layer::Handle embedded_nested_group = embedded_root->front();
	synfig::Canvas::Handle still_external_child = embedded_nested_group->get_param("canvas").get(canvas);
	ASSERT_EQUAL(external_child.get(), still_external_child.get())
	ASSERT(synfigapp::Action::LayerEmbed::is_candidate(make_layerembed_params(embedded_root, embedded_nested_group)))

	perform_layerembed(instance, embedded_root, embedded_nested_group);

	synfig::Canvas::Handle embedded_child = embedded_nested_group->get_param("canvas").get(canvas);
	ASSERT(embedded_child)
	ASSERT_NOT_EQUAL(external_child.get(), embedded_child.get())
	ASSERT_EQUAL(canvas.get(), embedded_child->get_root().get())
	ASSERT_EQUAL(embedded_root.get(), embedded_child->parent().get())
	ASSERT_EQUAL("canvas", embedded_child->get_id())
}

int main(int argc, const char* argv[])
{
// test binaries are in `bin/test` folder, but for Windows they should be in `bin`
// folder, because there is no RPATH on Windows, and it can't find required dll's
#ifdef _WIN32
	const std::string root_path = synfig::filesystem::Path::absolute_path(std::string(argv[0]) + "/../../");
#else
	const std::string root_path = synfig::filesystem::Path::absolute_path(std::string(argv[0]) + "/../../../");
#endif
	synfigapp::Main Main(root_path);

	TEST_SUITE_BEGIN()
		TEST_FUNCTION(test_synfigapp_layerembed_rejects_local_child_canvas)
		TEST_FUNCTION(test_synfigapp_layerembed_embeds_external_child_canvas)
	TEST_SUITE_END();

	return tst_exit_status;
}
