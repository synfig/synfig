/* === S Y N F I G ========================================================= */
/*!	\file dock_sound.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cassert>

#include <gtkmm/scrolledwindow.h>

#include <synfig/general.h>

#include <gui/localization.h>
#include <app.h>
#include <instance.h>
#include <canvasview.h>
#include <workarea.h>

#include <trees/layerparamtreestore.h>
#include <widgets/widget_sound.h>

#include "dock_sound.h"
#include <cstring>

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
Dock_Sound::Dock_Sound():
	Dock_CanvasSpecific("sounds",_("Waveform"),Gtk::StockID("synfig-sound")),
	action_group(Gtk::ActionGroup::create("action_group_dock_sound"))
	table_(),
	last_widget_sound_()
{ 	

	action_group->add(Gtk::Action::create(
		"action-SoundAdd",
		Gtk::StockID("gtk-load"),
		_("Load new Sound File"),
		_("Load a new Sound File")
	),
		sigc::mem_fun(
			*this,
			&Dock_MetaData::on_load_pressed
		)
	);
}
Dock_Sound::~Dock_Sound()
{ 	if (table_) delete table_;
}

Dock_Sound::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	Widget_Sound* sound(new Widget_Sound());
	sound->setup_canvas();

	/*Gtk::TreeView* param_tree_view(
		static_cast<Gtk::TreeView*>(canvas_view->get_ext_widget("params"))
	);

	param_tree_view->get_selection()->signal_changed().connect(
		sigc::bind(
			sigc::bind(
				sigc::ptr_fun(
					_sound_changed
				),curves
			),param_tree_view
		)
	);*/  			       // couldn't understand this part yet

	studio::LayerTree* tree_layer(dynamic_cast<studio::LayerTree*>(canvas_view->get_ext_widget("layers_cmp")));
	tree_layer->signal_param_tree_header_height_changed().connect(sigc::mem_fun(*this, &studio::Dock_Sound::on_update_header_height));

	canvas_view->set_ext_widget(get_name(),sound);
}

Dock_Sound::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	if(table_)
	{
		delete table_;
		table_ = 0;

		last_widget_sound_ = 0;

		hscrollbar_.unset_adjustment();
		vscrollbar_.unset_adjustment();

		widget_timeslider_.set_canvas_view( CanvasView::Handle() );

		widget_kf_list_.set_time_model( etl::handle<TimeModel>() );
		widget_kf_list_.set_canvas_interface( etl::loose_handle<synfigapp::CanvasInterface>() );
	}


	if(canvas_view)
	{
		last_widget_sound_=dynamic_cast<Widget_Sound*>( canvas_view->get_ext_widget(get_name()) );

		vscrollbar_.set_adjustment(last_widget_sound_->get_range_adjustment());
		hscrollbar_.set_adjustment(canvas_view->time_model()->scroll_time_adjustment());

		widget_timeslider_.set_canvas_view(canvas_view);

		widget_kf_list_.set_time_model(canvas_view->time_model());
		widget_kf_list_.set_canvas_interface(canvas_view->canvas_interface());

		table_=new Gtk::Table(3, 2);
		table_->attach(widget_kf_list_,      0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(widget_timeslider_,   0, 1, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(*last_widget_sound_,  0, 1, 2, 3, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND);
		table_->attach(hscrollbar_,          0, 1, 3, 4, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::SHRINK);
		table_->attach(vscrollbar_,          1, 2, 0, 3, Gtk::FILL|Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND);
		add(*table_);

		last_widget_sound_->show();
		table_->show_all();
		show_all();
	}
	else
	{
		//clear_previous();
	}
}

void
Dock_Sound::on_load_pressed()
{

			
	String filename("*.*");
	bool selected;   
	if(App::dialog_open_file_audio(_("Please choose an audio file"), filename, ANIMATION_DIR_PREFERENCE))
		{ selected=true;
		}
	else 	{ selected=false;
		}	


}
void
Dock_Sound::on_update_header_height( int header_height)
{
#ifdef _WIN32
	header_height-=2;
#elif defined(__APPLE__)
	header_height+=6;
#else
// *nux and others
	header_height+=2;
#endif
	int kf_list_height=10;

	//widget_timeslider_.set_size_request(-1, header_height+1);
	widget_timeslider_.set_size_request(-1, header_height - kf_list_height + 5);
	widget_kf_list_.set_size_request(-1, kf_list_height);
}
