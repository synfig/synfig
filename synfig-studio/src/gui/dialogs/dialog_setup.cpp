/* === S Y N F I G ========================================================= */
/*!	\file dialog_setup.cpp
**	\brief Dialog Preference implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2008, 2009, 2012 Carlos López
**	Copyright (c) 2014 Yu Chen
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "dialogs/dialog_setup.h"

#include <gtkmm/eventbox.h>
#include <gtkmm/scale.h>

#include "canvasview.h"
#include "widgets/widget_enum.h"
#include "autorecover.h"
#include "duck.h"

#include <ETL/stringf>
#include <ETL/misc>

#include <synfig/rendering/renderer.h>

#include <synfigapp/canvasinterface.h>
#include <synfigapp/main.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_Setup::Dialog_Setup(Gtk::Window& parent):
	Dialog_Template(parent,_("Synfig Studio Preferences")),
	input_settings(synfigapp::Main::get_selected_input_device()->settings()),
	adj_recent_files(Gtk::Adjustment::create(15,1,50,1,1,0)),
	adj_undo_depth(Gtk::Adjustment::create(100,10,5000,1,1,1)),
	time_format(Time::FORMAT_NORMAL),
	listviewtext_brushes_path(manage (new Gtk::ListViewText(1, true, Gtk::SELECTION_BROWSE))),
	adj_pref_x_size(Gtk::Adjustment::create(480,1,10000,1,10,0)),
	adj_pref_y_size(Gtk::Adjustment::create(270,1,10000,1,10,0)),
	adj_pref_fps(Gtk::Adjustment::create(24.0,1.0,100,0.1,1,0)),
	pref_modification_flag(false),
	refreshing(false)
{
	synfig::String
		interface_str(_("Interface")),
		document_str(_("Document")),
		editing_str(_("Editing")),
		render_str(_("Render")),
		system_str(_("System"));
	// WARNING FIXED ORDER : the page added to notebook same has treeview
	// Interface
	create_interface_page(add_page(interface_str));
	// Document
	create_document_page(add_page(document_str));
	// Editing
	create_editing_page(add_page(editing_str));
	// Render
	create_render_page(add_page(render_str));
	// System
	create_system_page(add_page(system_str));

	show_all_children();
}

Dialog_Setup::~Dialog_Setup()
{
}

void
Dialog_Setup::create_system_page(PageInfo pi)
{
	/*---------System--------------------*\
	 * UNITS
	 *  Timestamp  [_____________________]
	 *  UnitSystem [_____________________]
	 * RECENTFILE  [_____________________]
	 * AUTOBACKUP  [x| ]
	 *  Interval   [_____________________]
	 * BROWSER     [_____________________]
	 * BRUSH       [_____________________]
	 */

	int row(1);
	// System _ Units section
	attach_label_section(pi.grid, _("Units"), row);
	// System - 0 Timestamp
	attach_label(pi.grid, _("Timestamp"), ++row);
	pi.grid->attach(timestamp_comboboxtext, 1, row, 1, 1);
	timestamp_comboboxtext.set_hexpand(true);

	#define ADD_TIMESTAMP(desc,x) {				\
		timestamp_comboboxtext.append(desc);	\
		time_formats[desc] = x;					\
	}

	ADD_TIMESTAMP("HH:MM:SS.FF",		Time::FORMAT_VIDEO	);
	ADD_TIMESTAMP("(HHh MMm SSs) FFf",	Time::FORMAT_NORMAL	);
	ADD_TIMESTAMP("(HHhMMmSSs)FFf",		Time::FORMAT_NORMAL	| Time::FORMAT_NOSPACES	);
	ADD_TIMESTAMP("HHh MMm SSs FFf",	Time::FORMAT_NORMAL	| Time::FORMAT_FULL		);
	ADD_TIMESTAMP("HHhMMmSSsFFf",		Time::FORMAT_NORMAL	| Time::FORMAT_NOSPACES	| Time::FORMAT_FULL);
	ADD_TIMESTAMP("FFf",				Time::FORMAT_FRAMES );

	#undef ADD_TIMESTAMP

	timestamp_comboboxtext.signal_changed().connect(
		sigc::mem_fun(*this, &Dialog_Setup::on_time_format_changed) );

	// System - 1 Unit system
	{
		ParamDesc param_desc;
		param_desc
			.set_hint("enum")
			.add_enum_value(Distance::SYSTEM_UNITS,"u",_("Units"))
			.add_enum_value(Distance::SYSTEM_PIXELS,"px",_("Pixels"))
			.add_enum_value(Distance::SYSTEM_POINTS,"pt",_("Points"))
			.add_enum_value(Distance::SYSTEM_INCHES,"in",_("Inches"))
			.add_enum_value(Distance::SYSTEM_METERS,"m",_("Meters"))
			.add_enum_value(Distance::SYSTEM_CENTIMETERS,"cm",_("Centimeters"))
			.add_enum_value(Distance::SYSTEM_MILLIMETERS,"mm",_("Millimeters"));

		widget_enum=manage(new Widget_Enum());
		widget_enum->set_param_desc(param_desc);

		attach_label(pi.grid, _("Unit System"), ++row);
		pi.grid->attach(*widget_enum, 1, row, 1, 1);
		widget_enum->set_hexpand(true);
	}

	// System - Recent files
	attach_label_section(pi.grid, _("Recent Files"), ++row);
	Gtk::SpinButton* recent_files_spinbutton(manage(new Gtk::SpinButton(adj_recent_files,1,0)));
	pi.grid->attach(*recent_files_spinbutton, 1, row, 1, 1);

	// System - Auto backup interval
	attach_label_section(pi.grid, _("Autosave"), ++row);
	pi.grid->attach(toggle_autobackup, 1, row, 1, 1);
	toggle_autobackup.set_hexpand(false);
	toggle_autobackup.set_halign(Gtk::ALIGN_START);
	toggle_autobackup.property_active().signal_changed().connect(
			sigc::mem_fun(*this, &Dialog_Setup::on_autobackup_changed));

	attach_label(pi.grid, _("Interval"), ++row);
	pi.grid->attach(auto_backup_interval, 1, row, 1, 1);
	auto_backup_interval.set_hexpand(false);

	// System - Brushes path
	{
		attach_label_section(pi.grid, _("Brush Presets Path"), ++row);
		// TODO Check if Gtk::ListStore::create need something like manage
		brushpath_refmodel = Gtk::ListStore::create(prefs_brushpath);
		listviewtext_brushes_path->set_model(brushpath_refmodel);

		Gtk::ScrolledWindow* scroll(manage (new Gtk::ScrolledWindow()));
		scroll->add(*listviewtext_brushes_path);
		listviewtext_brushes_path->set_headers_visible(false);
		pi.grid->attach(*scroll, 1, row, 1, 3);

		// Brushes path buttons
		Gtk::Grid* brush_path_btn_grid(manage (new Gtk::Grid()));
		Gtk::Button* brush_path_add(manage (new Gtk::Button()));
		brush_path_add->set_image_from_icon_name("add", Gtk::ICON_SIZE_BUTTON);
		brush_path_btn_grid->attach(*brush_path_add, 0, 0, 1, 1);
		brush_path_add->set_halign(Gtk::ALIGN_END);
		brush_path_add->signal_clicked().connect(
				sigc::mem_fun(*this, &Dialog_Setup::on_brush_path_add_clicked));
		Gtk::Button* brush_path_remove(manage (new Gtk::Button()));
		brush_path_remove->set_image_from_icon_name("remove", Gtk::ICON_SIZE_BUTTON);
		brush_path_btn_grid->attach(*brush_path_remove, 0, 1, 1, 1);
		brush_path_remove->set_halign(Gtk::ALIGN_END);
		brush_path_remove->signal_clicked().connect(
				sigc::mem_fun(*this, &Dialog_Setup::on_brush_path_remove_clicked));
		pi.grid->attach(*brush_path_btn_grid, 0, ++row, 1, 2);
		brush_path_btn_grid->set_halign(Gtk::ALIGN_END);
		++row;
	}
	// System - 11 enable_experimental_features
	attach_label_section(pi.grid, _("Experimental features (requires restart)"), ++row);
	pi.grid->attach(toggle_enable_experimental_features, 1, row, 1, 1);
	toggle_enable_experimental_features.set_halign(Gtk::ALIGN_START);
	toggle_enable_experimental_features.set_hexpand(false);

	// signal for change resume
	auto_backup_interval.signal_changed().connect(
			sigc::bind<int>(sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_AUTOBACKUP));
	toggle_autobackup.property_active().signal_changed().connect(
			sigc::bind<int>(sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_AUTOBACKUP));
}

