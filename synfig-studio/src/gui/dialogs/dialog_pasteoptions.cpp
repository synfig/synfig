/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_pasteoptions.h
**	\brief Implementation of dialog to user choose how to handle value nodes when pasting layers
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
/* ========================================================================= */

#include "dialog_pasteoptions.h"

#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrenderertoggle.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>

#include <gui/iconcontroller.h>
#include <gui/localization.h>
#include <gui/resourcehelper.h>

#include <synfig/general.h>

using namespace synfig;
using namespace studio;

static ValueNode::LooseHandle find_valuenode_by_id(const std::vector<ValueNode::LooseHandle>& list, std::string id)
{
	auto has_same_id = [id](ValueNode::LooseHandle vn) -> bool { return vn->get_id() == id; };
	auto viter = std::find_if(list.begin(), list.end(), has_same_id);
	if (viter == list.end())
		return nullptr;
	return *viter;
}

Dialog_PasteOptions::Dialog_PasteOptions(Gtk::Dialog::BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
	: Gtk::Dialog(cobject),
	  builder(refGlade)
{
	valuenodes_model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
				refGlade->get_object("valuenodes_liststore")
	);

	Glib::RefPtr<Gtk::CellRendererToggle> cellrendererer_toggle = Glib::RefPtr<Gtk::CellRendererToggle>::cast_dynamic(
														  refGlade->get_object("copy_column_renderer")
	);
	if (cellrendererer_toggle) {
		cellrendererer_toggle->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_PasteOptions::on_valuenode_copy_toggled));
	}

	Glib::RefPtr<Gtk::CellRendererText> cellrendererer_name = Glib::RefPtr<Gtk::CellRendererText>::cast_dynamic(
														  refGlade->get_object("name_column_renderer")
	);
	if (cellrendererer_name) {
		cellrendererer_name->signal_edited().connect(sigc::mem_fun(*this, &Dialog_PasteOptions::on_valuenode_name_edited));
	}

	Gtk::IconSize icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
	int window_scale_factor = get_window() ? get_window()->get_scale_factor() : 1;
	auto theme = Gtk::IconTheme::get_default();

	pixbuf_link = theme->load_icon("gtk-connect", Gtk::ICON_SIZE_SMALL_TOOLBAR, window_scale_factor, Gtk::ICON_LOOKUP_USE_BUILTIN);
	pixbuf_external_link = theme->load_icon("valuenode_icon", Gtk::ICON_SIZE_SMALL_TOOLBAR, window_scale_factor, Gtk::ICON_LOOKUP_USE_BUILTIN);
	pixbuf_conflict = theme->load_icon("dialog-error", Gtk::ICON_SIZE_SMALL_TOOLBAR, window_scale_factor, Gtk::ICON_LOOKUP_USE_BUILTIN);
	pixbuf_empty = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, pixbuf_link->get_bits_per_sample(), pixbuf_link->get_width(), pixbuf_link->get_height());
	pixbuf_empty->fill(0);
}

std::shared_ptr<Dialog_PasteOptions> Dialog_PasteOptions::create(Gtk::Window& parent)
{
	auto refBuilder = ResourceHelper::load_interface("dialog_pasteoptions.glade");
	if (!refBuilder)
		return nullptr;
	Dialog_PasteOptions * dialog_ptr = nullptr;
	refBuilder->get_widget_derived("dialog_pasteoptions", dialog_ptr);
	if (dialog_ptr) {
		dialog_ptr->set_transient_for(parent);
	}
	std::shared_ptr<Dialog_PasteOptions> dialog(dialog_ptr);
	return dialog;
}

void Dialog_PasteOptions::set_value_nodes(const std::vector<ValueNode::LooseHandle>& value_node_list)
{
	if (value_node_list.empty()) {
		clear();
		return;
	}

	// grant uniqueness

	for (auto value_node : value_node_list) {
		if (!value_node->is_exported())
			continue;

		value_nodes.insert(value_node);

		std::vector<ValueNode::LooseHandle> dependencies;
		for (auto w : value_node_list) {
			if (w != value_node && value_node->is_ancestor_of(w))
				dependencies.push_back(w);
		}

		value_node_dependencies[value_node] = dependencies;
	}

	rebuild_model();
}

void Dialog_PasteOptions::set_destination_canvas(Canvas::Handle canvas)
{
	if (destination_canvas == canvas)
		return;
	if (!canvas) {
		clear();
		return;
	}
	destination_canvas = canvas;
	rebuild_model();
}

void Dialog_PasteOptions::get_user_choices(std::map<ValueNode::LooseHandle, std::string>& user_choices) const
{
	valuenodes_model->foreach_iter([=, &user_choices](const Gtk::TreeModel::iterator& iter) -> bool {
		ValueNode* value_node;
		std::string name;
		std::string status;
		bool should_copy;
		iter->get_value(COLUMN_VALUENODE_POINTER, value_node);
		iter->get_value(COLUMN_NAME, name);
		iter->get_value(COLUMN_STATUS, status);
		iter->get_value(COLUMN_COPY_OR_NOT, should_copy);
		should_copy &= status != "conflict";

		user_choices[value_node] = should_copy ? name : "";

		return false;
	});
}

