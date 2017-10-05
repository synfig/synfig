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
#include <dialogs/dialog_template.h>

#include <gtkmm/adjustment.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/switch.h>
#include <gtkmm/tooltip.h>

#include <gui/widgets/widget_time.h>

#include <synfig/gamma.h>
#include <synfig/time.h>
#include <algorithm>

#include <synfigapp/settings.h>

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

class Dialog_Setup : public Dialog_Template
{
	/* Draft for change resume */
	enum Change
	{
		CHANGE_NONE					=	(0),		//    0
		CHANGE_UI_LANGUAGE			=	(1 <<  0),	//    1
		CHANGE_AUTOBACKUP			=	(1 <<  1),	//    2
		CHANGE_UI_HANDLE_TOOLTIP					=	(1 <<  2),	//    4
		CHANGE_WIDTH					=	(1 <<  3),	//    8
		CHANGE_ANGLE					=	(1 <<  4),		CHANGE_VERTEX					=	(1 <<  5),		CHANGE_BONE_RECURSIVE			=	(1 <<  6),		CHANGE_BRUSH_PATH				=	(1 <<  7),		CHANGE_SCALE					=	(1 <<  8),		CHANGE_SCALE_X				=	(1 <<  9),		CHANGE_SCALE_Y				=	(1 << 10),		CHANGE_SKEW					=	(1 << 11),
		CHANGE_ALL					=	(~0)
	};

	// Change mechanism
	// TODO on change class
	void on_value_change(int valueflag);
	//Signal handlers pages
	void on_gamma_r_change();
	void on_gamma_g_change();
	void on_gamma_b_change();
	void on_black_level_change();
	void on_red_blue_level_change();
	void on_size_template_combo_change();
	void on_fps_template_combo_change();
	void on_ui_language_combo_change();
	void on_time_format_changed();
	void on_autobackup_changed();
	void on_tooltip_transformation_changed();

	void on_brush_path_add_clicked();
	void on_brush_path_remove_clicked();

	void create_gamma_page(PageInfo pi);
	void create_system_page(PageInfo pi);
	void create_document_page(PageInfo pi);
	void create_render_page(PageInfo pi);
	void create_interface_page(PageInfo pi);
	void create_editing_page(PageInfo pi);

	synfigapp::Settings &input_settings;

	// Widget for pages
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

	Gtk::Switch toggle_use_colorspace_gamma;
#ifdef SINGLE_THREADED
	Gtk::Switch toggle_single_threaded;
#endif

	synfig::Time::Format time_format;

	Gtk::Menu *timestamp_menu;
	Widget_Enum *widget_enum;

	Widget_Time auto_backup_interval;

	Gtk::Switch toggle_restrict_radius_ducks;
	Gtk::Switch toggle_resize_imported_images;
	Gtk::CheckButton toggle_enable_experimental_features;
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

	Gtk::Entry image_sequence_separator;
	Gtk::ComboBoxText navigator_renderer_combo;
	Gtk::ComboBoxText workarea_renderer_combo;

	Gtk::Switch toggle_handle_tooltip_widthpoint;
	Gtk::Switch toggle_handle_tooltip_radius;
	Gtk::Switch toggle_handle_tooltip_transformation;
	Gtk::Switch toggle_autobackup;

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

public:
	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/

	//Signal handlers dialog
	virtual void on_apply_pressed();
	virtual void on_restore_pressed();

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