void
Dialog_Setup::create_document_page(PageInfo pi)
{
	/*---------Document------------------*\
	 * NEW CANVAS
	 *  prefix  ___________________
	 *  fps   [_]                    [FPS]
	 *  size  H[_]xW[_]      [resolutions]
	 * DEFAULT BACKGROUND
	 * (*) None (Transparent)
	 * ( ) Solid Color         [colorbutton]
	 * ( ) Image               [file path  ]
	 *
	 *
	 *
	 */

	int row(1);
	attach_label_section(pi.grid, _("New Canvas"), row);
	// Document - Preferred file name prefix
	attach_label(pi.grid, _("Name prefix"), ++row);
	pi.grid->attach(textbox_custom_filename_prefix, 1, row, 2, 1);
	textbox_custom_filename_prefix.set_tooltip_text( _("File name prefix for the new created document"));
	textbox_custom_filename_prefix.set_hexpand(true);

	// TODO add label with some FPS description ( ex : 23.976 FPS->NTSC television , 25 PAL, 48->Film Industrie, 30->cinematic-like appearance ...)
	// Document - New Document FPS
	attach_label(pi.grid,_("FPS"), ++row);
	pref_fps_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_fps, 1, 3));
	pi.grid->attach(*pref_fps_spinbutton, 1, row, 1, 1);
	pref_fps_spinbutton->set_tooltip_text(_("Frames per second of the new created document"));
	pref_fps_spinbutton->set_hexpand(true);

	//Document - Template for predefined fps
	fps_template_combo = Gtk::manage(new Gtk::ComboBoxText());
	pi.grid->attach(*fps_template_combo, 2, row, 1 ,1);
	fps_template_combo->set_halign(Gtk::ALIGN_END);
	fps_template_combo->signal_changed().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_fps_template_combo_change));
	//Document - Fill the FPS combo box with proper strings (not localised)
	float f[8];
	f[0] = 60;
	f[1] = 50;
	f[2] = 30;
	f[3] = 25;
	f[4] = 24.967;
	f[5] = 24;
	f[6] = 15;
	f[7] = 12;
	for (int i=0; i<8; i++)
		fps_template_combo->prepend(strprintf("%5.3f", f[i]));

	//Document - Size
	attach_label(pi.grid, _("Size"),++row);
	// TODO chain icon for ratio / ratio indication (see Widget_RendDesc)
	// Document - New Document X size
	Gtk::Grid *grid_size(manage (new Gtk::Grid()));

	Gtk::Label* label = attach_label(grid_size,_("Width"), 0, 0);
	label->set_halign(Gtk::ALIGN_END);

	pref_x_size_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_x_size, 1, 0));
	grid_size->attach(*pref_x_size_spinbutton, 1, 0, 1, 1);
	pref_x_size_spinbutton->set_tooltip_text(_("Width in pixels of the new created document"));
	pref_x_size_spinbutton->set_hexpand(true);

	label = attach_label(grid_size, "X", 0, 2, false);// "X" stand for multiply operation
	// NOTA : Use of "section" attributes for BOLDING
	label->set_attributes(section_attrlist);
	label->set_margin_start(3);
	label->set_margin_end(3);
	label->set_halign(Gtk::ALIGN_CENTER);

	// Document - New Document Y size
	attach_label(grid_size,_("Height"), 0, 3);
	pref_y_size_spinbutton = Gtk::manage(new Gtk::SpinButton(adj_pref_y_size, 1, 0));
	grid_size->attach(*pref_y_size_spinbutton, 4, 0, 1, 1);
	pref_y_size_spinbutton->set_tooltip_text(_("Height in pixels of the new created document"));
	pref_y_size_spinbutton->set_hexpand(true);
	pi.grid->attach(*grid_size, 1, row, 1,1);

	//Document - Template for predefined sizes of canvases.
	size_template_combo = Gtk::manage(new Gtk::ComboBoxText());
	pi.grid->attach(*size_template_combo, 2, row, 1 ,1);
	size_template_combo->set_halign(Gtk::ALIGN_END);
	size_template_combo->signal_changed().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_size_template_combo_change));
	size_template_combo->prepend(_("4096x3112 Full Aperture 4K"));
	size_template_combo->prepend(_("2048x1556 Full Aperture Native 2K"));
	size_template_combo->prepend(_("1920x1080 HDTV 1080p/i"));
	size_template_combo->prepend(_("1280x720  HDTV 720p"));
	size_template_combo->prepend(_("720x576   DVD PAL"));
	size_template_combo->prepend(_("720x480   DVD NTSC"));
	size_template_combo->prepend(_("720x540   Web 720x"));
	size_template_combo->prepend(_("720x405   Web 720x HD"));
	size_template_combo->prepend(_("640x480   Web 640x"));
	size_template_combo->prepend(_("640x360   Web 640x HD"));
	size_template_combo->prepend(_("480x360   Web 480x"));
	size_template_combo->prepend(_("480x270   Web 480x HD"));
	size_template_combo->prepend(_("360x270   Web 360x"));
	size_template_combo->prepend(_("360x203   Web 360x HD"));
	size_template_combo->prepend(DEFAULT_PREDEFINED_SIZE);

	fps_template_combo->prepend(DEFAULT_PREDEFINED_FPS);

	attach_label_section(pi.grid, _("Default Background"), ++row);
	//attach_label(pi.grid, _("Name prefix"), ++row);
	//pi.grid->attach(textbox_custom_filename_prefix, 1, row, 2, 1);
	//textbox_custom_filename_prefix.set_tooltip_text( _("File name prefix for the new created document"));
	//textbox_custom_filename_prefix.set_hexpand(true);

	//Gtk::RadioButton::Group group_def_background;
	def_background_none.set_label(_("None (Transparent)"));
	def_background_none.set_group(group_def_background);
	pi.grid->attach(def_background_none,  0, ++row, 1, 1);
	def_background_none.signal_clicked().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_def_background_type_changed) );

	def_background_color.set_label(_("Solid Color"));
	def_background_color.set_group(group_def_background);
	pi.grid->attach(def_background_color, 0, ++row, 1, 1);
	def_background_color.signal_clicked().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_def_background_type_changed) );

	Gdk::RGBA m_color;
	m_color.set_rgba( App::default_background_layer_color.get_r(),
					  App::default_background_layer_color.get_g(),
					  App::default_background_layer_color.get_b(),
					  App::default_background_layer_color.get_a());
	def_background_color_button.set_rgba(m_color);
	pi.grid->attach(def_background_color_button, 1,   row, 1, 1);
	def_background_color_button.signal_color_set().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_def_background_color_changed) );

	def_background_image.set_label(_("Image"));
	def_background_image.set_group(group_def_background);
	pi.grid->attach(def_background_image,      0, ++row, 1, 1);
	def_background_image.signal_clicked().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_def_background_type_changed) );
	//<!- Button to select the image
	fcbutton_image.set_title(_("Select"));
	fcbutton_image.set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);
	fcbutton_image.set_filename(App::default_background_layer_image);
	/*
	filter_images.set_name(_("Images (*.bmp,*.jpg,*.jpeg,*.png,*.svg,*.lst)"));
		filter_images.add_pattern("*.bmp");
		filter_images.add_pattern("*.jpg");
		filter_images.add_pattern("*.jpeg");
		filter_images.add_pattern("*.png");
		filter_images.add_pattern("*.svg");
		filter_images.add_pattern("*.lst");
		filter_any.set_name(_("Any file (*.*)"));
		filter_any.add_pattern("*.*");

	fcbutton_image.add_filter(filter_images);
	fcbutton_image.add_filter(filter_any);
	*/
	//-> Button to select the image
	pi.grid->attach(fcbutton_image,            1,   row, 2, 1);
	fcbutton_image.signal_file_set().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_def_background_image_set) );

	if (App::default_background_layer_type == "none")        def_background_none.set_active();
	if (App::default_background_layer_type == "solid_color") def_background_color.set_active();
	if (App::default_background_layer_type == "image")       def_background_image.set_active();

}