void Dialog_PasteOptions::on_valuenode_copy_toggled(const Glib::ustring& path)
{
	auto iter = valuenodes_model->get_iter(path);
	bool is_copy;
	iter->get_value(COLUMN_COPY_OR_NOT, is_copy);
	iter->set_value(COLUMN_COPY_OR_NOT, !is_copy);
	refresh_row_status(std::stoul(path));

	// Enable/Disable choice for its dependencies: if this value node will be
	// external link, all dependencies MUST be linked too

	ValueNode *vn;
	iter->get_value(COLUMN_VALUENODE_POINTER, vn);

	std::vector<Gtk::TreeIter> dependencies;

	valuenodes_model->foreach_iter([vn, &dependencies, this](const Gtk::TreeModel::iterator& w_iter)-> bool {
		ValueNode* w;
		w_iter->get_value(COLUMN_VALUENODE_POINTER, w);

		if (std::find(value_node_dependencies[vn].begin(), value_node_dependencies[vn].end(), w) != value_node_dependencies[vn].end()) {
			dependencies.push_back(w_iter);
		}
		return false;
	});

	for (auto iter : dependencies) {
		std::string path_str = valuenodes_model->get_path(iter).to_string();
		bool would_be_copied;
		iter->get_value(COLUMN_COPY_OR_NOT, would_be_copied);
		if (would_be_copied) {
			iter->set_value(COLUMN_COPY_OR_NOT, false);
			on_valuenode_copy_toggled(path_str);
		} else {
			refresh_row_status(std::stoul(path_str));
		}
	}

	update_ok_button_sensitivity();
}

void Dialog_PasteOptions::on_valuenode_name_edited(const Glib::ustring& path, const Glib::ustring& new_text)
{
	// Check if it is a valid valuenode id
	if (new_text.empty())
		return;
	if (new_text.find_first_of(" :#@$^&()*\t") != new_text.npos)
		return;

	Gtk::TreeIter edited_iter = valuenodes_model->get_iter(path);

	// Will it be linked, not copied?
	bool will_be_copied;
	edited_iter->get_value(COLUMN_COPY_OR_NOT, will_be_copied);
	if (!will_be_copied)
		return;

	// Did the name actually change?
	std::string current_name;
	edited_iter->get_value(COLUMN_NAME, current_name);
	if (current_name == new_text)
		return;

	// Is it a name of another row?
	bool is_currently_unique_name = true;
	valuenodes_model->foreach_iter([=, &is_currently_unique_name](const Gtk::TreeModel::iterator& iter) -> bool {
		std::string data;
		iter->get_value(COLUMN_NAME, data);
		if (data == new_text) {
			is_currently_unique_name = false;
			return true;
		}
		return false;
	});

	if (!is_currently_unique_name)
		return;

	edited_iter->set_value(COLUMN_NAME, new_text);

	refresh_row_status(std::stoul(path));
	update_ok_button_sensitivity();
}

void Dialog_PasteOptions::update_ok_button_sensitivity()
{
	bool enable_button = true;
	if (!valuenodes_model) {
		enable_button = false;
	} else {
		valuenodes_model->foreach_iter([=, &enable_button](const Gtk::TreeModel::iterator& iter) -> bool {
			std::string status;
			iter->get_value(COLUMN_STATUS, status);
			if (status == "conflict") {
				enable_button = false;
				return true;
			}
			return false;
		});
	}

	set_response_sensitive(Gtk::RESPONSE_ACCEPT, enable_button);
	set_response_sensitive(Gtk::RESPONSE_APPLY, enable_button);
	set_response_sensitive(Gtk::RESPONSE_OK, enable_button);
}

void Dialog_PasteOptions::clear()
{
	if (valuenodes_model)
		valuenodes_model->clear();
	update_ok_button_sensitivity();
}

void Dialog_PasteOptions::rebuild_model()
{
	if (!valuenodes_model)
		return;

	clear();

	if (!destination_canvas)
		return;

	for (auto v : value_nodes) {
		if (!v->is_exported())
			continue;
		if (v->get_root_canvas() == destination_canvas->get_root())
			continue;

		auto iter = valuenodes_model->append();

		iter->set_value(COLUMN_VALUENODE_POINTER, v.get());
		iter->set_value(COLUMN_ORIGINAL_NAME, v->get_id());
		iter->set_value(COLUMN_NAME, v->get_id());
		iter->set_value(COLUMN_FILE_PATH, strprintf("(%s)", v->get_root_canvas()->get_file_name().c_str()));
		iter->set_value(COLUMN_VALUE_TYPE, get_tree_pixbuf(v->get_type()));
		iter->set_value(COLUMN_COPY_OR_NOT, true);
		iter->set_value(COLUMN_IS_EDITABLE, true);
	}

	refresh_status();
	update_ok_button_sensitivity();
}

