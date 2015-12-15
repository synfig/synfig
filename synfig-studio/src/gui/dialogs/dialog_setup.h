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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_DIALOG_SETUP_H
#define __SYNFIG_STUDIO_DIALOG_SETUP_H

/* === H E A D E R S ======================================================= */

#include <gtk/gtk.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/switch.h>
#include <gtkmm/notebook.h>
#include <gtkmm/grid.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>

#include <gui/widgets/widget_time.h>

#include <synfig/gamma.h>
#include <synfig/time.h>
#include <algorithm>

#include "app.h"

/* === M A C R O S ========================================================= */
#ifndef DEFAULT_PREDEFINED_SIZE
#define DEFAULT_PREDEFINED_SIZE _("Custom Size")
#endif
#ifndef DEFAULT_PREDEFINED_FPS
#define DEFAULT_PREDEFINED_FPS _("Custom fps")
#endif

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; };

namespace studio {

class GammaPattern : public Gtk::DrawingArea
{
	float gamma_r;
	float gamma_g;
	float gamma_b;
	float black_level;
	float red_blue_level;

	int tile_w, tile_h;

	Gdk::Color black[4],white[4],gray50[4],gray25[4];

	float r_F32_to_F32(float x)const { float f((pow(x,gamma_r)*std::min(red_blue_level,1.0f)*(1.0f-black_level)+black_level)); if(f<0)f=0; if(f>1)f=1; return f; }
	float g_F32_to_F32(float x)const { float f((pow(x,gamma_g)*sqrt(std::min(2.0f-red_blue_level,red_blue_level))*(1.0f-black_level)+black_level)); if(f<0)f=0; if(f>1)f=1; return f; }
	float b_F32_to_F32(float x)const { float f((pow(x,gamma_b)*std::min(2.0f-red_blue_level,1.0f)*(1.0f-black_level)+black_level)); if(f<0)f=0; if(f>1)f=1; return f; }

public:

	void refresh();

	void set_gamma_r(float x) { gamma_r=x; }
	void set_gamma_g(float x) { gamma_g=x; };
	void set_gamma_b(float x) { gamma_b=x; };
	void set_black_level(float x) { black_level=x; };
	void set_red_blue_level(float x) { red_blue_level=x; };

	float get_gamma_r()const { return gamma_r; }
	float get_gamma_g()const { return gamma_g; }
	float get_gamma_b()const { return gamma_b; }
	float get_black_level()const { return black_level; }
	float get_red_blue_level()const { return red_blue_level; }

	GammaPattern();

	~GammaPattern();

	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
}; // END of class GammaPattern

class BlackLevelSelector : public Gtk::DrawingArea
{
	float level;

	sigc::signal<void> signal_value_changed_;

public:

	BlackLevelSelector();

	~BlackLevelSelector();

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }

	void set_value(float x) { level=x; queue_draw(); }

	const float &get_value()const { return level; }

	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	bool on_event(GdkEvent *event);
}; // END of class BlackLevelSelector

class RedBlueLevelSelector : public Gtk::DrawingArea
{
	float level;

	sigc::signal<void> signal_value_changed_;

public:

	RedBlueLevelSelector();

	~RedBlueLevelSelector();

	sigc::signal<void>& signal_value_changed() { return signal_value_changed_; }

	void set_value(float x) { level=x; queue_draw(); }

	const float &get_value()const { return level; }

	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	bool on_event(GdkEvent *event);
}; // END of class RedBlueSelector

class Widget_Enum;

class Dialog_Setup : public Gtk::Dialog
{

	void on_ok_pressed();
	void on_apply_pressed();
	void on_restore_pressed();

	void on_gamma_r_change();
	void on_gamma_g_change();
	void on_gamma_b_change();
	void on_black_level_change();
	void on_red_blue_level_change();
	void on_size_template_combo_change();
	void on_fps_template_combo_change();
	void on_ui_language_combo_change();
	void on_time_format_changed();

	void create_gamma_page(synfig::String name);
	void create_system_page(synfig::String name);
	void create_document_page(synfig::String name);
	void create_render_page(synfig::String name);
	void create_interface_page(synfig::String name);
	void create_editing_page(synfig::String name);

