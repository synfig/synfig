/* === S Y N F I G ========================================================= */
/*!	\file mainwindow.cpp
**	\brief MainWindow
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#include "mainwindow.h"
#include "canvasview.h"

#include <synfigapp/main.h>

#include <gtkmm/menubar.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#define GRAB_HINT_DATA(y)	{ \
		String x; \
		if(synfigapp::Main::settings().get_value(String("pref.")+y+"_hints",x)) \
		{ \
			set_type_hint((Gdk::WindowTypeHint)atoi(x.c_str()));	\
		} \
	}

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

MainWindow::MainWindow()
{
	set_default_size(600, 400);

	notebook_.show();

	Gtk::Widget* menubar = App::ui_manager()->get_widget("/menubar-main");
	if (menubar != NULL)
	{
		menubar->show();
		vbox_.pack_start(*menubar, false, false, 0);
	}

	vbox_.pack_end(notebook_, true, true, 0);

	vbox_.show();
	add(vbox_);

	add_accel_group(App::ui_manager()->get_accel_group());

	notebook_.signal_switch_page().connect(
		sigc::mem_fun(*this, &studio::MainWindow::on_switch_page) );

	GRAB_HINT_DATA("canvas_view");
}

MainWindow::~MainWindow() { }

void
MainWindow::init_menus()
{
	/*Menus to worry about:
	- filemenu
	- editmenu
	- layermenu
	- duckmaskmenu
	- mainmenu
	- canvasmenu
	- viewmenu
	*/
