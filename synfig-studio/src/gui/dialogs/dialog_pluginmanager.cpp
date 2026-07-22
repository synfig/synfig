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
#include <fstream>
#include <memory>
#include <curl/curl.h>
#include <libxml++/libxml++.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
#include <gtkmm/builder.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/notebook.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/scale.h>
#include <gtkmm/scalebutton.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/spinner.h>
#include <gtkmm/switch.h>
#include <gtkmm/volumebutton.h>

#include <gui/app.h>
#include <gui/resourcehelper.h>

#include <synfig/general.h>
#include <synfigapp/main.h>
#include <synfig/os.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


enum PluginZipStatus 
{
    PLUGIN_ZIP_NO_PLUGIN_XML = 1 << 1,
    PLUGIN_ZIP_AT_ROOT_DIR = 1 << 2,
    PLUGIN_ZIP_AT_CHILD_DIR = 1 << 3
};

size_t write_callback_string(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static void set_widget_value(Gtk::Widget* widget, const std::string& value) {
    if (!widget) return;
    
    if (GTK_IS_COMBO_BOX_TEXT(widget->gobj())) {
        auto* combo = dynamic_cast<Gtk::ComboBoxText*>(widget);
        if (combo->get_has_entry()) {
            if (auto entry = combo->get_entry()) {
                entry->set_text(value);
            }
        }
        else {
            combo->set_active_id(value);
        }
    }
    else if (GTK_IS_COMBO_BOX(widget->gobj())) {
        auto* combo = dynamic_cast<Gtk::ComboBox*>(widget);
        if (combo->get_has_entry()) {
            if (auto entry = combo->get_entry()) {
                entry->set_text(value);
                entry->set_editable(true);
            }
        }
        else
            combo->set_active_id(value);
    }
    else if (GTK_IS_SWITCH(widget->gobj())) {
        dynamic_cast<Gtk::Switch*>(widget)->set_active(value == "1" || value == "true");
    }
    else if (GTK_IS_CHECK_BUTTON(widget->gobj())) {
        dynamic_cast<Gtk::CheckButton*>(widget)->set_active(value == "1" || value == "true");
    }
    else if (GTK_IS_TOGGLE_BUTTON(widget->gobj())) {
        dynamic_cast<Gtk::ToggleButton*>(widget)->set_active(value == "1" || value == "true");
    }
    else if (GTK_IS_FILE_CHOOSER_BUTTON(widget->gobj())) {
        dynamic_cast<Gtk::FileChooserButton*>(widget)->set_filename(value);
    }
    else if (GTK_IS_COLOR_BUTTON(widget->gobj())) {
        Gdk::RGBA color;
        color.set(value);
        dynamic_cast<Gtk::ColorButton*>(widget)->set_rgba(color);
    }
    else if (GTK_IS_FONT_BUTTON(widget->gobj())) {
        dynamic_cast<Gtk::FontButton*>(widget)->set_font_name(value);
    }
    else if (GTK_IS_SCALE_BUTTON(widget->gobj())) {
        dynamic_cast<Gtk::ScaleButton*>(widget)->set_value(std::stod(value));
    }
    else if (GTK_IS_VOLUME_BUTTON(widget->gobj())) {
        dynamic_cast<Gtk::VolumeButton*>(widget)->set_value(std::stod(value));
    }
    else if (GTK_IS_SCALE(widget->gobj())) {
        dynamic_cast<Gtk::Scale*>(widget)->set_value(std::stod(value));
    }
    else if (GTK_IS_SPIN_BUTTON(widget->gobj())) {
        dynamic_cast<Gtk::SpinButton*>(widget)->set_value(std::stod(value));
    }
    else if (GTK_IS_ENTRY(widget->gobj())) {
        dynamic_cast<Gtk::Entry*>(widget)->set_text(value);
    }
}

static void hydrate_config(Gtk::Container* container, const std::map<std::string, std::string>& values) {
    if (!container) return;

    std::vector<Gtk::Widget*> children = container->get_children();
    for (Gtk::Widget* child : children) {
        if (!child->get_name().empty()) {
            auto it = values.find(child->get_name());
            if (it != values.end())
                set_widget_value(child, it->second);
        }
        
        if (GTK_IS_CONTAINER(child->gobj()))
            hydrate_config(dynamic_cast<Gtk::Container*>(child), values);
    }
}

void open_url(const std::string &url)
{
    try
    {
        Gio::AppInfo::launch_default_for_uri(url);
    }
    catch (const Glib::Error &ex)
    {
        std::cerr << "Error opening URL: " << ex.what() << std::endl;

        Gtk::MessageDialog dialog("Failed to open URL",
                                  false,
                                  Gtk::MessageType::MESSAGE_ERROR,
                                  Gtk::ButtonsType::BUTTONS_OK,
                                  true);
        dialog.run();
    }
}

std::vector<PluginInfo> get_plugins(xmlpp::Node &root)
{
    std::vector<PluginInfo> plugins;

    for (xmlpp::Node *pluginNode = root.get_first_child("plugin"); pluginNode; pluginNode = pluginNode->get_next_sibling())
    {
        if (pluginNode->get_name() != "plugin")
        {
            continue;
        }
        PluginInfo plugin;

        for (xmlpp::Node *child = pluginNode->get_first_child(); child; child = child->get_next_sibling())
        {
            if (child->get_name() == "id")
            {
                // Parse ID field
                if (auto textNode = dynamic_cast<xmlpp::TextNode *>(child->get_first_child()))
                {
                    plugin.id = textNode->get_content();
                }
            }
            else if (child->get_name() == "name")
            {
                if (auto textNode = dynamic_cast<xmlpp::TextNode *>(child->get_first_child()))
                {
                    plugin.name = textNode->get_content();
                }
            }
            else if (child->get_name() == "download")
            {
                if (auto textNode = dynamic_cast<xmlpp::TextNode *>(child->get_first_child()))
                {
                    plugin.download = textNode->get_content();
                }
            }
            else if (child->get_name() == "author")
            {
                if (auto textNode = dynamic_cast<xmlpp::TextNode *>(child->get_first_child()))
                {
                    plugin.author = textNode->get_content();
                }
            }
            else if (child->get_name() == "release")
            {
                if (auto textNode = dynamic_cast<xmlpp::TextNode *>(child->get_first_child()))
                {
                    plugin.release = textNode->get_content();
                }
            }
            else if (child->get_name() == "version")
            {
                if (auto textNode = dynamic_cast<xmlpp::TextNode *>(child->get_first_child()))
                {
                    plugin.version = textNode->get_content();
                }
            }
            else if (child->get_name() == "url")
            {
                if (auto textNode = dynamic_cast<xmlpp::TextNode *>(child->get_first_child()))
                {
                    plugin.url = textNode->get_content();
                }
            }
            else if (child->get_name() == "description")
            {
                if (auto textNode = dynamic_cast<xmlpp::TextNode *>(child->get_first_child()))
                {
                    plugin.description = textNode->get_content();
                }
            }
        }

        plugins.push_back(plugin);
    }
    return plugins;
}

size_t write_callback_file(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct FileData *data = (struct FileData *)userdata;
    size_t written = fwrite(ptr, size, nmemb, data->fp);
    data->size += written;
    return written;
}

Dialog_PluginManager::Dialog_PluginManager(Gtk::Window& parent):
    Gtk::Dialog(_("Plugin Manager"), parent),
    plugin_file_dialog(*this, "Select plugin zip file", Gtk::FILE_CHOOSER_ACTION_OPEN),
    message_dialog(*this, _(""), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true),
    confirmation_dialog(*this, _("Are you sure you want to delete this plugin?"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true),
    plugin_list(App::plugin_manager.plugins())
{
    plugin_file_dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
    plugin_file_dialog.add_button(_("Open"), Gtk::RESPONSE_OK);

    auto filter_zip = Gtk::FileFilter::create();
    filter_zip->set_name("Zip File (A synfig plugin zip file)");
    filter_zip->add_mime_type("application/zip");
    plugin_file_dialog.add_filter(filter_zip);

    App::plugin_manager.signal_list_changed().connect([this]() {
        refresh();
        remote_repo_page.update_cards_status();
    });
    confirmation_dialog.set_transient_for(*this);

    build_listbox();
    build_notebook();

    notebook.signal_switch_page().connect(sigc::mem_fun(*this, &Dialog_PluginManager::on_notebook_switch_page));
    notebook.set_scrollable(true);
    notebook.set_hexpand(true);
    set_size_request(1410, 600);
    get_content_area()->pack_start(notebook);
    show_all_children();
}

Dialog_PluginManager::~Dialog_PluginManager()
{
}

void
Dialog_PluginManager::refresh()
{
    plugin_list = App::plugin_manager.plugins();
    auto children = plugin_list_box.get_children();
    for (auto child : children) {
        plugin_list_box.remove(*child);
        delete child;
    }

    build_listbox();
    build_notebook();
}

void
Dialog_PluginManager::save_plugin_config(const std::string& plugin_id, Gtk::Widget* config_widget)
{
    if (!config_widget) return;

    Plugin plugin = App::plugin_manager.get_plugin(plugin_id);
    if (!plugin.is_valid()) return;

    std::string user_config_file = plugin.pluginDir + "/user_config.json";
    auto config_data = PluginManager::parse_dialog(*config_widget);
    
    std::string json_data;
    for (const auto& d : config_data) {
        if (!json_data.empty())
            json_data.push_back(',');
        json_data += synfig::strprintf("\"%s\":\"%s\"", 
            JSON::escape_string(d.first).c_str(), 
            JSON::escape_string(d.second).c_str());
    }
    json_data = "{" + json_data + "}";

    try {
        auto file_system = FileSystemNative::instance();
        auto stream = file_system->get_write_stream(user_config_file);
        if (!stream) {
            throw std::runtime_error(_("Could not open file for writing"));
        }
        stream->write(json_data.c_str(), json_data.size());

        message_dialog.set_message(_("Configuration saved successfully"));
        message_dialog.run();
        message_dialog.hide();
    } catch (const std::exception& ex) {
        message_dialog.set_message(_("Error saving configuration: ") + std::string(ex.what()));
        message_dialog.run();
        message_dialog.hide();
    }
}

void
Dialog_PluginManager::reset_plugin_config(const std::string& plugin_id, Gtk::Widget* config_widget)
{
    if (!config_widget) return;

    confirmation_dialog.set_message(_("Are you sure you want to reset all settings to default?"));
    int response = confirmation_dialog.run();
    confirmation_dialog.hide();

    if (response == Gtk::RESPONSE_OK) {
        Plugin plugin = App::plugin_manager.get_plugin(plugin_id);
        if (!plugin.is_valid()) return;

        std::string default_config_file = plugin.pluginDir + "/default_config.json";
        std::string user_config_file = plugin.pluginDir + "/user_config.json";

        try {
            auto file_system = FileSystemNative::instance();
            
            if (!file_system->is_file(default_config_file)) {
                message_dialog.set_message(_("Error: Default configuration file not found"));
                message_dialog.run();
                message_dialog.hide();
                return;
            }

            auto src_stream = file_system->get_read_stream(default_config_file);
            if (!src_stream) {
                throw std::runtime_error(_("Could not open default configuration file"));
            }

            std::string config_data;
            char buffer[4096];
            while (!src_stream->eof()) {
                src_stream->read(buffer, sizeof(buffer));
                std::streamsize bytes_read = src_stream->gcount();
                if (bytes_read > 0) {
                    config_data.append(buffer, bytes_read);
                }
            }

            auto dst_stream = file_system->get_write_stream(user_config_file);
            if (!dst_stream) {
                throw std::runtime_error(_("Could not open user configuration file for writing"));
            }
            
            if (!config_data.empty()) {
                dst_stream->write(config_data.c_str(), config_data.size());
            }
            dst_stream->flush();
            dst_stream.reset();

            message_dialog.set_message(_("Configuration reset to defaults successfully"));
            message_dialog.run();
            message_dialog.hide();

            // Reload the configuration UI
            build_notebook();
        } catch (const std::exception& ex) {
            message_dialog.set_message(_("Error resetting configuration: ") + std::string(ex.what()));
            message_dialog.run();
            message_dialog.hide();
        }
    }
}
void
Dialog_PluginManager::build_notebook()
{
    building_notebook = true;
    int activePage = notebook.get_current_page();
    while (notebook.get_n_pages() > 0) {
        notebook.remove_page(-1);
    }

    Gtk::ScrolledWindow* installed_plugins_scroll = Gtk::manage(new Gtk::ScrolledWindow());
    installed_plugins_scroll->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    Gtk::VBox* installed_plugins_tab = Gtk::manage(new Gtk::VBox());
    installed_plugins_tab->property_expand().set_value(false);
    
    Gtk::Button* install_plugin_button = manage(new Gtk::Button(_("Install Plugin")));
    install_plugin_button->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
    install_plugin_button->set_always_show_image(true);
    install_plugin_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_PluginManager::on_install_plugin_button_clicked));
    plugin_list_box.set_selection_mode(Gtk::SELECTION_NONE);

    installed_plugins_scroll->add(*installed_plugins_tab);
    
    Gtk::Label* installed_plugins_label = Gtk::manage(new Gtk::Label(_("Installed Plugins")));
    installed_plugins_label->set_margin_bottom(10);
    installed_plugins_label->set_margin_top(10);
    installed_plugins_label->set_margin_left(20);
    installed_plugins_label->set_margin_right(20);
    notebook.append_page(*installed_plugins_scroll, *installed_plugins_label);
    
    installed_plugins_tab->pack_start(plugin_list_box, Gtk::PACK_SHRINK);
    installed_plugins_tab->pack_end(*install_plugin_button, Gtk::PACK_SHRINK);

    auto file_system = FileSystemNative::instance();

    for (const auto& plugin : plugin_list) {

        Gtk::ScrolledWindow* plugin_scroll = Gtk::manage(new Gtk::ScrolledWindow());
        plugin_scroll->set_hexpand(true);
        plugin_scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

        Gtk::VBox* plugin_tab = Gtk::manage(new Gtk::VBox());
        plugin_tab->set_halign(Gtk::ALIGN_FILL);
        plugin_tab->set_hexpand(true);
        
        Gtk::Label* plugin_label = Gtk::manage(new Gtk::Label(plugin.name.get()));
        plugin_label->set_margin_bottom(10);
        plugin_label->set_margin_top(10);
        plugin_label->set_margin_left(20);
        plugin_label->set_margin_right(20);

        std::string config_ui_path = plugin.pluginDir + "/configuration.ui";
        std::string default_config_file = plugin.pluginDir + "/default_config.json";
        std::string user_config_file = plugin.pluginDir + "/user_config.json";

        if (!file_system->is_file(config_ui_path)) {
            Gtk::Label* info_label = Gtk::manage(new Gtk::Label(
                _("This plugin doesn't have any configuration available (configuration file doesn't exist)")
            ));
            info_label->set_margin_top(20);
            info_label->set_margin_bottom(20);
            info_label->set_margin_left(20);
            info_label->set_margin_right(20);
            plugin_tab->pack_start(*info_label, Gtk::PACK_SHRINK);
        } else {
            // Configuration UI exists, check for default config
            if (!file_system->is_file(default_config_file)) {
                // Show error if default config is missing
                Gtk::Label* error_label = Gtk::manage(new Gtk::Label(
                    _("Configuration cannot be loaded - missing default_config.json")
                ));
                error_label->set_margin_top(20);
                error_label->set_margin_bottom(20);
                error_label->set_margin_left(20);
                error_label->set_margin_right(20);
                plugin_tab->pack_start(*error_label, Gtk::PACK_SHRINK);
            } else {
                try {
                    auto builder = Gtk::Builder::create();
                    try {
                        builder->add_from_file(config_ui_path);
                    } catch (const Glib::FileError& ex) {
                        throw std::runtime_error(ex.what());
                    }
                    Gtk::Widget* config_widget = nullptr;
                    builder->get_widget("dialog_contents", config_widget);
                    synfig::FileSystemNative::Handle native_fs = synfig::FileSystemNative::instance();
                    if (!native_fs->is_file(plugin.pluginDir + "/user_config.json")) {
                        auto src_stream = native_fs->get_read_stream(plugin.pluginDir + "/default_config.json");
                        if (!src_stream) {
                            throw std::runtime_error(_("Could not open default configuration file"));
                        }
                        
                        std::stringstream buffer;
                        buffer << src_stream->rdbuf();
                        std::string config_data = buffer.str();

                        auto dst_stream = native_fs->get_write_stream(plugin.pluginDir + "/user_config.json");
                        if (!dst_stream) {
                            throw std::runtime_error(_("Could not create user configuration file"));
                        }
                        
                        if (!config_data.empty()) {
                            dst_stream->write(config_data.c_str(), config_data.size());
                        }
                        dst_stream->flush();
                        dst_stream.reset();
                    }

                    auto stream = native_fs->get_read_stream(user_config_file);
                    if (!stream) {
                        throw std::runtime_error(_("Could not open user configuration file for reading"));
                    }
                    std::stringstream buffer;
                    buffer << stream->rdbuf();
                    std::string config_data = buffer.str();
                    auto values = JSON::Parser::parse(config_data);
                    if (config_widget && GTK_IS_CONTAINER(config_widget->gobj())) {
                        auto container = static_cast<Gtk::Container*>(config_widget);
                        hydrate_config(container, values);
                        synfig::info("Hydrated");

                        plugin_tab->pack_start(*config_widget, Gtk::PACK_SHRINK);

                        Gtk::HBox* button_box = Gtk::manage(new Gtk::HBox());
                        button_box->set_spacing(10);
                        button_box->set_margin_top(10);
                        button_box->set_margin_bottom(10);
                        button_box->set_margin_left(20);
                        button_box->set_margin_right(20);

                        Gtk::Button* save_button = Gtk::manage(new Gtk::Button(_("Save Configuration")));
                        Gtk::Button* reset_button = Gtk::manage(new Gtk::Button(_("Reset to Defaults")));

                        save_button->set_image_from_icon_name("document-save", Gtk::ICON_SIZE_BUTTON);
                        reset_button->set_image_from_icon_name("edit-undo", Gtk::ICON_SIZE_BUTTON);

                        save_button->set_always_show_image(true);
                        reset_button->set_always_show_image(true);

                        save_button->signal_clicked().connect(
                            sigc::bind(sigc::mem_fun(*this, &Dialog_PluginManager::save_plugin_config),
                            plugin.id, config_widget));
                        reset_button->signal_clicked().connect(
                            sigc::bind(sigc::mem_fun(*this, &Dialog_PluginManager::reset_plugin_config),
                            plugin.id, config_widget));

                        button_box->pack_end(*save_button, Gtk::PACK_SHRINK);
                        button_box->pack_end(*reset_button, Gtk::PACK_SHRINK);
                        plugin_tab->pack_end(*button_box, Gtk::PACK_EXPAND_WIDGET);
                    } else {
                        Gtk::Label* error_label = Gtk::manage(new Gtk::Label(
                            _("Error: Configuration UI file found but missing 'dialog_contents' element")));
                        plugin_label->set_margin_bottom(10);
                        plugin_label->set_margin_top(10);
                        plugin_label->set_margin_left(20);
                        plugin_label->set_margin_right(20);
                        plugin_tab->pack_start(*error_label, Gtk::PACK_SHRINK);
                    }
                } 
                catch (const Glib::Error& ex) {
                    Gtk::Label* error_label = Gtk::manage(new Gtk::Label(
                        _("Error loading configuration UI: ") + ex.what()));
                    error_label->set_margin_bottom(20);
                    error_label->set_margin_top(20);
                    error_label->set_margin_left(20);
                    error_label->set_margin_right(20);
                    error_label->set_selectable(true);
                    plugin_tab->pack_start(*error_label, Gtk::PACK_SHRINK);
                }
                catch (const std::exception& ex) {
                    synfig::error("Error loading configuration UI: " + std::string(ex.what()));
                }
            }
        }
        plugin_scroll->add(*plugin_tab);
        notebook.append_page(*plugin_scroll, *plugin_label);
    }
    auto scroll_window = manage(new Gtk::ScrolledWindow());
    scroll_window->add(remote_repo_page);
    notebook.append_page(*scroll_window, *(Gtk::manage(new Gtk::Label("Get Plugins"))));
    notebook.show_all_children();
    notebook.set_current_page(activePage == -1 ? 0 : activePage);
    notebook.set_tab_pos(Gtk::POS_LEFT);
    building_notebook = false;
}
void
Dialog_PluginManager::build_listbox()
{
    for (const auto& plugin : plugin_list) {
        Gtk::HBox* plugin_list_item = manage(new Gtk::HBox());
        Gtk::Label* plugin_name = manage(new Gtk::Label());

        Gtk::HBox* plugin_option_box = manage(new Gtk::HBox());
        Gtk::Button* open_folder = manage(new Gtk::Button());
        Gtk::Button* delete_plugin = manage(new Gtk::Button());

        plugin_list_item->property_expand().set_value(false);
        plugin_list_item->set_margin_top(10);
        plugin_option_box->property_expand().set_value(false);

        open_folder->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
        delete_plugin->set_image_from_icon_name("user-trash-symbolic", Gtk::ICON_SIZE_BUTTON);

        delete_plugin->signal_clicked().connect([plugin, this ](){
            this->confirmation_dialog.set_message(_("Do you want to delete the plugin: ") + plugin.name.get());
            int response = this->confirmation_dialog.run();
            if (response == Gtk::RESPONSE_OK) {              
                App::plugin_manager.remove_plugin(plugin.id);
                this->confirmation_dialog.hide();
                this->message_dialog.set_message("Plugin uninstalled successfully");;
                this->message_dialog.run();
                this->message_dialog.close();
            } else {
                this->confirmation_dialog.close();
            }
        });

        open_folder->signal_clicked().connect([plugin](){
            plugin.launch_dir();
        });

        plugin_option_box->pack_start(*open_folder, Gtk::PACK_SHRINK, 10);
        plugin_option_box->pack_start(*delete_plugin, Gtk::PACK_SHRINK, 10);
        plugin_option_box->set_margin_top(10);
        plugin_option_box->set_margin_bottom(10);

        plugin_name->set_text(plugin.name.get());

        plugin_list_item->pack_start(*plugin_name, Gtk::PACK_SHRINK, 20);
        plugin_list_item->pack_end(*plugin_option_box, Gtk::PACK_SHRINK, 20);
        
        plugin_list_box.add(*plugin_list_item);

    }
    plugin_list_box.show_all_children();
}

