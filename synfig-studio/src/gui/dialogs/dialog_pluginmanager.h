/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_pluginmanager.h
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

/* === S T A R T =========================================================== */

#ifndef SYNFIG_GTKMM_DIALOG_PLUGINMANAGER_H
#define SYNFIG_GTKMM_DIALOG_PLUGINMANAGER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <thread>
#include <curl/curl.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/flowbox.h>
#include <gtkmm/notebook.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/listbox.h>
#include <gui/pluginmanager.h>
/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{
struct PluginInfo
{
    std::string name;
    std::string id;
    std::string download;
    std::string author;
    std::string release;
    std::string version;
    std::string url;
    std::string description;
};
struct FileData
{
    FILE *fp;
    size_t size;
};


class PluginCard : public Gtk::Box
{
public:
    PluginCard(PluginInfo plugin);
    ~PluginCard();
    void update_install_status();
    const std::string& get_plugin_id() const { return plugin.id; }
private:
    PluginInfo plugin;
    Gtk::Box *card_box{nullptr};
    Gtk::Label *plugin_name_label{nullptr};
    Gtk::Label *plugin_info_label{nullptr};
    Gtk::Label *plugin_description_label{nullptr};
    Gtk::Box *download_container{nullptr};
    Gtk::Button *get_plugin_button{nullptr};
    Gtk::Button *visit_site_button{nullptr};
    Gtk::ProgressBar progress_bar;
    Glib::Dispatcher m_dispatcher;  // Changed to direct member instead of pointer
    std::atomic<double> current_progress{0.0};
    std::unique_ptr<std::thread> download_thread;
    std::atomic<bool> download_in_progress{false};

    void on_progress_update();
    void on_get_plugin_button_clicked();
    void on_visit_site_button_clicked();
    static int progress_callback(void *clientp,
                               curl_off_t dltotal,
                               curl_off_t dlnow,
                               curl_off_t ultotal,
                               curl_off_t ulnow);
    void get_network_file(std::string url);
};

class RemoteRepoPage : public Gtk::Box 
{
public:

    RemoteRepoPage();
    void load_content();
    void start_network_request();
    void on_network_complete();
    void build_card_for_plugin(PluginInfo &plugin);
    void build_cards();
    void update_cards_status();
    ~RemoteRepoPage();
private:
    bool content_loaded{false};
    Gtk::Box loader_box;
    Gtk::FlowBox m_flowbox;
    Glib::Dispatcher m_dispatcher;
    std::unique_ptr<std::thread> m_thread;
    bool m_request_success{false};
    std::vector<PluginCard*> plugin_cards;
    std::string response_data;
};


class Dialog_PluginManager : public Gtk::Dialog 
{
public:
    Dialog_PluginManager(Gtk::Window& parent);
    bool install_plugin_from_zip(const std::string& zip_filename);
    ~Dialog_PluginManager();

private:
    Gtk::ListBox plugin_list_box;
    Gtk::FileChooserDialog plugin_file_dialog;
    Gtk::MessageDialog message_dialog;
    Gtk::MessageDialog confirmation_dialog;
    Gtk::Notebook notebook;
    RemoteRepoPage remote_repo_page;
    std::vector<studio::Plugin> plugin_list;
    bool building_notebook{false};
    void build_listbox();
    void build_notebook();
    void save_plugin_config(const std::string &plugin_id, Gtk::Widget *config_widget);
    void reset_plugin_config(const std::string &plugin_id, Gtk::Widget *config_widget);
    void on_install_plugin_button_clicked();
    void on_notebook_switch_page(Gtk::Widget* page, int page_num);
    void refresh();

}; // END of class Dialog_PluginManager

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
