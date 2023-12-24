/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_fixmissingfiles.cpp
**	\brief Implementation of Dialog_FixMissingFiles
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2023-     Synfig Contributors
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
# include "pch.h"
#else
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

# include "dialog_fixmissingfiles.h"

# include <gtkmm/button.h>
# include <gtkmm/image.h>
# include <gtkmm/label.h>
# include <gtkmm/listbox.h>

# include <synfig/string_helper.h>

# include <gui/app.h>
# include <gui/localization.h>
# include <gui/resourcehelper.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static Gtk::Image*
create_image_from_icon(const std::string& icon_name, Gtk::IconSize icon_size)
{
#if GTK_CHECK_VERSION(3,24,0)
	return new Gtk::Image(icon_name, icon_size);
#else
	Gtk::Image* image = new Gtk::Image();
	image->set_from_icon_name(icon_name, icon_size);
	return image;
#endif
}

/* === M E T H O D S ======================================================= */

std::shared_ptr<Dialog_FixMissingFiles>
Dialog_FixMissingFiles::create(Gtk::Window& parent)
{
	auto refBuilder = ResourceHelper::load_interface("dialog_fixmissingfiles.glade");
	if (!refBuilder)
		return nullptr;
	Dialog_FixMissingFiles * dialog_ptr = nullptr;
	refBuilder->get_widget_derived("dialog_fixmissingfiles", dialog_ptr);
	if (dialog_ptr) {
		dialog_ptr->set_transient_for(parent);
	}
	std::shared_ptr<Dialog_FixMissingFiles> dialog(dialog_ptr);
	return dialog;
}

Dialog_FixMissingFiles::Dialog_FixMissingFiles(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
	: Gtk::Dialog(cobject),
	  builder_(refGlade)
{
	if (!builder_)
		return;

	missing_file_list_ = Glib::RefPtr<Gtk::ListBox>::cast_dynamic(builder_->get_object("missing_file_list"));

	signal_response().connect(sigc::mem_fun(*this, &Dialog_FixMissingFiles::on_response));
}

void
Dialog_FixMissingFiles::set_broken_useids(synfig::CanvasBrokenUseIdMap& map)
{
	map_ = &map;
	replacer_map_.clear();

	if (!missing_file_list_)
		return;

	auto rows = missing_file_list_->get_children();
	for (const auto& row : rows)
		missing_file_list_->remove(*row);

	for (auto iter = map_->begin(); iter != map.end(); ++iter) {
		replacer_map_[iter->first] = iter->second.replacement;

		create_row(replacer_map_, iter);
	}

	Gtk::Button* button = static_cast<Gtk::Button*>(get_widget_for_response(Gtk::RESPONSE_OK));
	button->set_sensitive(is_replacer_map_complete());
}

void
Dialog_FixMissingFiles::on_response(int response_id)
{
	if (response_id != Gtk::RESPONSE_OK)
		return;

	for (auto& item : *map_) {
		item.second.replacement = replacer_map_.at(item.first);
	}
}

void
Dialog_FixMissingFiles::create_row(Dialog_FixMissingFiles::FileReplacerMap& replacer_map, const synfig::CanvasBrokenUseIdMap::iterator& iter)
{
	synfig::filesystem::Path missing_path(iter->first.u8string());
	const synfig::BrokenUseIdInfo& uses = iter->second;

	auto box = Gtk::manage(new Gtk::Box());
	box->set_hexpand(true);

	if (!synfig::FileSystemNative::instance()->is_file(missing_path.u8string())) {
		auto icon = Gtk::manage(create_image_from_icon("image-missing", Gtk::ICON_SIZE_MENU));
		icon->set_tooltip_text("Missing file");
		box->pack_start(*icon);
	}

	auto label = Gtk::manage(new Gtk::Label(missing_path.u8string()));
	label->set_hexpand(true);
	label->set_xalign(0);
	label->set_ellipsize(Pango::ELLIPSIZE_START);
	std::string reasons = _("Issues:");
	for (const auto& item : iter->second.missing_items) {
		reasons += '\n';
		if (item.id.empty()) {
			reasons += _("- Missing file");
		} else {
			reasons += synfig::strprintf(_("- Missing exported valuenode: %s (%s)"), item.id.c_str(), item.type_name.c_str());
		}
	}
	label->set_tooltip_text(missing_path.u8string() + "\n\n" + reasons);
	box->pack_start(*label);

	label = Gtk::manage(new Gtk::Label());
	label->set_hexpand(true);
	label->set_xalign(0);
	label->set_ellipsize(Pango::ELLIPSIZE_START);
	if (!uses.replacement.empty()) {
		label->set_markup(synfig::strprintf("(<small>%s</small>)", uses.replacement.u8_str()));
		label->set_tooltip_text(uses.replacement.u8string());
	}
	box->pack_start(*label);

	auto button = Gtk::manage(new Gtk::Button(_("Change")));
	button->set_hexpand(false);
	box->pack_end(*button, Gtk::PACK_SHRINK);

	button->signal_clicked().connect(sigc::track_obj([this, label, missing_path, &replacer_map]() {
		synfig::filesystem::Path replacement(missing_path);
		if (App::dialog_open_file(replacer_map[missing_path].u8string(), replacement, "file")) {
			label->set_markup(synfig::strprintf("(<small>%s</small>)", replacement.u8_str()));
			label->set_tooltip_text(replacement.u8string());
			replacer_map[missing_path] = replacement;
		}
		Gtk::Button* button = static_cast<Gtk::Button*>(get_widget_for_response(Gtk::RESPONSE_OK));
		button->set_sensitive(is_replacer_map_complete());
	}, *label));

	box->show_all();
	missing_file_list_->append(*box);
}

bool
Dialog_FixMissingFiles::is_replacer_map_complete() const
{
	for (const auto& item : replacer_map_) {
		if (item.second.empty())
			return false;
	}
	return true;
}