std::string extract_plugin_name(std::istream &fileStream)
{
    xmlpp::DomParser parser;
    parser.set_substitute_entities();
    parser.parse_stream(fileStream);

    const xmlpp::Node* pNode = parser.get_document()->get_root_node();
    if (std::string(pNode->get_name()) != std::string("plugin") ) {
        synfig::warning("Invalid plugin.xml file (missing root <plugin>)");
        return std::string("");
    }

    auto execlist = pNode->find("./exec");
    if (!execlist.empty())
    {
        for (const xmlpp::Node* node : pNode->find("./name") ) {
            const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
            std::string lang = element->get_attribute_value("lang");
            if (lang.empty()) {
                return element->get_child_text()->get_content();
            }
        }
    }

    return "";
}
void Dialog_PluginManager::on_install_plugin_button_clicked() 
{
    int result = plugin_file_dialog.run();
    if (result == Gtk::RESPONSE_OK) {
        std::string zip_filename = plugin_file_dialog.get_filename();
        install_plugin_from_zip(zip_filename);
    }
    plugin_file_dialog.close();
}
bool Dialog_PluginManager::install_plugin_from_zip(const std::string& zip_filename)
{
    std::vector<std::string> files;
    synfig::FileSystemNative::Handle native_fs = FileSystemNative::instance();
    synfig::FileContainerZip::Handle zip_fs = new FileContainerZip();
    filesystem::Path path_to_user_plugins = synfigapp::Main::get_user_app_directory() / filesystem::Path("plugins");
    std::string output_path;
    std::string plugin_metadata_file;
    std::string plugin_name;

    if (!zip_fs->open(zip_filename)) {
        synfig::error("Failed to open zip file: " + zip_filename);
        return false;
    }

    zip_fs->directory_scan("", files);
    for (const auto& file : files) {
        if (file == "plugin.xml") {
            plugin_metadata_file = file;
            break;
        } else if(zip_fs->is_directory(file)) {
            std::vector<std::string> child_files;
            if (zip_fs->directory_scan(file, child_files)) {
                auto it = std::find(child_files.begin(), child_files.end(), "plugin.xml");
                if (it != child_files.end()) {
                    plugin_metadata_file = file + "/plugin.xml";
                }
                if (!plugin_metadata_file.empty())
                    break;
            }
        }
    }

    if (plugin_metadata_file.empty() || !zip_fs->is_file(plugin_metadata_file)) {
        synfig::error("Failed to find plugin.xml in zip file: " + zip_filename);
        message_dialog.set_message("Failed to find plugin.xml in zip file: " + zip_filename);
        message_dialog.run();
        message_dialog.close();
        return false;
    }

    plugin_name = extract_plugin_name(*zip_fs->get_read_stream(plugin_metadata_file));
    output_path = ((path_to_user_plugins / plugin_name).add_suffix("/")).u8string();

    if (native_fs->is_exists(output_path) && native_fs->is_directory(output_path)) {
        confirmation_dialog.set_message(_("Plugin already exists. Do you want to overwrite it?"));
        int response = confirmation_dialog.run();
        confirmation_dialog.close();
        if (response != Gtk::RESPONSE_OK) {
            return false;
        }
        auto it = std::find_if(App::plugin_manager.plugins().begin(), 
                              App::plugin_manager.plugins().end(), 
                              [plugin_name](const Plugin& plugin) { 
                                  return plugin.name.fallback() == plugin_name; 
                              });
        if (it != App::plugin_manager.plugins().end()) {
            App::plugin_manager.remove_plugin(it->id);
        }
        native_fs->remove_recursive(output_path);
    }

    if (native_fs->is_file(output_path)) {
        if (!native_fs->file_remove(output_path)) {
            synfig::error("Failed to remove file: " + output_path);
            return false;
        }
    }

    if (native_fs->directory_create(output_path)) {
        if (plugin_metadata_file.find("/") == std::string::npos) {
            for(const auto& file : files) {
                zip_fs->copy_recursive(zip_fs, file, native_fs, output_path + file);
            }
        } else {
            zip_fs->copy_recursive(zip_fs, files[0], native_fs, output_path);
        }
    }

    App::plugin_manager.load_plugin(output_path + "plugin.xml", output_path, true);
    zip_fs->close();

    message_dialog.set_message(_("Plugin installed successfully."));
    message_dialog.run();
    message_dialog.close();
    
    return true;
}
void
Dialog_PluginManager::on_notebook_switch_page(Gtk::Widget* page, int page_num) {
    if (page_num == notebook.get_n_pages() - 1 && !building_notebook) {
        remote_repo_page.load_content();
    }
}

