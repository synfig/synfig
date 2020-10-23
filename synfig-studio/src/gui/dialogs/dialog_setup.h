/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_setup.h
**	\brief Dialog Preference Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2008, 2009. 2012 Carlos López
**	Copyright (c) 2015 Jerome Blanchi
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

#ifndef __SYNFIG_STUDIO_DIALOG_SETUP_H
#define __SYNFIG_STUDIO_DIALOG_SETUP_H

/* === H E A D E R S ======================================================= */
#include <gtkmm/adjustment.h>
#include <gtkmm/button.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/switch.h>

#include <gui/dialogs/dialog_template.h>
#include <gui/widgets/widget_time.h>

#include <synfig/time.h>

#include <synfigapp/settings.h>

/* === M A C R O S ========================================================= */
#ifndef DEFAULT_PREDEFINED_SIZE
#define DEFAULT_PREDEFINED_SIZE _("Custom Size")
#endif
#ifndef DEFAULT_PREDEFINED_FPS
#define DEFAULT_PREDEFINED_FPS _("Custom fps")
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Enum;

class Dialog_Setup : public Dialog_Template
{
	/* Draft for change resume */
	enum Change
	{
		CHANGE_NONE              = (0),       //    0
		CHANGE_UI_LANGUAGE       = (1 <<  0), //    1
		CHANGE_AUTOBACKUP        = (1 <<  1), //    2
		CHANGE_UI_HANDLE_TOOLTIP = (1 <<  2), //    4
		CHANGE_WIDTH             = (1 <<  3), //    8
		CHANGE_ANGLE             = (1 <<  4), //   16
		CHANGE_VERTEX            = (1 <<  5), //   32
		CHANGE_BONE_RECURSIVE    = (1 <<  6), //   64
		CHANGE_BRUSH_PATH        = (1 <<  7), //  128
		CHANGE_SCALE             = (1 <<  8), //  256
		CHANGE_SCALE_X           = (1 <<  9), //  512
		CHANGE_SCALE_Y           = (1 << 10), // 1024
		CHANGE_SKEW              = (1 << 11), // 2048

		CHANGE_ALL               = (~0)
	};

	// Change mechanism
	// TODO on change class
	void on_value_change(int valueflag);
	//Signal handlers pages
	void on_size_template_combo_change();
	void on_fps_template_combo_change();
	void on_ui_language_combo_change();
	void on_theme_changed();
	void on_time_format_changed();
	void on_autobackup_changed();
	void on_tooltip_transformation_changed();
	void on_play_sound_on_render_done_changed();
	void on_def_background_type_changed(); //bound on clicked
	void on_def_background_color_changed();
	void on_def_background_image_set();
	void on_number_of_thread_changed();
	void on_preview_background_color_changed();
	void on_brush_path_add_clicked();
	void on_brush_path_remove_clicked();
	void on_choose_editor_pressed();
	bool select_path_dialog(const std::string &title, std::string &filename);

	void create_system_page(PageInfo pi);
	void create_document_page(PageInfo pi);
	void create_render_page(PageInfo pi);
	void create_interface_page(PageInfo pi);
	void create_editing_page(PageInfo pi);
	void create_shortcuts_page(PageInfo pi);

	synfigapp::Settings &input_settings;

	// Widget for pages
	Gtk::ComboBoxText timestamp_comboboxtext;
	std::map<std::string, synfig::Time::Format> time_formats;

	Glib::RefPtr<Gtk::Adjustment> adj_recent_files;
	Glib::RefPtr<Gtk::Adjustment> adj_undo_depth;

	synfig::Time::Format time_format;

	Widget_Enum *widget_enum;

	Widget_Time auto_backup_interval;

	Gtk::Switch toggle_restrict_radius_ducks;
	Gtk::Switch toggle_animation_thumbnail_preview;
	Gtk::Switch toggle_enable_experimental_features;
	Gtk::Switch toggle_use_dark_theme;
	Gtk::Switch toggle_show_file_toolbar;

	Gtk::Entry textbox_brushe_path;
	Gtk::ListViewText* listviewtext_brushes_path;
	Glib::RefPtr<Gtk::ListStore> brushpath_refmodel;

	Gtk::ComboBoxText* size_template_combo;
	Gtk::ComboBoxText* fps_template_combo;
	Gtk::ComboBoxText ui_language_combo;
	Gtk::Switch toggle_handle_tooltip_transfo_value;
	Gtk::Switch toggle_handle_tooltip_transfo_name;

	Gtk::Entry textbox_custom_filename_prefix;
	Glib::RefPtr<Gtk::Adjustment> adj_pref_x_size;
	Glib::RefPtr<Gtk::Adjustment> adj_pref_y_size;
	Glib::RefPtr<Gtk::Adjustment> adj_pref_fps;
	Gtk::SpinButton* pref_fps_spinbutton;
	Gtk::SpinButton* pref_y_size_spinbutton;
	Gtk::SpinButton* pref_x_size_spinbutton;

	Gtk::RadioButton::Group group_def_background;
	Gtk::RadioButton        def_background_none;
	Gtk::RadioButton        def_background_color;
	Gtk::RadioButton        def_background_image;
	Gtk::ColorButton        def_background_color_button;
	Gtk::FileChooserButton  fcbutton_image;
	Gtk::ColorButton        preview_background_color_button;
	//Gtk::FileFilter         filter_images;
	//Gtk::FileFilter         filter_any;

	Gtk::Entry        image_sequence_separator;
	Gtk::ComboBoxText workarea_renderer_combo;
	Gtk::Switch       toggle_play_sound_on_render_done;
	Glib::RefPtr<Gtk::Adjustment> adj_number_of_threads;
	Gtk::SpinButton*  number_of_threads_select;	

	Gtk::Switch toggle_handle_tooltip_widthpoint;
	Gtk::Switch toggle_handle_tooltip_radius;
	Gtk::Switch toggle_handle_tooltip_transformation;
	Gtk::Switch toggle_autobackup;
	Gtk::Entry image_editor_path_entry;
	long pref_modification_flag;
	//! Do not update state flag on refreshing
	bool refreshing;

	//Brush path Tree model columns:
	class PrefsBrushPath : public Gtk::TreeModel::ColumnRecord
	{
		public:
		PrefsBrushPath() { add(path); }
		Gtk::TreeModelColumn<synfig::String> path;
	};
	PrefsBrushPath prefs_brushpath;

	Gtk::CellRendererAccel renderer_accel;
	Gtk::TreeView *treeview_accels;

	void on_accel_edited(const Glib::ustring& path_string, guint accel_key, Gdk::ModifierType accel_mods, guint hardware_keycode);
	void on_accel_cleared(const Glib::ustring& path_string);
	void on_restore_default_accels_pressed();
public:
	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

	//Signal handlers dialog
	void on_resize_imported_changed();
	virtual void on_apply_pressed();
	virtual void on_restore_pressed();

public:

	Dialog_Setup(Gtk::Window& parent);
	~Dialog_Setup();

	void set_time_format(synfig::Time::Format time_format);
	const synfig::Time::Format& get_time_format()const { return time_format; }

    void refresh();
}; // END of Dialog_Setup

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
