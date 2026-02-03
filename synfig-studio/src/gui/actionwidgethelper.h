/* === S Y N F I G ========================================================= */
/*! \file actionwidgethelper.h
**  \brief Header for easier creation of widgets with GActions
**
**  \legal
**  Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2026 Synfig Contributors
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

#ifndef SYNFIG_STUDIO_ACTIONWIDGETHELPER_H
#define SYNFIG_STUDIO_ACTIONWIDGETHELPER_H

/* === H E A D E R S ======================================================= */

#include <string>

#include <giomm/menuitem.h>

#include <gtkmm/togglebutton.h>
#include <gtkmm/toggletoolbutton.h>
#include <gtkmm/toolbutton.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === P R O C E D U R E S ================================================= */

namespace ActionWidgetHelper
{

Glib::RefPtr<Gio::MenuItem> create_menu_item_for_action(const std::string& action_name);
Glib::RefPtr<Gio::MenuItem> create_menu_item_for_action(const std::string& action_name, const std::string& icon_name, const std::string& label);
Glib::RefPtr<Gio::MenuItem> create_menu_item_for_synfigapp_action(const std::string& group_prefix, const std::string& action_name, const std::string& icon_name = ">default<");

Gtk::ToolButton* create_action_toolbutton(const std::string& action_name);
Gtk::ToolButton* create_action_toolbutton(const std::string& action_name, const std::string& icon_name, const std::string& label, const std::string& tooltip);
Gtk::ToolButton* create_synfigapp_action_toolbutton(const std::string& group_prefix, const std::string& action_name);

Gtk::ToggleToolButton* create_action_toggletoolbutton(const std::string& action);
Gtk::ToggleToolButton* create_action_toggletoolbutton(const std::string& action, const std::string& icon_name, const std::string& label, const std::string& tooltip);

Gtk::ToggleButton* create_action_togglebutton(const std::string& action);
Gtk::ToggleButton* create_action_togglebutton(const std::string& action, const std::string& icon_name, const std::string& label, const std::string& tooltip);

void init_toolbutton(Gtk::ToolButton& button, const std::string& action, const std::string& icon_name, const std::string& label, const std::string& tooltip);

void init_button(Gtk::Button& button, const std::string& action_name);
void init_label_only_button(Gtk::Button& button, const std::string& action_name);
void init_icon_only_button(Gtk::Button& button, const std::string& action_name);
void init_button(Gtk::Button& button, const std::string& action_name, const std::string& icon_name, const std::string& label, const std::string& tooltip);

};

/* === C L A S S E S & S T R U C T S ======================================= */

#endif // SYNFIG_STUDIO_ACTIONWIDGETHELPER_H