RemoteRepoPage::RemoteRepoPage() : content_loaded(false) {
    m_dispatcher.connect(sigc::mem_fun(*this, &RemoteRepoPage::on_network_complete));

    Gtk::Spinner *loading_spinner = manage(new Gtk::Spinner());

    loader_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    loader_box.set_hexpand(true);
    loading_spinner->set_size_request(100, 100);
    loading_spinner->start();
    loader_box.pack_start(*loading_spinner, Gtk::PACK_EXPAND_WIDGET);
    loading_spinner->set_halign(Gtk::ALIGN_CENTER);
    loading_spinner->set_hexpand(true);
    loading_spinner->set_valign(Gtk::ALIGN_CENTER);

    m_flowbox.set_hexpand(true);
    m_flowbox.set_vexpand(true);
    m_flowbox.set_max_children_per_line(3);
    m_flowbox.set_row_spacing(10);
    m_flowbox.set_column_spacing(10);
    m_flowbox.set_selection_mode(Gtk::SELECTION_NONE);

    add(loader_box);
}
RemoteRepoPage::~RemoteRepoPage() {
    if(m_thread && m_thread->joinable()) {
        m_thread->join();
    }
    plugin_cards.clear();
}


void RemoteRepoPage::load_content() {
    if (!content_loaded) {
        synfig::info("Get Plugin tab activated - Starting network request");
        start_network_request();
    } else {
        synfig::info("Content already loaded - Skipping network request");
    }
}

