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

#include "dialog_canvasdependencies.h"

#include <glibmm/fileutils.h>

#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/treeview.h>

#include <gui/app.h>
#include <gui/iconcontroller.h>
#include <gui/localization.h>
#include <gui/resourcehelper.h>

#include <synfig/canvasfilenaming.h>
#include <synfig/general.h>
#include <synfig/loadcanvas.h>
#include <synfig/synfig_iterations.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

struct ExternalValueNodeCollector {
	struct CanvasFileStatistics {
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

	struct LayerParameterStatistics {
		int total = 0;
		int dynamic = 0;
		/** Map (Layer, parameter name) -> match number **/
		std::map<std::pair<Layer::LooseHandle, std::string>, int> per_parameter;

		void record_usage(Layer::LooseHandle layer, const std::string& param_name, bool pdynamic)
		{
			total++;
			if (pdynamic)
				dynamic++;
			auto it = per_parameter.find({layer, param_name});
			if (it == per_parameter.end()) {
				per_parameter[{layer, param_name}] = 1;
			} else {
				it->second++;
			}
		}
	};

	std::map<std::string, CanvasFileStatistics> external_canvas_stats;
	std::map<filesystem::Path, LayerParameterStatistics> external_resource_stats;

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
			if (param_desc.get_name() == "canvas") {
				const std::string& param_name = param_desc.get_name();

				std::set<ValueBase> values;
				bool is_from_dynamic_param_list = false;
				auto it = layer->dynamic_param_list().find(param_name);
				if (it != layer->dynamic_param_list().end()) {
					it->second->get_values(values);
					is_from_dynamic_param_list = true;
				} else {
					ValueBase v = layer->get_param(param_name);
					if (v.is_valid())
						values.insert(v);
				}

				for (const auto& v : values) {
					if (v.get_type() != type_canvas)
						continue; // or break ?
					if (auto canvas = v.get(Canvas::Handle())) {
						if (canvas->is_inline() || canvas == this->canvas || canvas->get_root() == this->canvas)
							continue;
						auto filename_str = canvas->get_file_name();
						if (!filename_str.empty()) {
							filename_str = CanvasFileNaming::make_full_filename(layer->get_canvas()->get_file_name(), filename_str);
							external_resource_stats[filename_str].record_usage(layer, param_name, is_from_dynamic_param_list);
						}
					}
				}
			}
			else if (param_desc.get_hint() == "filename" || param_desc.get_name() == "filename") {
				const std::string& param_name = param_desc.get_name();

				std::set<ValueBase> values;
				bool is_from_dynamic_param_list = false;
				auto it = layer->dynamic_param_list().find(param_name);
				if (it != layer->dynamic_param_list().end()) {
					it->second->get_values(values);
					is_from_dynamic_param_list = true;
				} else {
					ValueBase v = layer->get_param(param_name);
					if (v.is_valid())
						values.insert(v);
				}

				for (const auto& v : values) {
					auto filename_str = v.get(String());
					if (!filename_str.empty()) {
						filename_str = CanvasFileNaming::make_full_filename(layer->get_canvas()->get_file_name(), filename_str);
						external_resource_stats[filename_str].record_usage(layer, param_name, is_from_dynamic_param_list);
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

	external_resource_model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
				refGlade->get_object("external_resources_liststore")
			);

	refGlade->get_widget("external_resources_listbox", resources_listbox);

	refresh();
}

Dialog_CanvasDependencies* Dialog_CanvasDependencies::create(Gtk::Window& parent)
{
	auto refBuilder = ResourceHelper::load_interface("dialog_canvasdependencies.glade");
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
Dialog_CanvasDependencies::set_canvas_interface(etl::handle<synfigapp::CanvasInterface> canvas_interface)
{
	this->canvas = canvas_interface->get_canvas();
	this->canvas_interface = canvas_interface;
	refresh();
}

void Dialog_CanvasDependencies::refresh()
{
	canvas_filepath_label->set_text(canvas ? canvas->get_file_name() : _("No canvas set"));

	if (external_canvas_model)
		external_canvas_model->clear();

	if (external_resource_model)
		external_resource_model->clear();
	if (resources_listbox) {
		auto rows = resources_listbox->get_children();
		for (const auto& row : rows)
			resources_listbox->remove(*row);
	}

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
			auto filename = CanvasFileNaming::make_short_filename(canvas->get_file_name(), pair.first);
			row->set_value(0, filename);
			row->set_value(1, pair.second.total);
			row->set_value(3, !Glib::file_test(pair.first, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_REGULAR));
			for (const auto &vn_pair : pair.second.per_valuenode) {
				if (vn_pair.first->get_id().empty())
					continue;
				Gtk::TreeIter child = external_canvas_model->append(row->children());
				child->set_value(0, vn_pair.first->get_id());
				child->set_value(1, vn_pair.second);
				child->set_value(2, get_tree_pixbuf(vn_pair.first->get_type()));
				child->set_value(3, false);
			}
		}
	}

	if (external_resource_model) {
		for (const auto& pair : collector.external_resource_stats) {
			Gtk::TreeIter row = external_resource_model->append();
			auto filename = CanvasFileNaming::make_short_filename(canvas->get_file_name(), pair.first.u8string());
			row->set_value(0, filename);
			row->set_value(1, pair.second.total);
			Glib::RefPtr<Gdk::Pixbuf> pixbuf;
			std::string ext = Glib::ustring(pair.first.extension().u8string()).lowercase();
			if (!ext.empty())
				ext = ext.substr(1);
			const std::vector<std::string> audio_ext = {"wav", "wave", "mp3", "ogg", "ogm", "oga", "wma", "m4a", "aiff", "aif", "aifc"};
			const std::vector<std::string> image_ext = {"png", "bmp", "jpg", "jpeg", "gif", "tiff", "tif", "dib", "ppm", "pbm", "pgm", "pnm", "webp"};
			const std::vector<std::string> lipsync_ext = {"pgo", "tsv", "xml"};
			const std::vector<std::string> video_ext = {"mpg", "mpeg", "mp2", "m2v", "m4v", "mp4", "m4p", "ogv", "avi", "mov", "webm", "wmv", "mkv", "vob", "mng"};
			const std::vector<std::string> synfig_ext = {"sif", "sifz", "sfg"};
			if (std::find(synfig_ext.begin(), synfig_ext.end(), ext) != synfig_ext.end())
				pixbuf = get_tree_pixbuf_from_icon_name("about_icon");
			else if (std::find(image_ext.begin(), image_ext.end(), ext) != image_ext.end())
				pixbuf = get_tree_pixbuf_layer("import");
			else if (std::find(audio_ext.begin(), audio_ext.end(), ext) != audio_ext.end())
				pixbuf = get_tree_pixbuf_layer("sound");
			else if (std::find(video_ext.begin(), video_ext.end(), ext) != video_ext.end())
				pixbuf = get_tree_pixbuf_from_icon_name("toggle_background_rendering_icon");
			else if (std::find(lipsync_ext.begin(), lipsync_ext.end(), ext) != lipsync_ext.end())
				pixbuf = get_tree_pixbuf_layer("text");
			row->set_value(2, pixbuf);
			row->set_value(3, !Glib::file_test(pair.first.u8string(), Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_REGULAR));

			auto box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));
			auto icon = Gtk::manage(new Gtk::Image(pixbuf));
			icon->set_hexpand(false);
			icon->set_halign(Gtk::ALIGN_START);
			box->pack_start(*icon, Gtk::PACK_SHRINK);
			if (!Glib::file_test(pair.first.u8string(), Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_REGULAR)) {
				auto missing = Gtk::manage(new Gtk::Image("image-missing"));
				box->pack_start(*missing, Gtk::PACK_SHRINK);
				missing->set_halign(Gtk::ALIGN_END);
			}
			auto label = Gtk::manage(new Gtk::Label());
			label->set_markup(strprintf(_n("%s\t<small>(%d reference)</small>", "%s\t<small>(%d references)</small>", pair.second.total), filename.c_str(), pair.second.total));
			label->set_halign(Gtk::ALIGN_START);
			label->set_hexpand(true);
			box->pack_start(*label);
			auto replace_btn = Gtk::manage(new Gtk::Button(_("Change")));
			replace_btn->set_tooltip_text(_("Change the resource file"));
			replace_btn->set_hexpand(false);
			replace_btn->set_halign(Gtk::ALIGN_END);
			replace_btn->signal_clicked().connect(sigc::bind(sigc::bind(sigc::bind(sigc::mem_fun(*this, &Dialog_CanvasDependencies::on_replace_button_pressed), pair.second.dynamic), pair.second.per_parameter), pair.first));
			box->pack_start(*replace_btn, Gtk::PACK_SHRINK);
			box->show_all();
			box->set_hexpand();
			resources_listbox->append(*box);
		}
	}
}

