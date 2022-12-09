/* === S Y N F I G ========================================================= */
/*! \file actionmanager.h
**  \brief Header for a visual info database for GActions
**
**  \legal
**  Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2022 Synfig Contributors
**
**  This file is part of Synfig.
**
**  Synfig is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  Synfig is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**  \endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef SYNFIG_STUDIO_ACTIONMANAGER_H
#define SYNFIG_STUDIO_ACTIONMANAGER_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <string>
#include <vector>

//#include <giomm/action.h>
#include <gtkmm/application.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class ActionManager
{
public:

	class Entry
	{
	public:
		std::string name_;
		std::string label_;
		std::vector<Glib::ustring> accelerators_;
		std::string icon_;
		std::string synfigapp_name_;
//		Glib::RefPtr<Gio::Action> action_;
		std::string group_;
		std::string tooltip_;
		std::string l10n_domain_;

		Entry() = default;
		Entry(const std::string& action_name, const std::string& label, const std::vector<Glib::ustring>& accelerators);
		Entry(const std::string& action_name, const std::string& label, const std::string& accelerator);
		Entry(const std::string& action_name, const std::string& label, const std::string& accelerator, const std::string& icon);
		static Entry create_from_synfigapp_action(const std::string& action_name, const std::string& accelerator);

		std::string get_menu_item_string(Glib::RefPtr<Gtk::Application> app) const;
		std::string get_tooltip(Glib::RefPtr<Gtk::Application> app) const;
		std::string get_actual_group() const;

		std::string get_standard_icon_name_for_state(const std::string& state) const;
		std::string get_standard_icon_name_for_layer(const std::string& layer) const;
		std::string get_standard_icon_name_for_type(const std::string& type) const;
		std::string get_standard_icon_name_for_synfigapp_action(const std::string& action) const;
	};

	typedef std::vector<Entry> EntryList;

	ActionManager(Glib::RefPtr<Gtk::Application> app);

	const Entry& get(const std::string& name) const;
	void add(const Entry& entry);

	std::vector<std::string> get_groups() const;
	EntryList get_entries() const;
	EntryList get_entries_for_group(const std::string& group) const;

private:
	Glib::RefPtr<Gtk::Application> app_;
	std::map<std::string, Entry> book_;
};

class UserShortcutList
{
public:
	typedef std::map<std::string, Glib::ustring> List;

	bool load_from_file(const std::string& file, bool force_reset = true);
	bool load_from_string(const std::string& contents, bool force_reset = true);

	bool save_to_file(const std::string& file) const;
	std::string get_string() const;

	bool restore_to_defaults(const ActionManager& actions);
	bool restore_to_defaults_and_apply(Glib::RefPtr<Gtk::Application> app, const ActionManager& actions);

	void apply(Glib::RefPtr<Gtk::Application> app, const ActionManager& actions) const;

	List shortcuts;
};

} // END of namespace studio

/* === E N D =============================================================== */

#endif // SYNFIG_STUDIO_ACTIONMANAGER_H