void RemoteRepoPage::start_network_request() {
    if (content_loaded || (m_thread && m_thread->joinable())) {
        return; 
    }
    
    response_data = "";
    m_thread.reset(new std::thread([this]() {
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://9149efbd.plugins-6ja.pages.dev/");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_string);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                synfig::warning("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
                m_request_success = false;
            } else {
                m_request_success = true;
            }

            m_dispatcher.emit();
        }
    }));
}
void RemoteRepoPage::on_network_complete() {
    if (content_loaded) {
        return;  
    }
    
    this->remove(loader_box);
    if (m_request_success) {
        build_cards();
        m_flowbox.set_size_request(1110, 400);
        this->add(m_flowbox);
        show_all_children();
        content_loaded = true;
    } else {
        Gtk::Label *error_label = manage(new Gtk::Label("Failed to get plugin list"));
        error_label->set_hexpand(true);
        error_label->set_halign(Gtk::ALIGN_CENTER);
        error_label->set_valign(Gtk::ALIGN_CENTER);
        this->pack_start(*error_label, Gtk::PACK_EXPAND_WIDGET);
        show_all();
    }
}
void 
RemoteRepoPage::build_card_for_plugin(PluginInfo &plugin)
{
    auto *card = Gtk::manage(new PluginCard(plugin));
    try
    {
        plugin_cards.push_back(card);
        m_flowbox.add(*card);
    }
    catch (const Glib::FileError &ex)
    {
        std::cerr << "FileError: " << ex.what() << std::endl;
        return;
    }
    catch (const Glib::MarkupError &ex)
    {
        std::cerr << "MarkupError: " << ex.what() << std::endl;
        return;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return;
    }
    std::cout << "Card built for plugin: " << plugin.name << std::endl;
}
void
RemoteRepoPage::build_cards()
{
    xmlpp::DomParser parser;
    parser.parse_memory(response_data);

    xmlpp::Node *root = parser.get_document()->get_root_node();
    std::vector<PluginInfo> plugins = get_plugins(*root);
    for (auto &plugin : plugins)
    {
        build_card_for_plugin(plugin);
    }
}


