/* === S Y N F I G ========================================================= */
/*!	\file dialog_pluginmanager.cpp
**	\brief Plugin Manager Dialog
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

#include "dialog_pluginmanager.h"

#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>

#include <gui/localization.h>
#include <gui/app.h>

#include <synfig/general.h>
#include <synfig/os.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_PluginManager::Dialog_PluginManager(Gtk::Window& parent):
    Gtk::Dialog(_("Plugin Manager"), parent),
    message_dialog(_("Are you sure you want to delete this plugin?"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true),
    plugin_list(App::plugin_manager.plugins())
{
    // this->set_resizable(false);
    message_dialog.set_transient_for(*this);
    Gtk::Button* install_plugin_button = manage(new Gtk::Button(_("Install Plugin")));
    install_plugin_button->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
    install_plugin_button->set_always_show_image(true);
    install_plugin_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_PluginManager::on_install_plugin_button_clicked));

    Gtk::VBox* plugin_tab = manage(new Gtk::VBox());
    plugin_tab->property_expand().set_value(false);
    Gtk::ListBox* plugin_list_box = manage(new Gtk::ListBox());
    plugin_list_box->set_selection_mode(Gtk::SELECTION_NONE);

    for (const auto plugin : plugin_list) {
        synfig::info("  Plugin Dir: %s", plugin.pluginDir.c_str() );
        Gtk::HBox* plugin_list_item = manage(new Gtk::HBox());
        plugin_list_item->get_style_context()->add_class("plugin-list-item");
        Gtk::Label* plugin_name = manage(new Gtk::Label());

        Gtk::HBox* plugin_option_box = manage(new Gtk::HBox());
        Gtk::Button* restore_settings = manage(new Gtk::Button());
        Gtk::Button* open_folder = manage(new Gtk::Button());
        Gtk::Button* delete_plugin = manage(new Gtk::Button());

        plugin_list_item->property_expand().set_value(false);
        // plugin_list_item->override_background_color(*(new Gdk::RGBA("rgb(242, 242, 242)")));
        plugin_list_item->set_margin_top(10);
        plugin_option_box->property_expand().set_value(false);

        restore_settings->set_image_from_icon_name("view-refresh", Gtk::ICON_SIZE_BUTTON);
        open_folder->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
        delete_plugin->set_image_from_icon_name("user-trash-symbolic", Gtk::ICON_SIZE_BUTTON);

        delete_plugin->signal_clicked().connect([plugin, this](){
            this->message_dialog.set_message(_("Do you want to delete the plugin: ") + plugin.name.get());
            int response = this->message_dialog.run();
            if (response == Gtk::RESPONSE_OK) {              
                App::plugin_manager.remove_plugin(plugin.id);
            }
            this->message_dialog.close();
        });

        open_folder->signal_clicked().connect([plugin](){
            plugin.launch_dir();
        });

        plugin_option_box->pack_start(*restore_settings, Gtk::PACK_SHRINK, 10);
        plugin_option_box->pack_start(*open_folder, Gtk::PACK_SHRINK, 10);
        plugin_option_box->pack_start(*delete_plugin, Gtk::PACK_SHRINK, 10);
        plugin_option_box->set_margin_top(10);
        plugin_option_box->set_margin_bottom(10);

        plugin_name->set_text(plugin.name.get());

        plugin_list_item->pack_start(*plugin_name, Gtk::PACK_SHRINK, 20);
        plugin_list_item->pack_end(*plugin_option_box, Gtk::PACK_SHRINK, 20);
        
        plugin_list_box->add(*plugin_list_item);

    }
    plugin_tab->pack_start(*plugin_list_box, Gtk::PACK_SHRINK);
    plugin_tab->pack_end(*install_plugin_button, Gtk::PACK_SHRINK);

    Gtk::Label* custom_label = manage(new Gtk::Label());
    custom_label->set_text("Installed Plugins");
    custom_label->set_margin_bottom(10);
    custom_label->set_margin_top(10);
    custom_label->set_margin_left(20);
    custom_label->set_margin_right(20);
    notebook.append_page(*plugin_tab);
    notebook.set_tab_label(*plugin_tab, *custom_label);
    
    notebook.set_tab_pos(Gtk::POS_LEFT);
    get_content_area()->pack_start(notebook);
    set_default_size(1200, 600);
    show_all_children();
}

Dialog_PluginManager::~Dialog_PluginManager()
{
    
}

void Dialog_PluginManager::on_install_plugin_button_clicked()
{
    synfig::info("Install button clicked");
}

bool Dialog_PluginManager::open_directory(const std::string& path)
{
    return synfig::OS::launch_file_async(path);
}