/* === S Y N F I G ========================================================= */
/*! \file actionwidgethelper.cpp
**  \brief Implementation of easier creation of widgets with GActions
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2026 Synfig Contributors
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

# include "actionwidgethelper.h"

# include <giomm/themedicon.h>

# include <synfig/general.h>
# include <synfigapp/action.h>
# include <gui/actionmanagers/actionmanager.h>
# include <gui/app.h>
# include <gui/iconcontroller.h>
# include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */
/* === G L O B A L S ======================================================= */
/* === C L A S S E S ======================================================= */
/* === P R O C E D U R E S ================================================= */

Glib::RefPtr<Gio::MenuItem>
ActionWidgetHelper::create_menu_item_for_action(const std::string& action_name)
{
	try {
		ActionDatabase::Entry action_entry = App::get_action_database()->get(action_name);
		return create_menu_item_for_action(action_name, action_entry.icon_, action_entry.label_);
	} catch (...) {
		synfig::warning(_("Couldn't find action: %s"), action_name.c_str());
	}

	return {};
}

Glib::RefPtr<Gio::MenuItem>
ActionWidgetHelper::create_menu_item_for_action(const std::string& action_name, const std::string& icon_name, const std::string& label)
{
	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";
	auto item = Gio::MenuItem::create(label, action_name);
	if (!item)
		return item;
	if (!icon_name.empty())
		item->set_icon(Gio::ThemedIcon::create(icon_name + symbolic_suffix));
	return item;
}

Glib::RefPtr<Gio::MenuItem>
ActionWidgetHelper::create_menu_item_for_synfigapp_action(const std::string& group_prefix, const std::string& action_name, const std::string& icon_name)
{
	auto action_it = synfigapp::Action::book().find(action_name);
	if (action_it == synfigapp::Action::book().end()) {
		synfig::error(_("Internal error: can't find synfigapp action to create its menu item: '%s'"), action_name.c_str());
		return {}; // FIXME: SHOULD RETURN NULL OR an empty MenuItem?
	}

	const std::string symbolic_suffix = ""; // App::use-symbolic-icons ? "-symbolic" : "";
	const std::string full_action_name = synfig::strprintf("%s.action-%s", group_prefix.c_str(), action_name.c_str());
	auto item = Gio::MenuItem::create(action_it->second.local_name, full_action_name);
	if (icon_name == ">default<")
		item->set_icon(Gio::ThemedIcon::create(studio::get_action_icon_name(action_it->second) + symbolic_suffix));
	else if (!icon_name.empty())
		item->set_icon(Gio::ThemedIcon::create(icon_name + symbolic_suffix));
	return item;
}

Gtk::ToolButton*
ActionWidgetHelper::create_action_toolbutton(const std::string& action_name)
{
	try {
		ActionDatabase::Entry action_entry = App::get_action_database()->get(action_name);
		return create_action_toolbutton(action_name, action_entry.icon_, "", action_entry.get_tooltip(App::instance()));
	} catch (...) {
		synfig::warning(_("Couldn't find action: %s"), action_name.c_str());
	}

	return {};
}

Gtk::ToolButton*
ActionWidgetHelper::create_action_toolbutton(const std::string& action_name, const std::string& icon_name, const std::string& label, const std::string& tooltip)
{
	Gtk::ToolButton* button = Gtk::manage(new Gtk::ToolButton());
	if (button)
		ActionWidgetHelper::init_toolbutton(*button, action_name, icon_name, label, tooltip);
	return button;
}

Gtk::ToolButton*
ActionWidgetHelper::create_synfigapp_action_toolbutton(const std::string& group_prefix, const std::string& action_name)
{
	auto action_it = synfigapp::Action::book().find(action_name);
	if (action_it == synfigapp::Action::book().end()) {
		synfig::error(_("Internal error: can't find synfigapp action to create its button: '%s'"), action_name.c_str());
		return nullptr; // FIXME: SHOULD RETURN NULL OR an empty ToolButton?
	}
	const std::string full_action_name = synfig::strprintf("%s.action-%s", group_prefix.c_str(), action_name.c_str());
	return ActionWidgetHelper::create_action_toolbutton(full_action_name, studio::get_action_icon_name(action_it->second), "", action_it->second.local_name);
}

void
ActionWidgetHelper::init_toolbutton(Gtk::ToolButton& button, const std::string& action_name, const std::string& icon_name, const std::string& label, const std::string& tooltip)
{
	if (!icon_name.empty())
		button.set_icon_name(icon_name);
	if (!label.empty())
		button.set_label(label);
	button.set_tooltip_text(tooltip);
	button.show();

	gtk_actionable_set_detailed_action_name(GTK_ACTIONABLE(button.gobj()), action_name.c_str());
}

Gtk::ToggleToolButton*
ActionWidgetHelper::create_action_toggletoolbutton(const std::string& action_name)
{
	try {
		ActionDatabase::Entry action_entry = App::get_action_database()->get(action_name);
		return create_action_toggletoolbutton(action_name, action_entry.icon_, "", action_entry.get_tooltip(App::instance()));
	} catch (...) {
		synfig::warning(_("Couldn't find action: %s"), action_name.c_str());
	}

	return {};
}

Gtk::ToggleToolButton*
ActionWidgetHelper::create_action_toggletoolbutton(const std::string& action_name, const std::string& icon_name, const std::string& label, const std::string& tooltip)
{
	Gtk::ToggleToolButton* button = manage(new Gtk::ToggleToolButton());
	if (button)
		init_toolbutton(*button, action_name, icon_name, label, tooltip);
	return button;
}

Gtk::ToggleButton*
ActionWidgetHelper::create_action_togglebutton(const std::string& action_name)
{
	try {
		ActionDatabase::Entry action_entry = App::get_action_database()->get(action_name);
		return create_action_togglebutton(action_name, action_entry.icon_, "", action_entry.get_tooltip(App::instance()));
	} catch (...) {
		synfig::warning(_("Couldn't find action: %s"), action_name.c_str());
	}

	return {};
}

Gtk::ToggleButton*
ActionWidgetHelper::create_action_togglebutton(const std::string& action_name, const std::string& icon_name, const std::string& label, const std::string& tooltip)
{
	Gtk::ToggleButton* button = manage(new Gtk::ToggleButton());
	if (!button)
		return nullptr;
	button->set_tooltip_text(tooltip);
	if (!icon_name.empty())
		button->set_image_from_icon_name(icon_name);
	if (!label.empty())
		button->set_label(label);
	button->set_relief(Gtk::RELIEF_NONE);
	button->set_active();
	button->show();

	gtk_actionable_set_detailed_action_name(GTK_ACTIONABLE(button->gobj()), action_name.c_str());

	return button;
}

/* === M E T H O D S ======================================================= */