void RemoteRepoPage::update_cards_status()
{
    for (auto *card : plugin_cards) {
        card->update_install_status();
    }
}


PluginCard::PluginCard(PluginInfo plugin)
        : Gtk::Box(Gtk::Orientation::ORIENTATION_VERTICAL),
          m_dispatcher(),
          plugin(plugin)
    {
        m_dispatcher.connect(sigc::mem_fun(*this, &PluginCard::on_progress_update));

        auto builder = ResourceHelper::load_interface("plugin_manager_card.glade");

        builder->get_widget("card_box", card_box);
        builder->get_widget("plugin_name_label", plugin_name_label);
        builder->get_widget("plugin_info_label", plugin_info_label);
        builder->get_widget("plugin_description_label", plugin_description_label);
        builder->get_widget("download_container", download_container);
        builder->get_widget("get_plugin_button", get_plugin_button);
        builder->get_widget("visit_site_button", visit_site_button);

        if (card_box)
        {
            this->add(*card_box);
        }

        if (plugin_name_label)
        {
            plugin_name_label->set_text(plugin.name);
        }
        if (plugin_info_label)
        {
            plugin_info_label->set_text("By: " + plugin.author + " | Release: " + plugin.release + " | Version: " + plugin.version);
        }
        if (plugin_description_label)
        {
            plugin_description_label->set_text(plugin.description);
        }
        bool is_installed = false;
        for (const auto& installed_plugin : App::plugin_manager.plugins()) {
            if (installed_plugin.id == plugin.id) {
                is_installed = true;
                break;
            }
        }
        get_plugin_button->signal_clicked().connect(
            sigc::mem_fun(*this, &PluginCard::on_get_plugin_button_clicked));
        if (get_plugin_button)
        {
            if (is_installed) {
                get_plugin_button->set_label("Installed");
                get_plugin_button->set_sensitive(false);
            }
        }

        if (visit_site_button)
        {
            visit_site_button->signal_clicked().connect(
                sigc::mem_fun(*this, &PluginCard::on_visit_site_button_clicked));
        }

        show_all_children();
    }