void
Dialog_CanvasDependencies::on_replace_button_pressed(const synfig::filesystem::Path& filename, const std::map<std::pair<synfig::Layer::LooseHandle, std::string>, int>& parameter_list, int is_dynamic)
{
	// Check if it is in one or more dynamic parameters
	// TODO: Check if it is animated; it could easily be replaced
	if (is_dynamic) {
		App::dialog_message_1b("ERROR", _("Change resource file path"),
							   _("This resource file path is used %i times as parameter of value nodes.\n"
								 "For now, it is not supported, so this file path replacement is aborted."), _("OK"));
		return;
	}

	// Warn user about Undoing when it handles missing files
	if (!Glib::file_test(filename.u8string(), Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_REGULAR)) {
		bool accepted = App::dialog_message_2b(_("Change resource file path"),
											   _("You are about to replace a missing file path to an existent one.\n"
												 "You would be able to undo this task. Are you sure you want to proceed?"), Gtk::MESSAGE_WARNING, _("Cancel"), _("OK"));
		if (!accepted)
			return;
	}

	synfig::filesystem::Path new_filename = filename;
	bool selected = App::dialog_open_file(_("Please choose a replacement file"), new_filename, IMAGE_DIR_PREFERENCE);
	if (!selected)
		return;

	// Warn user about the selected file does not exist!
	while (!Glib::file_test(new_filename.u8string(), Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_REGULAR)) {
		synfig::warning(_("Replacement file does not exist: %s"), new_filename.u8_str());

		bool accepted = App::dialog_message_2b(_("File not found"), _("Do you really want to change resource to an inexisting file?"), Gtk::MESSAGE_WARNING, _("Cancel"), _("Replace"));
		if (accepted)
			break;

		bool selected = App::dialog_open_file(_("Please choose a replacement file"), new_filename, IMAGE_DIR_PREFERENCE);
		if (!selected)
			return;
	}

	synfigapp::Action::PassiveGrouper group(canvas_interface->get_instance().get(), strprintf(_("Change resource file %s to %s"), filename.u8_str(), new_filename.u8_str()));
	{
		synfigapp::PushMode push_mode(canvas_interface, synfigapp::MODE_NORMAL);
		for (const auto& param_item : parameter_list) {
			auto layer = param_item.first.first;
			auto param_name = param_item.first.second;
			if (param_name != "canvas") {
				synfig::filesystem::Path canvas_dir(layer->get_canvas()->get_file_name());
				auto short_path = new_filename.proximate_to(canvas_dir.parent_path());
				canvas_interface->change_value(synfigapp::ValueDesc(layer, param_name), short_path.u8string());
			} else {

				String short_filename = CanvasFileNaming::make_short_filename(canvas->get_file_name(), new_filename.u8string());
				String full_filename = CanvasFileNaming::make_full_filename(canvas->get_file_name(), short_filename);

				FileSystem::Handle file_system = CanvasFileNaming::make_filesystem(full_filename);
				if (!file_system)
					throw strprintf(_("Unable to open container:\n%s\n"), full_filename.c_str()) + "\n\n";

				synfig::String errors;
				synfig::String warnings;

				Canvas::Handle new_canvas(synfig::open_canvas_as(file_system->get_identifier(CanvasFileNaming::project_file(full_filename)), full_filename, errors, warnings));
				if (!new_canvas)
					throw String(_("Unable to open this composition")) + ":\n\n" + errors;

				canvas_interface->change_value(synfigapp::ValueDesc(layer, param_name), new_canvas);
				canvas->register_external_canvas(full_filename, new_canvas);
				canvas_interface->signal_layer_new_description()(layer, new_filename.filename().u8string());
			}
		}
	}
	refresh();
}