void
Dialog_Setup::create_editing_page(PageInfo pi)
{
	/*---------Editing------------------*\
	 * IMPORTED IMAGE
	 *  [x] Scale to fit
	 * OTHER
	 *  [x] Linear color
	 *  [x] Restrict radius
	 * EDIT IN EXTERNAL
	 * 	Preferred image editor [image_editor_path] (choose..)
	 *
	 */

	int row(1);
	// Editing Imported image section
	attach_label_section(pi.grid, _("Imported Image"), row);

	// Editing - Scaling New Imported Images to Fit Canvas
	attach_label(pi.grid,_("Scale to fit canvas"), ++row);
	pi.grid->attach(toggle_resize_imported_images, 1, row, 1, 1);
	toggle_resize_imported_images.set_tooltip_text(_("When you import images, check this option if you want they fit the Canvas size."));
	toggle_resize_imported_images.set_halign(Gtk::ALIGN_START);
	toggle_resize_imported_images.set_hexpand(false);

	// Editing Other section
	attach_label_section(pi.grid, _("Other"), ++row);

	// Editing - Restrict Real-value Handles to Top Right Quadrant
	attach_label(pi.grid,_("Restrict real value handles to top right quadrant"), ++row);
	pi.grid->attach(toggle_restrict_radius_ducks, 1, row, 1, 1);
	toggle_restrict_radius_ducks.set_halign(Gtk::ALIGN_START);
	toggle_restrict_radius_ducks.set_hexpand(false);
	toggle_restrict_radius_ducks.set_tooltip_text(_("Restrict the position of the handle \
(especially for radius) to be in the top right quadrant of the 2D space. Allow to set \
the real value to any number and also easily reach the value of 0.0 just \
dragging the handle to the left bottom part of your 2D space."));

	attach_label_section(pi.grid, _("Edit in external"), ++row);

	attach_label(pi.grid,_("Preferred image editor"), ++row);

	//create a button that will open the filechooserdialog to select image editor
	Gtk::Button *choose_button(manage(new class Gtk::Button(Gtk::StockID(_("Choose...")))));
	choose_button->show();
	choose_button->set_tooltip_text(_("Choose the preferred Image editor for Edit in external tool option"));
	
	//create a function to launch the dialog
	choose_button->signal_clicked().connect(sigc::mem_fun(*this,&Dialog_Setup::on_choose_editor_pressed));
	pi.grid->attach(image_editor_path_entry, 1, row, 1, 1);
	pi.grid->attach(*choose_button, 2,row,1,1);
	image_editor_path_entry.set_hexpand(true);
	image_editor_path_entry.set_text(App::image_editor_path);
	
}