PluginCard::~PluginCard()
{
    if (download_in_progress)
    {
        download_in_progress = false;
    }
    
    if (download_thread && download_thread->joinable())
    {
        download_thread->join();
    }
}
void
PluginCard::on_progress_update()
{
    if (download_in_progress)
    {
        progress_bar.set_fraction(current_progress.load());
    }
}

void
PluginCard::on_get_plugin_button_clicked()
{
    if (!download_container || !get_plugin_button || download_in_progress)
        return;

    download_in_progress = true;
    download_container->remove(*get_plugin_button);
    progress_bar.set_show_text(true);
    progress_bar.set_fraction(0.0);
    download_container->add(progress_bar);
    if (download_thread && download_thread->joinable()) {
        download_thread->join();
    }
    download_thread.reset(new std::thread([this]() {
        get_network_file(this->plugin.download);
    }));
    download_container->show_all();
}


void
PluginCard::on_visit_site_button_clicked()
{
    open_url(plugin.url);
}

int
PluginCard::progress_callback(void *clientp,
                            curl_off_t dltotal,
                            curl_off_t dlnow,
                            curl_off_t ultotal,
                            curl_off_t ulnow)
{
    auto self = static_cast<PluginCard *>(clientp);
    if (dltotal > 0 && self->download_in_progress)
    {
        self->current_progress.store((double)dlnow / (double)dltotal);
        self->m_dispatcher.emit();
    }
    return 0;
}

