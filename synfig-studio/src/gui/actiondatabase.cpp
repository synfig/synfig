/* === S Y N F I G ========================================================= */
/*! \file actiondatabase.cpp
**  \brief Implementation of a visual info database for GActions
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2022 Synfig Contributors
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

# include "actiondatabase.h"

# include <algorithm>
# include <set>

# include <glibmm/fileutils.h>
# include <glibmm/markup.h>

# include <synfig/general.h>
# include <synfig/string_helper.h>

# include <gui/localization.h>
#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */
/* === G L O B A L S ======================================================= */
/* === C L A S S E S ======================================================= */
/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ActionDatabase::Entry::Entry(const std::string& action_name, const std::string& label, const std::vector<Glib::ustring>& accelerators)
	: name_(action_name), label_(label), accelerators_(accelerators)
{
}

ActionDatabase::Entry::Entry(const std::string& action_name, const std::string& label, const std::string& accelerator)
	: name_(action_name), label_(label), accelerators_({accelerator})
{
}

ActionDatabase::Entry::Entry(const std::string& action_name, const std::string& label, const std::string& accelerator, const std::string& icon)
	: name_(action_name), label_(label), accelerators_({accelerator}), icon_(icon)
{
}

ActionDatabase::Entry::Entry(const std::string& action_name, const std::string& label, const std::string& accelerator, const std::string& icon, const std::string& tooltip)
	: name_(action_name), label_(label), accelerators_({accelerator}), icon_(icon), tooltip_(tooltip)
{
}

std::string
ActionDatabase::Entry::get_menu_item_string(Glib::RefPtr<Gtk::Application> app) const
{
	const bool has_accel = !accelerators_.empty();
	const std::string accel = has_accel ? accelerators_.front() : "";

	std::string res;
	res += "<attribute name='label' translatable='yes'>" + get_label() + "</attribute>";
	res += "<attribute name='action'>" + name_ + "</attribute>";

	if (has_accel)
		res += "<attribute name='accel'>" + Glib::Markup::escape_text(accel) + "</attribute>";

	if (!icon_.empty())
		res += "<attribute name='icon'>" + icon_ + "</attribute>";

	std::string tooltip = get_tooltip(app);
	if (!tooltip.empty())
		res += "<attribute name='tooltip'>" + get_tooltip(app) + "</attribute>";
	return res;
}

std::string
ActionDatabase::Entry::get_tooltip(Glib::RefPtr<Gtk::Application> app) const
{
	const std::string l10n_domain = l10n_domain_.empty() ? GETTEXT_PACKAGE : l10n_domain_;

	const auto app_accels = app ? app->get_accels_for_action(name_) : std::vector<Glib::ustring>();
	const bool has_accel = app ? !app_accels.empty() : !accelerators_.empty();
	const std::string accel = !has_accel ? "" : (app ? app_accels.front() : accelerators_.front());

	std::string tooltip = dgettext(l10n_domain.c_str(), (tooltip_.empty() ? label_ : tooltip_).c_str());

	if (has_accel && !accel.empty()) {
		Gtk::AccelKey accel_key(accel);
		tooltip += "  " + Gtk::AccelGroup::get_label(accel_key.get_key(), accel_key.get_mod());
	}
	return tooltip;
}

std::string
ActionDatabase::Entry::get_label() const
{
	const std::string l10n_domain = l10n_domain_.empty() ? GETTEXT_PACKAGE : l10n_domain_;

	return dgettext(l10n_domain.c_str(), label_.c_str());
}

std::string
ActionDatabase::Entry::get_actual_group() const
{
	return name_.substr(0, name_.find('.'));
}

ActionDatabase::ActionDatabase()
{
}

bool
ActionDatabase::has(const std::string& name) const
{
	return book_.find(name) != book_.cend();
}

const ActionDatabase::Entry&
ActionDatabase::get(const std::string& name) const
{
	return book_.at(name);
}

void
ActionDatabase::add(const Entry& entry)
{
	const auto it = book_.find(entry.name_);
	bool is_new_entry = it == book_.cend();

	book_[entry.name_] = entry;

	if (is_new_entry) {
		signal_new_entry_added.emit(entry);
	} else {
		signal_entry_changed.emit(it->second);
		synfig::warning(_("Replacing action in Action Database: %s"), entry.name_.c_str());
	}
}

void
ActionDatabase::merge(const ActionDatabase& adb)
{
	for (const auto& entry : adb.book_) {
		add(entry.second);
	}
}

std::vector<std::string>
ActionDatabase::get_groups() const
{
	std::vector<std::string> groups;
	std::set<std::string> group_set;
	for (const auto& item : book_) {
		const Entry& entry = item.second;
		group_set.insert(entry.get_actual_group());
	}
	groups.assign(group_set.begin(), group_set.end());
	return groups;
}

ActionDatabase::EntryList
ActionDatabase::get_entries() const
{
	EntryList v;
	std::transform(book_.begin(), book_.end(),
				   std::back_inserter(v),
				   [](const std::pair<std::string, Entry> &p) {
					   return p.second;
				   });
	return v;
}

ActionDatabase::EntryList
ActionDatabase::get_entries_for_group(const std::string& group) const
{
	EntryList entries;
	for (auto item : book_) {
		if (item.second.get_actual_group() == group)
			entries.push_back(item.second);
	}
	return entries;
}

bool
UserAcceleratorList::load_from_file(const synfig::filesystem::Path& file, bool force_reset)
{
	try {
		std::string contents = Glib::file_get_contents(file.u8string());
		return load_from_string(contents, force_reset);
	} catch (const Glib::FileError& error) {
		synfig::error(_("accelerator file: error loading from file %s: %s"), file.c_str(), error.what().c_str());
		return false;
	} catch (...) {
		return false;
	}
}