/*	action_group = Gtk::ActionGroup::create("canvasview");

	//action_group->add( Gtk::Action::create("MenuFile", _("_File")) );
	action_group->add( Gtk::Action::create("new", Gtk::Stock::NEW),
		sigc::hide_return(sigc::ptr_fun(&studio::App::new_instance))
	);
	action_group->add( Gtk::Action::create("open", Gtk::Stock::OPEN),
		sigc::hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::open))
	);
	action_group->add( Gtk::Action::create("save", Gtk::Stock::SAVE),
		hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::save))
	);
	action_group->add( Gtk::Action::create("save-as", Gtk::Stock::SAVE_AS),
		sigc::hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::dialog_save_as))
	);
	action_group->add( Gtk::Action::create("revert", Gtk::Stock::REVERT_TO_SAVED),
		sigc::hide_return(sigc::mem_fun(*get_instance().get(), &studio::Instance::safe_revert))
	);
	action_group->add( Gtk::Action::create("cvs-add", Gtk::StockID("synfig-cvs_add")),
		sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_add))
	);
	action_group->add( Gtk::Action::create("cvs-update", Gtk::StockID("synfig-cvs_update")),
		sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_update))
	);
	action_group->add( Gtk::Action::create("cvs-revert", Gtk::StockID("synfig-cvs_revert")),
		sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_revert))
	);
	action_group->add( Gtk::Action::create("cvs-commit", Gtk::StockID("synfig-cvs_commit")),
		sigc::hide_return(sigc::mem_fun(*get_instance(), &studio::Instance::dialog_cvs_commit))
	);
	action_group->add( Gtk::Action::create("import", _("Import")),
		sigc::hide_return(sigc::mem_fun(*this, &studio::CanvasView::image_import))
	);
	action_group->add( Gtk::Action::create("render", _("Render")),
		sigc::mem_fun0(render_settings,&studio::RenderSettings::present)
	);
	action_group->add( Gtk::Action::create("preview", _("Preview")),
		sigc::mem_fun(*this,&CanvasView::on_preview_option)
	);
	action_group->add( Gtk::Action::create("sound", _("Sound File")),
		sigc::mem_fun(*this,&CanvasView::on_audio_option)
	);
	action_group->add( Gtk::Action::create("options", _("Options")),
		sigc::mem_fun0(canvas_options,&studio::CanvasOptions::present)
	);
	action_group->add( Gtk::Action::create("close", Gtk::StockID("gtk-close"), _("Close Window")),
		sigc::hide_return(sigc::mem_fun(*this,&studio::CanvasView::close_view))
	);
	action_group->add( Gtk::Action::create("close-document", Gtk::StockID("gtk-close"), _("Close Document")),
		sigc::hide_return(sigc::mem_fun(*this,&studio::CanvasView::close_instance))
	);
	action_group->add( Gtk::Action::create("quit", Gtk::StockID("gtk-quit"), _("Quit")),
		sigc::hide_return(sigc::ptr_fun(&studio::App::quit))
	);

	//action_group->add( Gtk::Action::create("undo", Gtk::StockID("gtk-undo")),
	//	SLOT_EVENT(EVENT_UNDO)
	//);

	//action_group->add( Gtk::Action::create("redo", Gtk::StockID("gtk-redo")),
	//	SLOT_EVENT(EVENT_REDO)
	//);

	action_group->add( Gtk::Action::create("select-all-ducks", _("Select All Handles")),
		sigc::mem_fun(*work_area,&studio::WorkArea::select_all_ducks)
	);

	action_group->add( Gtk::Action::create("unselect-all-ducks", _("Unselect All Handles")),
		sigc::mem_fun(*work_area,&studio::WorkArea::unselect_all_ducks)
	);

	action_group->add( Gtk::Action::create("select-all-layers", _("Select All Layers")),
		sigc::mem_fun(*this,&CanvasView::on_select_layers)
	);

	action_group->add( Gtk::Action::create("unselect-all-layers", _("Unselect All Layers")),
		sigc::mem_fun(*this,&CanvasView::on_unselect_layers)
	);

	action_group->add( Gtk::Action::create("stop", Gtk::StockID("gtk-stop")),
		SLOT_EVENT(EVENT_STOP)
	);

	action_group->add( Gtk::Action::create("refresh", Gtk::StockID("gtk-refresh")),
		SLOT_EVENT(EVENT_REFRESH)
	);

	action_group->add( Gtk::Action::create("properties", Gtk::StockID("gtk-properties")),
		sigc::mem_fun0(canvas_properties,&studio::CanvasProperties::present)
	);

	list<synfigapp::PluginManager::plugin> plugin_list = studio::App::plugin_manager.get_list();
	for(list<synfigapp::PluginManager::plugin>::const_iterator p=plugin_list.begin();p!=plugin_list.end();++p) {

		synfigapp::PluginManager::plugin plugin = *p;

		action_group->add( Gtk::Action::create(plugin.id, plugin.name),
				sigc::bind(
					sigc::mem_fun(*get_instance().get(), &studio::Instance::run_plugin),
					plugin.path
				)
		);
	}

	// Preview Quality Menu
	{
		int i;
		action_group->add( Gtk::RadioAction::create(quality_group,"quality-00", _("Use Parametric Renderer")),
			sigc::bind(
				sigc::mem_fun(*work_area, &studio::WorkArea::set_quality),
				0
			)
		);
		for(i=1;i<=10;i++)
		{
			String note;
			if (i == 1) note = _(" (best)");
			if (i == 10) note = _(" (fastest)");
			Glib::RefPtr<Gtk::RadioAction> action(Gtk::RadioAction::create(quality_group,strprintf("quality-%02d",i),
																		   strprintf(_("Set Quality to %d"),i) + note));
			if (i==8)			// default quality
			{
				action->set_active();
				work_area->set_quality(i);
			}
			action_group->add( action,
				sigc::bind(
					sigc::mem_fun(*this, &studio::CanvasView::set_quality),
					i
				)
			);
		}
	}

	// Low-Res Quality Menu
	{
		int i;
		for(list<int>::iterator iter = CanvasView::get_pixel_sizes().begin(); iter != CanvasView::get_pixel_sizes().end(); iter++)
		{
			i = *iter;
			Glib::RefPtr<Gtk::RadioAction> action(Gtk::RadioAction::create(low_res_pixel_size_group,strprintf("lowres-pixel-%d",i),
																		   strprintf(_("Set Low-Res pixel size to %d"),i)));
			if(i==2)			// default pixel size
			{
				action->set_active();
				work_area->set_low_res_pixel_size(i);
			}
			action_group->add( action,
				sigc::bind(
					sigc::mem_fun(*work_area, &studio::WorkArea::set_low_res_pixel_size),
					i
				)
			);
		}

		Glib::RefPtr<Gtk::Action> action;

		action=Gtk::Action::create("decrease-low-res-pixel-size", _("Decrease Low-Res Pixel Size"));
		action_group->add( action,sigc::mem_fun(this, &studio::CanvasView::decrease_low_res_pixel_size));

		action=Gtk::Action::create("increase-low-res-pixel-size",  _("Increase Low-Res Pixel Size"));
		action_group->add( action, sigc::mem_fun(this, &studio::CanvasView::increase_low_res_pixel_size));

	}

	action_group->add( Gtk::Action::create("play", Gtk::Stock::MEDIA_PLAY),
		sigc::mem_fun(*this, &studio::CanvasView::on_play_pause_pressed)
	);

	action_group->add( Gtk::Action::create("dialog-flipbook", _("Preview Window")),
		sigc::mem_fun0(*preview_dialog, &studio::Dialog_Preview::present)
	);
	// Prevent call to preview window before preview option has created the preview window
	{
		Glib::RefPtr< Gtk::Action > action = action_group->get_action("dialog-flipbook");
		action->set_sensitive(false);
	}

	{
		Glib::RefPtr<Gtk::ToggleAction> action;

		grid_show_toggle = Gtk::ToggleAction::create("toggle-grid-show", _("Show Grid"));
		grid_show_toggle->set_active(work_area->grid_status());
		action_group->add(grid_show_toggle, sigc::mem_fun(*this, &studio::CanvasView::toggle_show_grid));

		grid_snap_toggle = Gtk::ToggleAction::create("toggle-grid-snap", _("Snap to Grid"));
		grid_snap_toggle->set_active(work_area->get_grid_snap());
		action_group->add(grid_snap_toggle, sigc::mem_fun(*this, &studio::CanvasView::toggle_snap_grid));

		action = Gtk::ToggleAction::create("toggle-guide-show", _("Show Guides"));
		action->set_active(work_area->get_show_guides());
		action_group->add(action, sigc::mem_fun(*work_area, &studio::WorkArea::toggle_show_guides));

		action = Gtk::ToggleAction::create("toggle-guide-snap", _("Snap to Guides"));
		action->set_active(work_area->get_guide_snap());
		action_group->add(action, sigc::mem_fun(*work_area, &studio::WorkArea::toggle_guide_snap));


		action = Gtk::ToggleAction::create("toggle-low-res", _("Use Low-Res"));
		action->set_active(work_area->get_low_resolution_flag());
		action_group->add(action, sigc::mem_fun(*this, &studio::CanvasView::toggle_low_res_pixel_flag));

		onion_skin_toggle = Gtk::ToggleAction::create("toggle-onion-skin", _("Show Onion Skin"));
		onion_skin_toggle->set_active(work_area->get_onion_skin());
		action_group->add(onion_skin_toggle, sigc::mem_fun(*this, &studio::CanvasView::toggle_onion_skin));
	}

	action_group->add( Gtk::Action::create("canvas-zoom-fit", Gtk::StockID("gtk-zoom-fit")),
		sigc::mem_fun(*work_area, &studio::WorkArea::zoom_fit)
	);
	action_group->add( Gtk::Action::create("canvas-zoom-100", Gtk::StockID("gtk-zoom-100")),
		sigc::mem_fun(*work_area, &studio::WorkArea::zoom_norm)
	);

	{
		Glib::RefPtr<Gtk::Action> action;

		action=Gtk::Action::create("seek-next-frame", Gtk::Stock::GO_FORWARD,_("Next Frame"),_("Next Frame"));
		action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_frame),1));
		action=Gtk::Action::create("seek-prev-frame", Gtk::Stock::GO_BACK,_("Prev Frame"),_("Prev Frame"));
		action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_frame),-1));

		action=Gtk::Action::create("seek-next-second", Gtk::Stock::GO_FORWARD,_("Seek Forward"),_("Seek Forward"));
		action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_time),Time(1)));
		action=Gtk::Action::create("seek-prev-second", Gtk::Stock::GO_BACK,_("Seek Backward"),_("Seek Backward"));
		action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_time),Time(-1)));

		action=Gtk::Action::create("seek-end", Gtk::Stock::GOTO_LAST,_("Seek to End"),_("Seek to End"));
		action_group->add(action,sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_time),Time::end()));

		action=Gtk::Action::create("seek-begin", Gtk::Stock::GOTO_FIRST,_("Seek to Begin"),_("Seek to Begin"));
		action_group->add( action, sigc::bind(sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::seek_time),Time::begin()));

		action=Gtk::Action::create("jump-next-keyframe", Gtk::Stock::GO_FORWARD,_("Jump to Next Keyframe"),_("Jump to Next Keyframe"));
		action_group->add( action,sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::jump_to_next_keyframe));

		action=Gtk::Action::create("jump-prev-keyframe", Gtk::Stock::GO_BACK,_("Jump to Prev Keyframe"),_("Jump to Prev Keyframe"));
		action_group->add( action,sigc::mem_fun(*canvas_interface().get(), &synfigapp::CanvasInterface::jump_to_prev_keyframe));

		action=Gtk::Action::create("canvas-zoom-in", Gtk::Stock::ZOOM_IN);
		action_group->add( action,sigc::mem_fun(*work_area, &studio::WorkArea::zoom_in));

		action=Gtk::Action::create("canvas-zoom-out", Gtk::Stock::ZOOM_OUT);
		action_group->add( action, sigc::mem_fun(*work_area, &studio::WorkArea::zoom_out) );

		action=Gtk::Action::create("time-zoom-in", Gtk::Stock::ZOOM_IN, _("Zoom In on Timeline"));
		action_group->add( action, sigc::mem_fun(*this, &studio::CanvasView::time_zoom_in) );

		action=Gtk::Action::create("time-zoom-out", Gtk::Stock::ZOOM_OUT, _("Zoom Out on Timeline"));
		action_group->add( action, sigc::mem_fun(*this, &studio::CanvasView::time_zoom_out) );

	}

	{
		Glib::RefPtr<Gtk::ToggleAction> action;

#define DUCK_MASK(lower,upper,string)												\
		action=Gtk::ToggleAction::create("mask-" #lower "-ducks", string);			\
		action->set_active((bool)(work_area->get_type_mask()&Duck::TYPE_##upper));	\
		action_group->add(action,													\
			sigc::bind(																\
				sigc::mem_fun(*this, &studio::CanvasView::toggle_duck_mask),		\
				Duck::TYPE_##upper))

		DUCK_MASK(position,POSITION,_("Show Position Handles"));
		DUCK_MASK(tangent,TANGENT,_("Show Tangent Handles"));
		DUCK_MASK(vertex,VERTEX,_("Show Vertex Handles"));
		DUCK_MASK(radius,RADIUS,_("Show Radius Handles"));
		DUCK_MASK(width,WIDTH,_("Show Width Handles"));
		DUCK_MASK(angle,ANGLE,_("Show Angle Handles"));
		DUCK_MASK(bone-setup,BONE_SETUP,_("Show Bone Setup Handles"));
		action_mask_bone_setup_ducks = action;
		DUCK_MASK(bone-recursive,BONE_RECURSIVE,_("Show Recursive Scale Bone Handles"));
		action_mask_bone_recursive_ducks = action;
		DUCK_MASK(widthpoint-position, WIDTHPOINT_POSITION, _("Show WidthPoints Position Handles"));

#undef DUCK_MASK

		action_group->add(Gtk::Action::create("mask-bone-ducks", _("Next Bone Handles")),
						  sigc::mem_fun(*this,&CanvasView::mask_bone_ducks));
	}
*/
}

void
MainWindow::on_switch_page(GtkNotebookPage* /* page */, guint page_num)
{
	Gtk::Notebook::PageList::iterator i = App::main_window->notebook().pages().find(page_num);
	if (i == App::main_window->notebook().pages().end())
		App::set_selected_canvas_view(NULL);
	else
		App::set_selected_canvas_view(dynamic_cast<CanvasView*>(i->get_child()));
}

/* === E N T R Y P O I N T ================================================= */