void
PluginCard::get_network_file(std::string url)
{
    CURL *curl = curl_easy_init();
    if (!curl) return;

    std::string filename = url.substr(url.find_last_of("/\\") + 1);
    if (filename.empty()) {
        filename = "plugin.zip";
    }
    
    filesystem::Path temp_dir = FileSystemTemporary::get_system_temporary_directory();
    auto pair = FileSystemTemporary::reserve_temporary_filename(temp_dir, "synfig_plugin_", ".zip");
    auto temp_path = pair.first;
    auto lock_file = pair.second;
    synfig::info("Downloading plugin to temporary path: %s", temp_path.u8string().c_str());
    
    if (!lock_file) {
        curl_easy_cleanup(curl);
        Glib::signal_idle().connect_once([this]() {
            if (download_in_progress) {
                auto message = Gtk::manage(new Gtk::Label(_("Failed to create temporary file")));
                download_container->remove(progress_bar);
                download_container->add(*message);
                download_container->show_all();
            }
        });
        download_in_progress = false;
        return;
    }

    lock_file.reset();

    struct FileData file_data = {0};
    file_data.fp = fopen(temp_path.u8string().c_str(), "wb");
    if (!file_data.fp)
    {
        curl_easy_cleanup(curl);
        Glib::signal_idle().connect_once([this]() {
            if (download_in_progress)
            {
                auto message = Gtk::manage(new Gtk::Label(_("Failed to open temporary file for writing")));
                download_container->remove(progress_bar);
                download_container->add(*message);
                download_container->show_all();
            }
        });
        download_in_progress = false;
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file_data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(file_data.fp);

    Glib::signal_idle().connect_once([this, res, temp_path]() {
        if (!download_in_progress) {
            synfig::info("Download cancelled, removing temporary file: %s", temp_path.u8string().c_str());
            FileSystemNative::instance()->file_remove(temp_path.u8string());
            return;
        }
        
        if (res != CURLE_OK)
        {
            synfig::info("Download failed, removing temporary file: %s", temp_path.u8string().c_str());
            auto message = Gtk::manage(new Gtk::Label(_("Download failed: ") + 
                std::string(curl_easy_strerror(res))));
            download_container->remove(progress_bar);
            download_container->add(*message);
            FileSystemNative::instance()->file_remove(temp_path.u8string());
        }
        else
        {
            synfig::info("Download completed successfully to: %s", temp_path.u8string().c_str());
            
            Dialog_PluginManager* dialog = dynamic_cast<Dialog_PluginManager*>(get_toplevel());
            if (dialog) {
                bool install_success = dialog->install_plugin_from_zip(temp_path.u8string());
                if (install_success) {
                    get_plugin_button->set_sensitive(false);
                    get_plugin_button->set_label("Installed!");
                } else {
                    get_plugin_button->set_sensitive(true);
                    get_plugin_button->set_label("Installation Failed");
                }
            } else {
                get_plugin_button->set_sensitive(true);
                get_plugin_button->set_label("Install Failed - Try Manual Install");
                synfig::error("Could not get Dialog_PluginManager instance");
            }
            
            FileSystemNative::instance()->file_remove(temp_path.u8string());
            
            download_container->remove(progress_bar);
            download_container->add(*get_plugin_button);
        }
        download_container->show_all();
        download_in_progress = false;
    });
}

void PluginCard::update_install_status()
{
    if (!get_plugin_button)
        return;

    bool is_installed = false;
    for (const auto& installed_plugin : App::plugin_manager.plugins()) {
        if (installed_plugin.id == plugin.id) {
            is_installed = true;
            break;
        }
    }

    // Only update if we're not in the middle of a download
    if (!download_in_progress) {
        if (is_installed) {
            get_plugin_button->set_label("Installed");
            get_plugin_button->set_sensitive(false);
        } else {
            get_plugin_button->set_label("Get Plugin");
            get_plugin_button->set_sensitive(true);
        }
        
        // Make sure button is visible (not progress bar)
        if (download_container) {
            auto children = download_container->get_children();
            if (!children.empty() && children[0] != get_plugin_button) {
                download_container->remove(*children[0]);
                download_container->add(*get_plugin_button);
                download_container->show_all();
            }
        }
    }
}