void
Dialog_Setup::on_choose_editor_pressed()
{
	//set the image editor path = filepath from dialog
	String filepath = image_editor_path_entry.get_text();
	if (select_path_dialog(_("Select Editor"), filepath)) {
		image_editor_path_entry.set_text(filepath);
		App::image_editor_path = filepath;
	}
}

bool 
Dialog_Setup::select_path_dialog(const std::string &title, std::string &filepath)
{
	Gtk::FileChooserDialog *dialog = new Gtk::FileChooserDialog(*App::main_window,title, Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog->set_transient_for(*App::main_window);
	#ifdef WIN32
	dialog->set_current_folder("C:\\Program Files");

	#elif defined(__APPLE__)
    dialog->set_current_folder("/Applications");

	#else
    	dialog->set_current_folder("/usr/bin");
	#endif

	//Add response buttons the the dialog:
	dialog->add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
	dialog->add_button(_("Select"), Gtk::RESPONSE_OK);
  	if(dialog->run() == Gtk::RESPONSE_OK) {
		filepath = dialog->get_filename();
		filepath = absolute_path(filepath);	//get the absolute path
		delete dialog;
		return true;
	}
	delete dialog;
	return false;
}

void
Dialog_Setup::create_render_page(PageInfo pi)
{
	/*---------Render------------------*\
	 *
	 *  sequence separator _________
	 *   workarea  [ Legacy ]
	 *   play sound on render done  [x| ]
	 *
	 */

	int row(1);
	// Render - Image sequence separator
	attach_label(pi.grid, _("Image Sequence Separator String"), row);
	pi.grid->attach(image_sequence_separator, 1, row, 1, 1);
	image_sequence_separator.set_hexpand(true);

	// Render - WorkArea
	attach_label(pi.grid, _("WorkArea renderer"), ++row);
	pi.grid->attach(workarea_renderer_combo, 1, row, 1, 1);
	// Render - Render Done sound
	attach_label(pi.grid, _("Chime on render done"), ++row);
	pi.grid->attach(toggle_play_sound_on_render_done, 1, row, 1, 1);
	toggle_play_sound_on_render_done.set_halign(Gtk::ALIGN_START);
	toggle_play_sound_on_render_done.set_hexpand(false);
	toggle_play_sound_on_render_done.set_tooltip_text(_("A chime is played when render has finished."));
	toggle_play_sound_on_render_done.property_active().signal_changed().connect(
			sigc::mem_fun(*this, &Dialog_Setup::on_play_sound_on_render_done_changed));

	synfig::rendering::Renderer::Handle default_renderer = synfig::rendering::Renderer::get_renderer("");
	workarea_renderer_combo.append("", String() + _("Default") + " - " + default_renderer->get_name());
	typedef std::map<synfig::String, synfig::rendering::Renderer::Handle> RendererMap;
	const RendererMap &renderers = synfig::rendering::Renderer::get_renderers();
	for(RendererMap::const_iterator i = renderers.begin(); i != renderers.end(); ++i)
	{
		assert(!i->first.empty());
		workarea_renderer_combo.append(i->first, i->second->get_name());
	}

	attach_label(pi.grid, _("Preview Background Color"), ++row);

	Gdk::RGBA m_color;
	m_color.set_rgba( App::preview_background_color.get_r(),
	                  App::preview_background_color.get_g(),
	                  App::preview_background_color.get_b(),
	                  App::preview_background_color.get_a());
	preview_background_color_button.set_rgba(m_color);
	pi.grid->attach(preview_background_color_button, 1, row, 1, 1);
	preview_background_color_button.signal_color_set().connect(
		sigc::mem_fun(*this, &studio::Dialog_Setup::on_preview_background_color_changed) );

}

void
Dialog_Setup::create_interface_page(PageInfo pi)
{
	/*---------Interface------------------*\
	 * LANGUAGE
	 *  [________________________________]
	 * COLORTHEME
	 *  DarkUI          [x]
	 * HANDLETOOLTIP
	 *  Widthpoint      [x| ]
	 *  Radius          [x| ]
	 *  Transformation  [x| ]
	 *        [x] Name
	 *        [x] Value
	 */

	// Interface - UI Language

	static const char* languages[][2] = {
		#include <languages.inc.c>
		{ NULL, NULL } // final entry without comma to avoid misunderstanding
	};

	ui_language_combo.append("os_LANG", Glib::ustring("(") + _("System Language") + ")");
	for(int i = 0; i < (int)(sizeof(languages)/sizeof(languages[0])) - 1; ++i)
		if (languages[i][1] == Glib::ustring())
			ui_language_combo.append(languages[i][0], Glib::ustring("[") + languages[i][0] + "]");
		else
			ui_language_combo.append(languages[i][0], languages[i][1]);
	ui_language_combo.set_active_id(App::ui_language);
	ui_language_combo.signal_changed().connect(sigc::mem_fun(*this, &studio::Dialog_Setup::on_ui_language_combo_change));

	int row = 1;

	// Interface - Language section
	attach_label_section(pi.grid, _("Language"), row);
	pi.grid->attach(ui_language_combo, 0, ++row, 4, 1);
	ui_language_combo.set_hexpand(true);
	ui_language_combo.set_margin_start(10);

	// Interface - Color Theme section
	attach_label_section(pi.grid, _("Color Theme"), ++row);
	// Interface - Dark UI theme
	attach_label(pi.grid, _("Dark UI theme (if available)"), ++row);
	pi.grid->attach(toggle_use_dark_theme, 1, row, 1, 1);
	toggle_use_dark_theme.set_halign(Gtk::ALIGN_START);
	toggle_use_dark_theme.set_hexpand(false);

	// Interface - Toolbars section
	attach_label_section(pi.grid, _("Toolbars"), ++row);
	// Interface - File Toolbar
	attach_label(pi.grid, _("Show file toolbar (requires restart)"), ++row);
	pi.grid->attach(toggle_show_file_toolbar, 1, row, 1, 1);
	toggle_show_file_toolbar.set_halign(Gtk::ALIGN_START);
	toggle_show_file_toolbar.set_hexpand(false);

	// Interface - Handle tooltip section
	attach_label_section(pi.grid, _("Handle Tooltips"), ++row);
	// Interface - width point tooltip
	attach_label(pi.grid, _("Width point"), ++row);
	pi.grid->attach(toggle_handle_tooltip_widthpoint, 1, row, 1, 1);
	toggle_handle_tooltip_widthpoint.set_halign(Gtk::ALIGN_START);
	toggle_handle_tooltip_widthpoint.set_hexpand(false);
	// Interface - radius tooltip
	attach_label(pi.grid, _("Radius"), ++row);
	pi.grid->attach(toggle_handle_tooltip_radius, 1, row, 1, 1);
	toggle_handle_tooltip_radius.set_halign(Gtk::ALIGN_START);
	toggle_handle_tooltip_radius.set_hexpand(false);
	// Interface - transformation widget tooltip
	attach_label_section(pi.grid, _("Transformation widget tooltips"), ++row);
	pi.grid->attach(toggle_handle_tooltip_transformation, 1, row, 1, 1);
	toggle_handle_tooltip_transformation.set_halign(Gtk::ALIGN_START);
	toggle_handle_tooltip_transformation.set_hexpand(false);
	toggle_handle_tooltip_transformation.property_active().signal_changed().connect(
		sigc::mem_fun(*this, &Dialog_Setup::on_tooltip_transformation_changed));

	attach_label(pi.grid, _("Name"), ++row);// HANDLE_TOOLTIP_TRANSFO_NAME
	pi.grid->attach(toggle_handle_tooltip_transfo_name, 1, row, 1, 1);
	toggle_handle_tooltip_transfo_name.set_halign(Gtk::ALIGN_START);
	toggle_handle_tooltip_transfo_name.set_hexpand(false);
	attach_label(pi.grid, _("Value"), ++row);// HANDLE_TOOLTIP_TRANSFO_VALUE
	pi.grid->attach(toggle_handle_tooltip_transfo_value, 1, row, 1, 1);
	toggle_handle_tooltip_transfo_value.set_halign(Gtk::ALIGN_START);
	toggle_handle_tooltip_transfo_value.set_hexpand(false);

	//! change resume signal connexion
	ui_language_combo.signal_changed().connect(
			sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_LANGUAGE));
	//TODO signal change on value
	//toggle_use_dark_theme.signal_changed().connect(
	//	sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_THEME));
	toggle_handle_tooltip_widthpoint.property_active().signal_changed().connect(
		sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
	toggle_handle_tooltip_radius.property_active().signal_changed().connect(
		sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
	toggle_handle_tooltip_transformation.property_active().signal_changed().connect(
		sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
	toggle_handle_tooltip_transfo_name.property_active().signal_changed().connect(
		sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
	toggle_handle_tooltip_transfo_value.property_active().signal_changed().connect(
		sigc::bind<int> (sigc::mem_fun(*this, &Dialog_Setup::on_value_change), CHANGE_UI_HANDLE_TOOLTIP));
}

void
Dialog_Setup::on_restore_pressed()
{
	App::restore_default_settings();
	refresh();
}


void
Dialog_Setup::on_apply_pressed()
{
	App::set_max_recent_files((int)adj_recent_files->get_value());

	// Set the time format
	App::set_time_format(get_time_format());

	//if(pref_modification_flag&CHANGE_AUTOBACKUP)
	// TODO catch change event on auto_backup_interval before use CHANGE_AUTOBACKUP
	{
		// Set the auto backup status
		App::auto_recover->set_enabled(toggle_autobackup.get_active());
		// Set the auto backup interval
		App::auto_recover->set_timeout_ms(auto_backup_interval.get_value() * 1000);
	}

	App::distance_system              = Distance::System(widget_enum->get_value());

	// Set the restrict_radius_ducks flag
	App::restrict_radius_ducks        = toggle_restrict_radius_ducks.get_active();

	// Set the resize_imported_images flag
	App::resize_imported_images       = toggle_resize_imported_images.get_active();

	// Set the experimental features flag
	App::enable_experimental_features = toggle_enable_experimental_features.get_active();

	// Set the dark theme flag
	App::use_dark_theme               = toggle_use_dark_theme.get_active();
	App::apply_gtk_settings();

	// Set file toolbar flag
	App::show_file_toolbar=toggle_show_file_toolbar.get_active();

	//! TODO Create Change mechanism has Class for being used elsewhere
	// Set the preferred brush path(s)
	if (pref_modification_flag&CHANGE_BRUSH_PATH)
	{
		App::brushes_path.clear();
		int path_count = 0;

		Glib::RefPtr<Gtk::ListStore> liststore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
			listviewtext_brushes_path->get_model());

		for(Gtk::TreeIter ui_iter = liststore->children().begin();
			ui_iter!=liststore->children().end();ui_iter++)
		{
			const Gtk::TreeRow row = *(ui_iter);
			// TODO utf_8 path : care to other locale than english ?
			synfig::String path((row[prefs_brushpath.path]));
			input_settings.set_value(strprintf("brush.path_%d", path_count++), path);
			App::brushes_path.insert(path);
		}
		input_settings.set_value("brush.path_count", strprintf("%d", path_count));
	}

	// Set the preferred file name prefix
	App::custom_filename_prefix = textbox_custom_filename_prefix.get_text();

	// Set the preferred image editor
	App::image_editor_path = image_editor_path_entry.get_text();

	// Set the preferred new Document X dimension
	App::preferred_x_size       = int(adj_pref_x_size->get_value());

	// Set the preferred new Document Y dimension
	App::preferred_y_size       = int(adj_pref_y_size->get_value());

	// Set the preferred Predefined size
	App::predefined_size        = size_template_combo->get_active_text();

	// Set the preferred Predefined fps
	App::predefined_fps         = fps_template_combo->get_active_text();

	// Set the preferred FPS
	App::preferred_fps          = Real(adj_pref_fps->get_value());

	// Set the preferred image sequence separator
	App::sequence_separator     = image_sequence_separator.get_text();

	// Set the workarea render and navigator render flag
	App::navigator_renderer = App::workarea_renderer  = workarea_renderer_combo.get_active_id();

	// Set the use of a render done sound
	App::use_render_done_sound  = toggle_play_sound_on_render_done.get_active();

	// Set ui language
	if (pref_modification_flag & CHANGE_UI_LANGUAGE)
		App::ui_language = ui_language_combo.get_active_id().c_str();

	if (pref_modification_flag & CHANGE_UI_HANDLE_TOOLTIP)
	{
		// Set ui tooltip on width point
		App::ui_handle_tooltip_flag=toggle_handle_tooltip_widthpoint.get_active()?Duck::STRUCT_WIDTHPOINT:Duck::STRUCT_NONE;
		// Set ui tooltip on radius
		App::ui_handle_tooltip_flag |= toggle_handle_tooltip_radius.get_active()?Duck::STRUCT_RADIUS:Duck::STRUCT_NONE;
		// Set ui tooltip on transformation
		if(toggle_handle_tooltip_transformation.get_active())
		{
			if(toggle_handle_tooltip_transfo_name.get_active())
			{
				App::ui_handle_tooltip_flag |= Duck::STRUCT_TRANSFORMATION;
			}
			if(toggle_handle_tooltip_transfo_value.get_active())
			{
				App::ui_handle_tooltip_flag |= Duck::STRUCT_TRANSFO_BY_VALUE;
			}
		}
	}


	App::save_settings();
	App::setup_changed();

	if ((pref_modification_flag&CHANGE_BRUSH_PATH) &&
			String(App::get_selected_canvas_view()->get_smach().get_state_name()) == String("brush"))
	{
		App::get_selected_canvas_view()->get_smach().process_event(EVENT_REFRESH_TOOL_OPTIONS);
	}

}

void
Dialog_Setup::on_size_template_combo_change()
{
	String selection(size_template_combo->get_active_text());
	if(selection==DEFAULT_PREDEFINED_SIZE)
	{
		pref_y_size_spinbutton->set_sensitive(true);
		pref_x_size_spinbutton->set_sensitive(true);
		return;
	}
	String::size_type locx=selection.find_first_of("x"); // here should be some comparison with string::npos
	String::size_type locspace=selection.find_first_of(" ");
	String x_size(selection.substr(0,locx));
	String y_size(selection.substr(locx+1,locspace));
	int x=atoi(x_size.c_str());
	int y=atoi(y_size.c_str());
	adj_pref_x_size->set_value(x);
	adj_pref_y_size->set_value(y);
	pref_y_size_spinbutton->set_sensitive(false);
	pref_x_size_spinbutton->set_sensitive(false);

	return;
}


void
Dialog_Setup::on_ui_language_combo_change()
{
}


void
Dialog_Setup::on_fps_template_combo_change()
{
	String selection(fps_template_combo->get_active_text());
	if(selection==DEFAULT_PREDEFINED_FPS)
	{
		pref_fps_spinbutton->set_sensitive(true);
		return;
	}
	adj_pref_fps->set_value(atof(selection.c_str()));
	pref_fps_spinbutton->set_sensitive(false);
	return;
}

void
Dialog_Setup::on_time_format_changed()
{
	std::map<std::string, synfig::Time::Format>::iterator i =
		time_formats.find(timestamp_comboboxtext.get_active_text());
	if (i != time_formats.end())
		time_format = i->second;
}

void
Dialog_Setup::on_autobackup_changed()
{
	auto_backup_interval.set_sensitive(toggle_autobackup.get_active());
	App::auto_recover->set_enabled(toggle_autobackup.get_active());
}

void
Dialog_Setup::on_play_sound_on_render_done_changed()
{
	App::use_render_done_sound = toggle_play_sound_on_render_done.get_active();
}

void
Dialog_Setup::on_def_background_type_changed()
{
	if (def_background_none.get_active())  App::default_background_layer_type = "none";
	if (def_background_color.get_active()) App::default_background_layer_type = "solid_color";
	if (def_background_image.get_active()) App::default_background_layer_type = "image";
}

void
Dialog_Setup::on_def_background_color_changed()
{
	Gdk::RGBA m_color = def_background_color_button.get_rgba();

	App::default_background_layer_color =
		synfig::Color(m_color.get_red(),
		              m_color.get_green(),
		              m_color.get_blue(),
		              m_color.get_alpha());
}

void
Dialog_Setup::on_def_background_image_set()
{
	App::default_background_layer_image = fcbutton_image.get_filename();
}

void
Dialog_Setup::on_preview_background_color_changed()
{
	Gdk::RGBA m_color = preview_background_color_button.get_rgba();

	App::preview_background_color =
		synfig::Color(m_color.get_red(),
		              m_color.get_green(),
		              m_color.get_blue(),
		              m_color.get_alpha());
	//studio::Widget_Preview::
}

void
Dialog_Setup::on_tooltip_transformation_changed()
{
	toggle_handle_tooltip_transfo_name.set_sensitive(toggle_handle_tooltip_transformation.get_active());
	toggle_handle_tooltip_transfo_value.set_sensitive(toggle_handle_tooltip_transformation.get_active());
}

void
Dialog_Setup::refresh()
{
	refreshing = true;
	pref_modification_flag = CHANGE_NONE;
	
	adj_recent_files->set_value(App::get_max_recent_files());

	// Refresh the ui language
	ui_language_combo.set_active_id(App::ui_language);
	
	// Refresh the time format
	set_time_format(App::get_time_format());

	widget_enum->set_value(App::distance_system);

	toggle_autobackup.set_active(App::auto_recover->get_enabled());
	// Refresh the value of the auto backup interval
	auto_backup_interval.set_value(App::auto_recover->get_timeout_ms() / 1000);
	auto_backup_interval.set_sensitive(App::auto_recover->get_enabled());

	// Refresh the status of the restrict_radius_ducks flag
	toggle_restrict_radius_ducks.set_active(App::restrict_radius_ducks);

	// Refresh the status of the resize_imported_images flag
	toggle_resize_imported_images.set_active(App::resize_imported_images);

	// Refresh the status of the experimental features flag
	toggle_enable_experimental_features.set_active(App::enable_experimental_features);

	// Refresh the status of the theme flag
	toggle_use_dark_theme.set_active(App::use_dark_theme);
	App::apply_gtk_settings();

	// Refresh the status of the render done sound flag
	toggle_play_sound_on_render_done.set_active(App::use_render_done_sound);
	
	// Refresh the default background
	if (App::default_background_layer_type == "none")        def_background_none.set_active();
	if (App::default_background_layer_type == "solid_color") def_background_color.set_active();
	if (App::default_background_layer_type == "image")       def_background_image.set_active();
	
	// Refresh the colors of background and preview background buttons
	Gdk::RGBA m_color;
	m_color.set_rgba( App::default_background_layer_color.get_r(),
					  App::default_background_layer_color.get_g(),
					  App::default_background_layer_color.get_b(),
					  App::default_background_layer_color.get_a());
	def_background_color_button.set_rgba(m_color);
	
	m_color.set_rgba( App::preview_background_color.get_r(),
					  App::preview_background_color.get_g(),
					  App::preview_background_color.get_b(),
					  App::preview_background_color.get_a());
	preview_background_color_button.set_rgba(m_color);

	// Refresh the status of file toolbar flag
	toggle_show_file_toolbar.set_active(App::show_file_toolbar);

	// Refresh the preferred image editor path
	image_editor_path_entry.set_text(App::image_editor_path);

	// Refresh the brush path(s)
	Glib::RefPtr<Gtk::ListStore> liststore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
			listviewtext_brushes_path->get_model());
	//! Keep "brushes_path" preferences entry for backward compatibility (15/12 - v1.0.3)
	//! Now brush path(s) are hold by input preferences : brush.path_count & brush.path_%d
	String value;
	Gtk::TreeIter ui_iter;
	bool bvalue(input_settings.get_value("brush.path_count",value));
	int i(atoi(value.c_str()));
	App::brushes_path.clear();
	liststore->clear();
	if(!bvalue || (bvalue && i<=0))
	{
		App::brushes_path.insert(App::get_base_path()+ETL_DIRECTORY_SEPARATOR+"share"+ETL_DIRECTORY_SEPARATOR+"synfig"+ETL_DIRECTORY_SEPARATOR+"brushes");
	}
	else
	{
		for(int j = 0; j<i;j++)
		{
			if(input_settings.get_value(strprintf("brush.path_%d", j),value))
			{
				App::brushes_path.insert(value);
			}
		}
	}
	for (set<synfig::String>::iterator setiter = App::brushes_path.begin();
			setiter != App::brushes_path.end(); setiter++)
	{
		ui_iter = liststore->append();
		(*ui_iter)[prefs_brushpath.path]=*setiter;
	}
	// Select the first brush path entry
	//listviewtext_brushes_path->get_selection()->select(
	//		listviewtext_brushes_path->get_model()->children().begin());

	// Refresh the preferred filename prefix
	textbox_custom_filename_prefix.set_text(App::custom_filename_prefix);

	// Refresh the preferred new Document X dimension
	adj_pref_x_size->set_value(App::preferred_x_size);

	// Refresh the preferred new Document Y dimension
	adj_pref_y_size->set_value(App::preferred_y_size);

	// Refresh the preferred Predefined size
	size_template_combo->set_active_text(App::predefined_size);

	// Refresh the preferred FPS
	adj_pref_fps->set_value(App::preferred_fps);

	// Refresh the predefined FPS
	fps_template_combo->set_active_text(App::predefined_fps);

	// Refresh the sequence separator
	image_sequence_separator.set_text(App::sequence_separator);

	// Refresh the status of the workarea_renderer
	workarea_renderer_combo.set_active_id(App::workarea_renderer);

	// Refresh ui tooltip handle info
	toggle_handle_tooltip_widthpoint.set_active(App::ui_handle_tooltip_flag&Duck::STRUCT_WIDTHPOINT);
	toggle_handle_tooltip_radius.set_active(App::ui_handle_tooltip_flag&Duck::STRUCT_RADIUS);
	if((App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFORMATION) ||
			(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFO_BY_VALUE))
	{
		toggle_handle_tooltip_transformation.set_active(true);
		toggle_handle_tooltip_transfo_name.set_active(
			(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFORMATION));
		toggle_handle_tooltip_transfo_value.set_active(
			(App::ui_handle_tooltip_flag&Duck::STRUCT_TRANSFO_BY_VALUE));
	}
	else
	{
		toggle_handle_tooltip_transformation.set_active(false);
		toggle_handle_tooltip_transfo_name.set_sensitive(false);
		toggle_handle_tooltip_transfo_value.set_sensitive(false);
	}

	refreshing = false;
}

void
Dialog_Setup::set_time_format(synfig::Time::Format x)
{
	time_format=x;
	if (x == (Time::FORMAT_NORMAL))
		timestamp_comboboxtext.set_active(1);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_NOSPACES))
		timestamp_comboboxtext.set_active(2);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_FULL))
		timestamp_comboboxtext.set_active(3);
	else if (x == (Time::FORMAT_NORMAL | Time::FORMAT_NOSPACES | Time::FORMAT_FULL))
		timestamp_comboboxtext.set_active(4);
	else if (x == (Time::FORMAT_FRAMES))
		timestamp_comboboxtext.set_active(5);
	else if (x <= Time::FORMAT_VIDEO)
		timestamp_comboboxtext.set_active(0);
	else
		timestamp_comboboxtext.set_active(1);
}