void Dialog_PasteOptions::refresh_status()
{
	for (size_t i = 0; i < value_nodes.size(); i++) {
		refresh_row_status(i);
	}
}

void Dialog_PasteOptions::refresh_row_status(size_t row_index)
{
	auto v_iter = valuenodes_model->get_iter(std::to_string(row_index));
	if (!v_iter)
		return;

	std::string original_name;
	v_iter->get_value(COLUMN_ORIGINAL_NAME, original_name);

	ValueNode* pv;
	v_iter->get_value(COLUMN_VALUENODE_POINTER, pv);
	ValueNode::LooseHandle v = pv;
	if (!v) {
		error(_("Couldn't find valuenode ID (%s). This shouldn't happen"), original_name.c_str());
		return;
	}

	if (!v->is_exported())
		return;

	if (v->get_root_canvas() == destination_canvas->get_root())
		return;

	// Exported value nodes that have value node v as dependency, and that will be externally linked, not copied
	std::vector<ValueNode::LooseHandle> linked_ascendent_value_nodes;

	valuenodes_model->foreach_iter([v, &linked_ascendent_value_nodes](const Gtk::TreeModel::iterator& w_iter)-> bool {
		ValueNode* w;
		w_iter->get_value(COLUMN_VALUENODE_POINTER, w);
		if (v == w)
			return false;
		bool will_w_be_copied;
		w_iter->get_value(COLUMN_COPY_OR_NOT, will_w_be_copied);
		if (!will_w_be_copied) {
			if (w->is_ancestor_of(v)) {
				linked_ascendent_value_nodes.push_back(w);
			}
		}
		return false;
	});

	bool is_forced_link = !linked_ascendent_value_nodes.empty();

	bool will_be_copied;
	if (is_forced_link)
		will_be_copied = false;
	else
		v_iter->get_value(COLUMN_COPY_OR_NOT, will_be_copied);

	std::string status;
	std::string status_tooltip;

	if (!will_be_copied) {
		status = "external-link";
		if (!is_forced_link) {
			status_tooltip = _("This valuenode will be linked to original file and will depend on such file.");
		} else {
			std::string names;
			for (auto w : linked_ascendent_value_nodes) {
				names += w->get_id() + ", ";
			}
			names.resize(names.size() - 2);
			std::string msg = strprintf(_("Some exported values depend on this one, and you chose to keep them linked to external file.\n"
                           "Therefore, this value must be linked too.\n"
                           "These are the values: %s"), names.c_str());
			status_tooltip = msg;
		}
	} else {
		ValueNode::ConstHandle existent_vn;

		try {
			std::string name;
			v_iter->get_value(COLUMN_NAME, name);
			existent_vn = destination_canvas->value_node_list().find(name, true);
		}  catch (...) {

		}

		if (!existent_vn) {
			status = "";
			status_tooltip = _("This valuenode will be copied to target file and will be independent.");
		} else {
			if (v->get_type() != existent_vn->get_type()) {
				status = "conflict"; // different value type
				const char *format = _("There is an exported value with same name ('%s') whose value type is %s, "
				                        "but you are trying to paste one whose value type is %s.\n"
				                        "Please rename it or choose to link it or cancel the whole copying.");
				status_tooltip = strprintf(format,
												v->get_id().c_str(),
												v->get_type().description.local_name.c_str(),
												existent_vn->get_type().description.local_name.c_str());
			} else if (v->get_name() != existent_vn->get_name()) {
				status = "conflict"; // different value node type
				const char *format = _("There is an exported value with same name ('%s') whose value node type is %s, "
				                       "but you are trying to paste one whose type is %s.\n"
				                       "Please rename it or choose to link it or cancel the whole copying.");
				status_tooltip = strprintf(format,
												v->get_id().c_str(),
												v->get_local_name().c_str(),
												existent_vn->get_local_name().c_str());
			} else  {
				status = "link";
				status_tooltip = _("This valuenode will be copied and reuse the existent value node in target file.\n"
				                   "It will not be linked to original file nor will it depend on such file.");
			}

		}
	}

	Glib::RefPtr<Gdk::Pixbuf> status_icon;
	if (status == "conflict")
		status_icon = pixbuf_conflict;
	else if (status == "link")
		status_icon = pixbuf_link;
	else if (status == "external-link")
		status_icon = pixbuf_external_link;
	else
		status_icon = pixbuf_empty;

	v_iter->set_value(COLUMN_IS_NAME_EDITABLE, will_be_copied);
	v_iter->set_value(COLUMN_STATUS, status);
	v_iter->set_value(COLUMN_STATUS_ICON, status_icon);
	v_iter->set_value(COLUMN_STATUS_TOOLTIP, status_tooltip);
	v_iter->set_value(COLUMN_FILE_PATH_VISIBILITY, true);
	if (!will_be_copied)
		v_iter->set_value(COLUMN_NAME, original_name);
	v_iter->set_value(COLUMN_IS_EDITABLE, !is_forced_link);
	if (is_forced_link)
		v_iter->set_value(COLUMN_COPY_OR_NOT, false);
}
