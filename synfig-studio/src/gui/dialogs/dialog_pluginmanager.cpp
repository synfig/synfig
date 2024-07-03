/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_pluginmanager.h
**	\brief Implementation of dialog to install, uninstall and manage plugins
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

#include "dialog_pluginmanager.h"

#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>
#include <gui/localization.h>


#include <synfig/general.h>
#include <synfig/os.h>

using namespace synfig;
using namespace studio;


Dialog_PluginManager::Dialog_PluginManager(Gtk::Window& parent):
    Gtk::Dialog(_("Plugin Manager"), parent),
    pluginList(App::plugin_manager.plugins())
{
    // this->set_resizable(false);

    Gtk::Button *installPluginButton = manage(new Gtk::Button(_("Install Plugin")));
    installPluginButton->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
    installPluginButton->set_always_show_image(true);
    installPluginButton->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_PluginManager::clicked));

    Gtk::VBox *pluginTab = manage(new Gtk::VBox());
    pluginTab->property_expand().set_value(false);
    Gtk::ListBox *pluginListBox = manage(new Gtk::ListBox());
    pluginListBox->set_selection_mode(Gtk::SELECTION_NONE);

    for (auto plugin : pluginList)
    {
        synfig::info("  Plugin Dir: %s", plugin.pluginDir.c_str() );
        Gtk::HBox *pluginListItem = manage(new Gtk::HBox());
        pluginListItem->get_style_context()->add_class("plugin-list-item");
        Gtk::Label *pluginName = manage(new Gtk::Label());

        Gtk::HBox *pluginOptionBox = manage(new Gtk::HBox());
        Gtk::Button *restoreSettings = manage(new Gtk::Button());
        Gtk::Button *openFolder = manage(new Gtk::Button());
        Gtk::Button *deletePlugin = manage(new Gtk::Button());

        pluginListItem->property_expand().set_value(false);
        // pluginListItem->override_background_color(*(new Gdk::RGBA("rgb(242, 242, 242)")));
        pluginListItem->set_margin_top(10);
        pluginOptionBox->property_expand().set_value(false);

        restoreSettings->set_image_from_icon_name("view-refresh", Gtk::ICON_SIZE_BUTTON);
        openFolder->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
        deletePlugin->set_image_from_icon_name("user-trash-symbolic", Gtk::ICON_SIZE_BUTTON);

        openFolder->signal_clicked().connect([plugin](){
            plugin.launch_dir();
        });

        pluginOptionBox->pack_start(*restoreSettings, Gtk::PACK_SHRINK, 10);
        pluginOptionBox->pack_start(*openFolder, Gtk::PACK_SHRINK, 10);
        pluginOptionBox->pack_start(*deletePlugin, Gtk::PACK_SHRINK, 10);
        pluginOptionBox->set_margin_top(10);
        pluginOptionBox->set_margin_bottom(10);

        pluginName->set_text(plugin.name.get());

        pluginListItem->pack_start(*pluginName, Gtk::PACK_SHRINK, 20);
        pluginListItem->pack_end(*pluginOptionBox, Gtk::PACK_SHRINK, 20);
        
        pluginListBox->add(*pluginListItem);

    }
    pluginTab->pack_start(*pluginListBox, Gtk::PACK_SHRINK);
    pluginTab->pack_end(*installPluginButton, Gtk::PACK_SHRINK);

    Gtk::Label *customLabel = manage(new Gtk::Label());
    customLabel->set_text("Installed Plugins");
    customLabel->set_margin_bottom(10);
    customLabel->set_margin_top(10);
    customLabel->set_margin_left(20);
    customLabel->set_margin_right(20);
    notebook.append_page(*pluginTab);
    notebook.set_tab_label(*pluginTab, *customLabel);
    
    notebook.set_tab_pos(Gtk::POS_LEFT);
    get_content_area()->pack_start(notebook);
    set_default_size(1200, 600);
    show_all_children();
}

Dialog_PluginManager::~Dialog_PluginManager()
{
    
}

void Dialog_PluginManager::clicked()
{
    synfig::info("Clicked");

}

bool Dialog_PluginManager::open_directory(const std::string& path)
{
    return synfig::OS::launch_file_async(path);
}