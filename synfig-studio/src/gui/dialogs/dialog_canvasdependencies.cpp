/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_canvasdependencies.cpp
**	\brief Implementation of Dialog that shows external dependencies for a canvas
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/dialogs/dialog_canvasdependencies.h>

#include <ETL/stringf>

#include <glibmm/fileutils.h>
#include <glibmm/markup.h>

#include <gtkmm/label.h>
#include <gtkmm/treeview.h>

#include <gui/iconcontroller.h>
#include <gui/localization.h>
#include <gui/resourcehelper.h>

#include <synfig/general.h>
#include <synfig/synfig_iterations.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static Glib::RefPtr<Gtk::Builder> load_interface() {
	auto refBuilder = Gtk::Builder::create();
	try
	{
		refBuilder->add_from_file(ResourceHelper::get_ui_path("dialog_canvasdependencies.glade"));
	}
	catch(const Glib::FileError& ex)
	{
		synfig::error("FileError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	catch(const Glib::MarkupError& ex)
	{
		synfig::error("MarkupError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	catch(const Gtk::BuilderError& ex)
	{
		synfig::error("BuilderError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	return refBuilder;
}

struct ExternalValueNodeCollector {
	struct Statistics {
		int total = 0;
		std::map<ValueNode::LooseHandle, int> per_valuenode;

		void record_usage(ValueNode::LooseHandle valuenode)
		{
			total++;
			auto it = per_valuenode.find(valuenode);
			if (it == per_valuenode.end()) {
				per_valuenode[valuenode] = 1;
			} else {
				it->second++;
			}
		}
	};

	std::map<std::string, Statistics> external_canvas_stats;
	std::map<std::string, int> external_resource_stats;

	ExternalValueNodeCollector(synfig::Canvas::Handle canvas)
	{
		this->canvas = canvas;
	}

	void operator()(Layer::LooseHandle layer, const TraverseLayerStatus& status)
	{
		for (const auto& dynamic_param : layer->dynamic_param_list()) {
			const ValueNode::Handle valuenode = dynamic_param.second;
			traverse_valuenodes(valuenode, [&](ValueNode::Handle value_node) -> TraverseCallbackAction {
				if (value_node->get_root_canvas() != canvas) {
					if (!value_node->get_id().empty()) {
						external_canvas_stats[value_node->get_root_canvas()->get_file_name()].record_usage(value_node);
					}
				}
				return TRAVERSE_CALLBACK_RECURSIVE;
			});
		}

		// Has filename? External resource
		Layer::Vocab vocab = layer->get_param_vocab();
		for (const auto& param_desc : vocab) {
			if (param_desc.get_hint() == "filename" || param_desc.get_name() == "filename") {
				const std::string& param_name = param_desc.get_name();
				auto it = layer->dynamic_param_list().find(param_name);
				if (it != layer->dynamic_param_list().end()) {
					std::set<ValueBase> values;
					it->second->get_values(values);
					for (const auto& v : values) {
						auto str_value = v.get(String());
						if (!str_value.empty())
							external_resource_stats[str_value]++;
					}
				} else {
					ValueBase v = layer->get_param(param_name);
					if (v.is_valid()) {
						auto str_value = v.get(String());
						if (!str_value.empty())
							external_resource_stats[str_value]++;
					}
				}
			}
		}
	}

private:
	synfig::Canvas::Handle canvas;
};

/* === M E T H O D S ======================================================= */

Dialog_CanvasDependencies::Dialog_CanvasDependencies(Gtk::Dialog::BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) :
	Gtk::Dialog(cobject),
	builder(refGlade)
{
	Gtk::Button *button = nullptr;

	refGlade->get_widget("dependencies_close_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &Gtk::Dialog::close));

	refGlade->get_widget("dependencies_canvas_filepath_label", canvas_filepath_label);

	external_canvas_model = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(
				refGlade->get_object("dependencies_treestore")
			);

	external_resource_model = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(
				refGlade->get_object("external_resources_treestore")
			);

	refresh();
}

Dialog_CanvasDependencies* Dialog_CanvasDependencies::create(Gtk::Window& parent)
{
	auto refBuilder = load_interface();
	if (!refBuilder)
		return nullptr;
	Dialog_CanvasDependencies * dialog = nullptr;
	refBuilder->get_widget_derived("dialog_canvasdependencies", dialog);
	if (dialog) {
		dialog->set_transient_for(parent);
	}
	return dialog;
}

Dialog_CanvasDependencies::~Dialog_CanvasDependencies()
{
}

void
Dialog_CanvasDependencies::set_canvas(synfig::Canvas::Handle canvas)
{
	this->canvas = canvas;
	refresh();
}

void Dialog_CanvasDependencies::refresh()
{
	canvas_filepath_label->set_text(canvas ? canvas->get_file_name() : _("No canvas set"));

	if (external_canvas_model)
		external_canvas_model->clear();

	if (external_resource_model)
		external_resource_model->clear();

	if (!canvas)
		return;

	if (!external_canvas_model && !external_resource_model)
		return;

	ExternalValueNodeCollector collector(canvas);
	auto collect_external_valuenodes = [&collector] (Layer::LooseHandle layer, const TraverseLayerStatus& status) {
		collector(layer, status);
	};

	TraverseLayerSettings settings;
	settings.traverse_static_non_inline_canvas = true;
	settings.traverse_dynamic_non_inline_canvas = true;
	for (const auto& layer : *canvas) {
		traverse_layers(layer, collect_external_valuenodes, settings);
	}

	if (external_canvas_model) {
		for (const auto& pair : collector.external_canvas_stats) {
			Gtk::TreeIter row = external_canvas_model->append();
			row->set_value(0, pair.first);
			row->set_value(1, pair.second.total);
			for (const auto &vn_pair : pair.second.per_valuenode) {
				if (vn_pair.first->get_id().empty())
					continue;
				Gtk::TreeIter child = external_canvas_model->append(row->children());
				child->set_value(0, vn_pair.first->get_id());
				child->set_value(1, vn_pair.second);
				child->set_value(2, get_tree_pixbuf(vn_pair.first->get_type()));
			}
		}
	}

	if (external_resource_model) {
		for (const auto& pair : collector.external_resource_stats) {
			Gtk::TreeIter row = external_resource_model->append();
			row->set_value(0, pair.first);
			row->set_value(1, pair.second);
			Glib::RefPtr<Gdk::Pixbuf> pixbuf;
			std::string ext = Glib::ustring(etl::filename_extension(pair.first)).lowercase();
			if (!ext.empty())
				ext = ext.substr(1);
			const std::vector<std::string> audio_ext = {"wav", "wave", "mp3", "ogg", "ogm", "oga", "wma", "m4a", "aiff", "aif", "aifc"};
			const std::vector<std::string> image_ext = {"png", "bmp", "jpg", "jpeg", "gif", "tiff", "tif", "dib", "ppm", "pbm", "pgm", "pnm", "webp"};
			const std::vector<std::string> lipsync_ext = {"pgo", "tsv", "xml"};
			const std::vector<std::string> video_ext = {"mpg", "mpeg", "mp2", "m2v", "m4v", "mp4", "m4p", "ogv", "avi", "mov", "webm", "wmv", "mkv", "vob", "mng"};
			if (std::find(image_ext.begin(), image_ext.end(), ext) != image_ext.end())
				pixbuf = get_tree_pixbuf_layer("import");
			else if (std::find(audio_ext.begin(), audio_ext.end(), ext) != audio_ext.end())
				pixbuf = get_tree_pixbuf_layer("sound");
			else if (std::find(video_ext.begin(), video_ext.end(), ext) != video_ext.end())
				pixbuf = Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-toggle_background_rendering"),Gtk::ICON_SIZE_SMALL_TOOLBAR);
			else if (std::find(lipsync_ext.begin(), lipsync_ext.end(), ext) != lipsync_ext.end())
				pixbuf = get_tree_pixbuf_layer("text");
			row->set_value(2, pixbuf);
		}
	}
}