bool
UserAcceleratorList::load_from_string(const std::string& contents, bool force_reset)
{
	if (force_reset)
		accels.clear();

	std::string::size_type pos = 0;
	std::string::size_type length = contents.length();
	unsigned int line_num = 0;
	while (pos != std::string::npos && pos < length) {
		auto end = contents.find_first_of("\r\n", pos);
		std::string line = synfig::trim(contents.substr(pos, end == std::string::npos ? end : end - pos));

		++line_num;

		if (end != std::string::npos)
			pos = contents.find_first_not_of("\r\n", end);
		else
			pos = end;

		if (line.empty() || line.front() == ';')
			continue;

		if (line.front() != '"') {
			synfig::error(_("accelerator file: malformed line: #%u: action must be quoted"), line_num);
			continue;
		}

		auto action_end = line.find('"', 1);
		auto accel_pos = line.find('"', action_end + 1);
		auto accel_end = line.find('"', accel_pos + 1);
		if (action_end == std::string::npos || accel_pos == std::string::npos || accel_end == std::string::npos) {
			synfig::error(_("accelerator file: malformed line: #%u: you must quote both action and accelerator"), line_num);
			continue;
		}
		std::string action, accelerator;
		action = line.substr(1, action_end - 1);
		accelerator = line.substr(accel_pos + 1, accel_end - accel_pos - 1);
		if (action.empty()) {
			synfig::error(_("accelerator file: malformed line: #%u: empty action name"), line_num);
			continue;
		}
		if (!synfig::trim(line.substr(action_end + 1, accel_pos - action_end - 1)).empty()) {
			synfig::error(_("accelerator file: malformed line: #%u: garbage after action name"), line_num);
			continue;
		}
		if (accel_end != line.length() - 1) {
			synfig::warning(_("accelerator file: malformed line: #%u: garbage after accelerator"), line_num);
		}

		accels[action] = accelerator;

		if (end != std::string::npos)
			pos = contents.find_first_not_of("\r\n", end);
		else
			pos = end;
	}
	return true;
}

bool
UserAcceleratorList::save_to_file(const synfig::filesystem::Path& file) const
{
	try {
		std::string contents = get_string();
		Glib::file_set_contents(file.u8string(), contents);
		return true;
	} catch (const Glib::FileError& error) {
		synfig::error(_("accelerator file: error saving to file %s: %s"), file.c_str(), error.what().c_str());
		return false;
	} catch (...) {
		return false;
	}
}

std::string
UserAcceleratorList::get_string() const
{
	std::string res;
	for (const auto& item : accels) {
		res += '"' + item.first + "\" \"" + item.second + "\"\n";
	}
	return res;
}

bool
UserAcceleratorList::restore_to_defaults(const ActionDatabase& actions)
{
	accels.clear();

	for (const auto& item : actions.get_entries()) {
		accels[item.name_] = item.accelerators_.empty() ? "" : item.accelerators_[0];
	}
	return true;
}

bool
UserAcceleratorList::restore_to_defaults_and_apply(Glib::RefPtr<Gtk::Application> app, const ActionDatabase& actions)
{
	if (!app)
		return false;

	restore_to_defaults(actions);

	auto action_list = app->list_action_descriptions();
	for (const auto& item_name : action_list)
		app->unset_accels_for_action(item_name);

	for (const auto& item : actions.get_entries()) {
		if (item.accelerators_.empty() || item.accelerators_.front().empty())
			app->unset_accels_for_action(item.name_);
		else
			app->set_accels_for_action(item.name_, item.accelerators_);
	}
	return true;
}

void
UserAcceleratorList::apply(Glib::RefPtr<Gtk::Application> app, const ActionDatabase& action_db) const
{
	if (!app) {
		synfig::error(_("accelerator file: internal error: app is null. Please report."));
		return;
	}

	for (const auto& entry : action_db.get_entries())
		app->unset_accels_for_action(entry.name_);

	for (const auto& item : accels) {
		const std::string& action_name = item.first;
		const std::string& accelerator = item.second;
		if (action_name.empty())
			continue;
		if (accelerator.empty())
			app->unset_accels_for_action(action_name);
		else
			app->set_accels_for_action(action_name, {accelerator});
	}
}

UserAcceleratorList::AcceleratorList
UserAcceleratorList::get_custom_accels_only(Glib::RefPtr<Gtk::Application> app, const ActionDatabase& action_db)
{
	AcceleratorList list;

	if (!app) {
		synfig::error(_("accelerator file: internal error: app is null. Please report."));
		return list;
	}

	for (const auto& entry : action_db.get_entries()) {
		auto user_accels = app->get_accels_for_action(entry.name_);
		if (user_accels.empty() && entry.accelerators_.empty()) {
			continue;
		} else if (user_accels.empty() && !entry.accelerators_.empty()) {
			if (!entry.accelerators_[0].empty())
				list[entry.name_] = "";
		} else if (!user_accels.empty() && entry.accelerators_.empty()) {
			if (!user_accels[0].empty())
				list[entry.name_] = user_accels[0];
		} else {
			std::sort(user_accels.begin(), user_accels.end());
			auto default_accels = entry.accelerators_;
			std::sort(default_accels.begin(), default_accels.end());
			if (user_accels[0] != default_accels[0]) {
				list[entry.name_] = user_accels[0];
			}
		}
	}
	return list;
}