	//! \Brief Set the main title of the page
	void attach_label_title(Gtk::Grid *grid, synfig::String str);
	//! \Brief Add a new section (col 0) at specified row
	void attach_label_section(Gtk::Grid *grid, synfig::String str, guint row);
	//! \Brief Add a single label (col 0) at specified row
	void attach_label(Gtk::Grid *grid, synfig::String str, guint row);
	//! \Brief Add a single label at specified row and col
	void attach_label(Gtk::Grid *grid, synfig::String str, guint row, guint col);

	Gtk::Notebook *notebook;
	// Style for title(s)
	Pango::AttrList title_attrlist;
	Pango::AttrList section_attrlist;

	GammaPattern gamma_pattern;
	BlackLevelSelector black_level_selector;
	RedBlueLevelSelector red_blue_level_selector;
	Gtk::ComboBoxText timestamp_comboboxtext;
	std::map<std::string, synfig::Time::Format> time_formats;

	Glib::RefPtr<Gtk::Adjustment> adj_gamma_r;
	Glib::RefPtr<Gtk::Adjustment> adj_gamma_g;
	Glib::RefPtr<Gtk::Adjustment> adj_gamma_b;

	Glib::RefPtr<Gtk::Adjustment> adj_recent_files;
	Glib::RefPtr<Gtk::Adjustment> adj_undo_depth;

	Gtk::CheckButton toggle_use_colorspace_gamma;
#ifdef SINGLE_THREADED
	Gtk::CheckButton toggle_single_threaded;
#endif

	synfig::Time::Format time_format;

	Gtk::Menu *timestamp_menu;
	Widget_Enum *widget_enum;

	Widget_Time auto_backup_interval;

	Gtk::CheckButton toggle_restrict_radius_ducks;
	Gtk::CheckButton toggle_resize_imported_images;
	Gtk::CheckButton toggle_enable_experimental_features;
	Gtk::CheckButton toggle_use_dark_theme;

	Gtk::Entry textbox_browser_command;
	Gtk::Entry textbox_brushes_path;

	Gtk::ComboBoxText* size_template_combo;
	Gtk::ComboBoxText* fps_template_combo;
	Gtk::ComboBoxText ui_language_combo;
	std::vector<Glib::ustring> _lang_codes;


	Gtk::Entry textbox_custom_filename_prefix;
	Glib::RefPtr<Gtk::Adjustment> adj_pref_x_size;
	Glib::RefPtr<Gtk::Adjustment> adj_pref_y_size;
	Glib::RefPtr<Gtk::Adjustment> adj_pref_fps;
	Gtk::SpinButton* pref_fps_spinbutton;
	Gtk::SpinButton* pref_y_size_spinbutton;
	Gtk::SpinButton* pref_x_size_spinbutton;

	Gtk::Entry image_sequence_separator;
	Gtk::ComboBoxText navigator_renderer_combo;
	Gtk::ComboBoxText workarea_renderer_combo;

	Gtk::Switch toggle_handle_tooltip_widthpoint;
	Gtk::Switch toggle_handle_tooltip_radius;
	Gtk::Switch toggle_handle_tooltip_transformation;
	Gtk::Switch toggle_autobackup;

protected:
	//Signal handlers:
	void on_treeview_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);



	//Tree model columns:
	class PrefsCategories : public Gtk::TreeModel::ColumnRecord
	{
		public:

		PrefsCategories() { add(category_id); add(category_name); }

		Gtk::TreeModelColumn<int> category_id;
		Gtk::TreeModelColumn<Glib::ustring> category_name;
	};

	PrefsCategories prefs_categories;

	//Child widgets:
	Gtk::Grid main_grid;

	Gtk::ScrolledWindow prefs_categories_scrolledwindow;
	Gtk::TreeView prefs_categories_treeview;
	Glib::RefPtr<Gtk::TreeStore> prefs_categories_reftreemodel;

public:

	void set_time_format(synfig::Time::Format time_format);

	const synfig::Time::Format& get_time_format()const { return time_format; }

	Dialog_Setup(Gtk::Window& parent);
	~Dialog_Setup();

    void refresh();

}; // END of Dialog_Waypoint

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