void
Dialog_Setup::on_brush_path_add_clicked()
{
	synfig::String foldername;
	//! TODO dialog_add_folder
	if(App::dialog_open_folder(_("Select a new path for brush"), foldername, MISC_DIR_PREFERENCE, *this))
	{
		// add the new path
		Glib::RefPtr<Gtk::ListStore> liststore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
				listviewtext_brushes_path->get_model());
		Gtk::TreeIter it(liststore->append());
		(*it)[prefs_brushpath.path]=foldername;
		// high light it in the brush path list
		listviewtext_brushes_path->scroll_to_row(listviewtext_brushes_path->get_model()->get_path(*it));
		listviewtext_brushes_path->get_selection()->select(listviewtext_brushes_path->get_model()->get_path(*it));

		pref_modification_flag|=CHANGE_BRUSH_PATH;
	}
}

void
Dialog_Setup::on_brush_path_remove_clicked()
{
	Glib::RefPtr<Gtk::ListStore> refLStore = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(listviewtext_brushes_path->get_model());
	refLStore->erase(listviewtext_brushes_path->get_selection()->get_selected());

	pref_modification_flag|=CHANGE_BRUSH_PATH;
	//! TODO if list size == 0: push warning to warning zone
}

void
Dialog_Setup::on_value_change(int valueflag)
{
	if(!refreshing) pref_modification_flag |= valueflag;
}
