/* === S Y N F I G ========================================================= */
/*!	\file dialogs/canvasproperties.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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


//#include <gtkmm/scrolledwindow.h>
//#include <gtkmm/table.h>
//#include <gtkmm/treeview.h>

#include <glibmm/fileutils.h>
#include <glibmm/markup.h>

#include "app.h"
#include "canvasproperties.h"
#include "gui/resourcehelper.h"
//#include "trees/metadatatreestore.h"

#include <gui/localization.h>
#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef DPM2DPI
#define DPM2DPI(x)	((x)/39.3700787402)
#define DPI2DPM(x)	((x)*39.3700787402)
#endif

#ifndef METERS2INCHES
#define METERS2INCHES(x)	((x)*39.3700787402)
#define INCHES2METERS(x)	((x)/39.3700787402)
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CanvasProperties::CanvasProperties(Gtk::Window& parent, etl::handle<synfigapp::CanvasInterface> canvas_interface) :
	Gtk::Dialog      (_("Canvas Properties"), parent),
	canvas_interface_(canvas_interface),
	rend_desc_       (canvas_interface_->get_canvas()->rend_desc()),
	outer_box        (get_content_area()),
	inner_box        (nullptr),
	// Canvas info
	entry_id         (nullptr),
	entry_name       (nullptr),
	entry_desc       (nullptr),
	// Image
	ratio_label      (nullptr),
	entry_width      (nullptr),
	entry_height     (nullptr),
	entry_xres       (nullptr),
	entry_yres       (nullptr),
	entry_phy_w      (nullptr),
	entry_phy_h      (nullptr),
	entry_span       (nullptr),
	toggle_wh_r      (nullptr),
	toggle_res_r     (nullptr),
	entry_tl         (nullptr),
	entry_br         (nullptr),
	// Time
	entry_fps        (nullptr),
	entry_start      (nullptr),
	entry_end        (nullptr),
	entry_dur        (nullptr),
	// Gamma Correction
	entry_gamma_r    (nullptr),
	entry_gamma_g    (nullptr),
	entry_gamma_b    (nullptr),
	scale_gamma_r    (nullptr),
	scale_gamma_g    (nullptr),
	scale_gamma_b    (nullptr),
	gamma_pattern    (nullptr),
	// Other
	toggle_pxl_w     (nullptr),
	toggle_pxl_h     (nullptr),
	toggle_pxl_a     (nullptr),
	toggle_img_w     (nullptr),
	toggle_img_h     (nullptr),
	toggle_img_a     (nullptr),
	toggle_img_s     (nullptr),
	entry_focus      (nullptr),
	dirty_rend_desc  (false),
	update_lock      (0)
{
	try {
		canvas_properties = Gtk::Builder::create_from_file(
					ResourceHelper::get_ui_path("canvas_properties.glade"));
	} catch (const Glib::FileError &exception) {
		error("Glade File Error: " + exception.what());
	} catch (const Glib::MarkupError &exception) {
		error("Glade Markup Error: " + exception.what());
	} catch (const Gtk::BuilderError &exception) {
		error("Glade Builder Error: " + exception.what());
	}

	canvas_properties->get_widget("inner_box", inner_box);

	outer_box->add(*inner_box);

	set_up_info_widgets();
	set_up_image_widgets();
	set_up_time_widgets();
	set_up_gamma_widgets();
	set_up_other_widgets();
	set_up_action_widgets();

	connect_signals();

	refresh();
}

CanvasProperties::~CanvasProperties()
{
}

void
CanvasProperties::set_rend_desc(const RendDesc &rend_desc)
{
	if (update_lock) return;
	rend_desc_ = rend_desc;
	refresh();
}

const RendDesc
&CanvasProperties::get_rend_desc()
{
	return rend_desc_;
}

//Gtk::Widget&
//CanvasProperties::create_meta_data_view()
//{
	//MetaDataTreeStore::Model model;
	//meta_data_tree_view=(manage(new class Gtk::TreeView()));

	//meta_data_tree_view->append_column(_("Key"),model.key);
	//meta_data_tree_view->append_column_editable(_("Data"),model.data);
	//meta_data_tree_view->set_model(MetaDataTreeStore::create(canvas_interface_));
	//meta_data_tree_view->set_rules_hint();
	//meta_data_tree_view->show();

	//Gtk::ScrolledWindow *scrolledwindow = manage(new class Gtk::ScrolledWindow());
	//scrolledwindow->set_can_focus(true);
	//scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	//scrolledwindow->add(*meta_data_tree_view);
	//scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	//scrolledwindow->show();



	//Gtk::Table *table=manage(new Gtk::Table());
	//table->attach(*scrolledwindow, 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	//Gtk::Button* button_add(manage(new Gtk::Button(Gtk::StockID("gtk-add"))));
	//button_add->show();
	//button_add->signal_clicked().connect(sigc::mem_fun(*this,&CanvasProperties::on_button_meta_data_add));
	//table->attach(*button_add, 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);

	//Gtk::Button* button_delete(manage(new Gtk::Button(Gtk::StockID("gtk-delete"))));
	//button_delete->show();
	//button_delete->signal_clicked().connect(sigc::mem_fun(*this,&CanvasProperties::on_button_meta_data_delete));
	//table->attach(*button_delete, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);

	//table->show();
	//return *table;
//}

//void
//CanvasProperties::on_button_meta_data_add()
//{
	//synfig::String key;
	//if(App::dialog_entry(_("New MetaData Entry"), _("Please enter the name of the key"),key) && !key.empty())
	//{
		//canvas_interface_->set_meta_data(key," ");
	//}

//}

//void
//CanvasProperties::on_button_meta_data_delete()
//{
//}

void
CanvasProperties::on_action_signal_response(int response_id)
{
	if (response_id == 1 || response_id == Gtk::RESPONSE_DELETE_EVENT) {
		// Either Cancel button was pressed or Dialog was closed
		refresh_dialog();
		hide();
	} else {
		// Either Apply or OK button was pressed
		synfigapp::Action::PassiveGrouper group(
				canvas_interface_->get_instance().get(),
				_("Edit Canvas Properties"));

		// fetch these three values first, because each set_() method refreshes
		// the dialog with currently set values
		String id   = entry_id->get_text();
		String name = entry_name->get_text();
		String desc = entry_desc->get_text();

		// do this first, because the other three cause the dialog to be
		// refreshed with currently set values
		if (dirty_rend_desc)
			canvas_interface_->set_rend_desc(get_rend_desc());

		if (id != canvas_interface_->get_canvas()->get_id() && !id.empty())
			canvas_interface_->set_id(id);
		if (name != canvas_interface_->get_canvas()->get_name())
			canvas_interface_->set_name(name);
		if (desc != canvas_interface_->get_canvas()->get_description())
			canvas_interface_->set_description(desc);

		dirty_rend_desc = false;

		if (response_id == 2) {
			// OK was pressed, hide the dialog
			hide();
		}
	}
}

void
CanvasProperties::on_rend_desc_changed()
{
	set_rend_desc(canvas_interface_->get_canvas()->rend_desc());
}

void
CanvasProperties::set_up_info_widgets()
{
	Gtk::Label *id_label;

	canvas_properties->get_widget("id_label"  , id_label);
	canvas_properties->get_widget("entry_id"  , entry_id);
	canvas_properties->get_widget("entry_name", entry_name);
	canvas_properties->get_widget("entry_desc", entry_desc);

	// The root canvas doesn't have an ID, so don't
	// display it if this is a root canvas.
	if(!canvas_interface_->get_canvas()->is_root()) {
		id_label->show();
		entry_id->show();
	}
}

void
CanvasProperties::set_up_image_widgets()
{
	canvas_properties->get_widget("ratio_label" , ratio_label);
	canvas_properties->get_widget("entry_width" , entry_width);
	canvas_properties->get_widget("entry_height", entry_height);
	canvas_properties->get_widget("entry_xres"  , entry_xres);
	canvas_properties->get_widget("entry_yres"  , entry_yres);
	canvas_properties->get_widget("entry_phy_w" , entry_phy_w);
	canvas_properties->get_widget("entry_phy_h" , entry_phy_h);
	canvas_properties->get_widget("entry_span"  , entry_span);
	canvas_properties->get_widget("entry_tl"    , entry_tl);
	canvas_properties->get_widget("entry_br"    , entry_br);
	canvas_properties->get_widget("toggle_wh_r" , toggle_wh_r);
	canvas_properties->get_widget("toggle_res_r", toggle_res_r);

	entry_tl->set_digits(4);
	entry_br->set_digits(4);
}

void
CanvasProperties::set_up_time_widgets()
{
	canvas_properties->get_widget("entry_fps"  , entry_fps);
	canvas_properties->get_widget("entry_start", entry_start);
	canvas_properties->get_widget("entry_end"  , entry_end);
	canvas_properties->get_widget("entry_dur"  , entry_dur);
}

void
CanvasProperties::set_up_gamma_widgets()
{
	canvas_properties->get_widget("gamma_pattern", gamma_pattern);
	canvas_properties->get_widget("entry_gamma_r", entry_gamma_r);
	canvas_properties->get_widget("entry_gamma_g", entry_gamma_g);
	canvas_properties->get_widget("entry_gamma_b", entry_gamma_b);
	canvas_properties->get_widget("scale_gamma_r", scale_gamma_r);
	canvas_properties->get_widget("scale_gamma_g", scale_gamma_g);
	canvas_properties->get_widget("scale_gamma_b", scale_gamma_b);

	scale_gamma_r->set_round_digits(1);
	scale_gamma_g->set_round_digits(1);
	scale_gamma_b->set_round_digits(1);
}

void
CanvasProperties::set_up_other_widgets()
{
	canvas_properties->get_widget("toggle_img_w", toggle_img_w);
	canvas_properties->get_widget("toggle_img_h", toggle_img_h);
	canvas_properties->get_widget("toggle_img_a", toggle_img_a);
	canvas_properties->get_widget("toggle_img_s", toggle_img_s);
	canvas_properties->get_widget("toggle_pxl_w", toggle_pxl_w);
	canvas_properties->get_widget("toggle_pxl_h", toggle_pxl_h);
	canvas_properties->get_widget("toggle_pxl_a", toggle_pxl_a);
	canvas_properties->get_widget("entry_focus" , entry_focus);
}

void
CanvasProperties::set_up_action_widgets()
{
	this->add_button("_Apply" , 0)->set_use_underline();
	this->add_button("_Cancel", 1)->set_use_underline();
	this->add_button("_OK"    , 2)->set_use_underline();

	set_default_response(2);
}

void
CanvasProperties::connect_signals()
{
	this->signal_response().connect(sigc::mem_fun(*this, &CanvasProperties::on_action_signal_response));

	canvas_interface_->signal_rend_desc_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_rend_desc_changed));
	canvas_interface_->signal_id_changed()       .connect(sigc::mem_fun(*this, &CanvasProperties::refresh));

	// Image
	entry_width ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_width_changed));
	entry_height->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_height_changed));
	entry_xres  ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_xres_changed));
	entry_yres  ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_yres_changed));
	entry_phy_w ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_phy_w_changed));
	entry_phy_h ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_phy_h_changed));
	entry_span  ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_span_changed));
	toggle_wh_r ->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_ratio_wh_toggled));
	toggle_res_r->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_ratio_res_toggled));
	entry_tl    ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_tl_changed));
	entry_br    ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_br_changed));

	// Time
	entry_fps  ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_fps_changed));
	entry_start->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_start_time_changed));
	entry_end  ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_end_time_changed));
	entry_dur  ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_duration_changed));

	// Gamma Correction
	entry_gamma_r->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_gamma_changed));
	entry_gamma_g->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_gamma_changed));
	entry_gamma_b->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_gamma_changed));

	// Other
	toggle_img_w->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_lock_changed));
	toggle_img_h->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_lock_changed));
	toggle_img_a->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_lock_changed));
	toggle_img_s->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_lock_changed));
	toggle_pxl_w->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_lock_changed));
	toggle_pxl_h->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_lock_changed));
	toggle_pxl_a->signal_toggled()      .connect(sigc::mem_fun(*this, &CanvasProperties::on_lock_changed));
	entry_focus ->signal_value_changed().connect(sigc::mem_fun(*this, &CanvasProperties::on_focus_changed));
}

void
CanvasProperties::refresh()
{
	UpdateLock lock(update_lock);

	refresh_dialog();
}

void
CanvasProperties::refresh_dialog()
{
	set_rend_desc(canvas_interface_->get_canvas()->rend_desc());

	set_title(_("Properties")+String(" - ")+canvas_interface_->get_canvas()->get_name());

	// Canvas Info
	entry_id  ->set_text(canvas_interface_->get_canvas()->get_id());
	entry_name->set_text(canvas_interface_->get_canvas()->get_name());
	entry_desc->set_text(canvas_interface_->get_canvas()->get_description());


	// Image
	entry_width ->set_value(rend_desc_.get_w());
	entry_height->set_value(rend_desc_.get_h());
	entry_phy_w ->set_value(METERS2INCHES(rend_desc_.get_physical_w()));
	entry_phy_h ->set_value(METERS2INCHES(rend_desc_.get_physical_h()));
	entry_xres  ->set_value(DPM2DPI(rend_desc_.get_x_res()));
	entry_yres  ->set_value(DPM2DPI(rend_desc_.get_y_res()));
	toggle_wh_r ->set_active((bool)(rend_desc_.get_flags()&RendDesc::LINK_IM_ASPECT));
	toggle_res_r->set_active((bool)(rend_desc_.get_flags()&RendDesc::LINK_RES));

	int w_ratio, h_ratio;
	rend_desc_.get_pixel_ratio_reduced(w_ratio, h_ratio);
	ostringstream pxl_ratio_str;
	pxl_ratio_str << _("Image Size Ratio: ") << w_ratio << '/' << h_ratio;
	ratio_label->set_label(pxl_ratio_str.str());

	entry_tl  ->set_value(rend_desc_.get_tl());
	entry_br  ->set_value(rend_desc_.get_br());
	entry_span->set_value(rend_desc_.get_span());


	// Time
	entry_fps  ->set_value(rend_desc_.get_frame_rate());
	entry_start->set_fps  (rend_desc_.get_frame_rate());
	entry_start->set_value(rend_desc_.get_time_start());
	entry_end  ->set_fps  (rend_desc_.get_frame_rate());
	entry_end  ->set_value(rend_desc_.get_time_end());
	entry_dur  ->set_fps  (rend_desc_.get_frame_rate());
	entry_dur  ->set_value(rend_desc_.get_duration());


	// Gamma Correction
	entry_gamma_r->set_value(rend_desc_.get_gamma().get_r());
	entry_gamma_g->set_value(rend_desc_.get_gamma().get_g());
	entry_gamma_b->set_value(rend_desc_.get_gamma().get_b());
	gamma_pattern->set_gamma(rend_desc_.get_gamma().get_inverted());


	// Other
	toggle_pxl_a->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_ASPECT));
	toggle_pxl_w->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_W));
	toggle_pxl_h->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_H));

	toggle_img_a->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_ASPECT));
	toggle_img_w->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_W));
	toggle_img_h->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_H));
	toggle_img_s->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_SPAN));
	entry_focus ->set_value(rend_desc_.get_focus());
}

void
CanvasProperties::on_width_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_w(round_to_int(entry_width->get_value()));
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_height_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_h(round_to_int(entry_height->get_value()));
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_xres_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_x_res(DPI2DPM(entry_xres->get_value()));
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_yres_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_y_res(DPI2DPM(entry_yres->get_value()));
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_phy_w_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_physical_w(INCHES2METERS(
			entry_phy_w->get_value()));
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_phy_h_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_physical_h(INCHES2METERS(
			entry_phy_h->get_value()));
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_span_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_span(entry_span->get_value());
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_ratio_wh_toggled()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);

	if (toggle_wh_r->get_active()) {
		rend_desc_.set_pixel_ratio(
				entry_width->get_value(),
				entry_height->get_value());
		rend_desc_.set_flags(rend_desc_.get_flags() | RendDesc::LINK_IM_ASPECT);
		rend_desc_.set_flags(rend_desc_.get_flags() & ~RendDesc::PX_ASPECT);
	} else {
		rend_desc_.set_flags(rend_desc_.get_flags() & ~RendDesc::LINK_IM_ASPECT);
		rend_desc_.set_flags(rend_desc_.get_flags() | RendDesc::PX_ASPECT);
	}

	dirty_rend_desc = true;
}

void
CanvasProperties::on_ratio_res_toggled()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);

	if (toggle_res_r->get_active()) {
		rend_desc_.set_res_ratio(
				entry_xres->get_value(),
				entry_yres->get_value());
		rend_desc_.set_flags(rend_desc_.get_flags() | RendDesc::LINK_RES);
	} else {
		rend_desc_.set_flags(rend_desc_.get_flags() & ~RendDesc::LINK_RES);
	}

	dirty_rend_desc = true;
}

void
CanvasProperties::on_tl_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_tl(entry_tl->get_value());
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_br_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_br(entry_br->get_value());
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_fps_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_frame_rate(entry_fps->get_value());
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_start_time_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_time_start(entry_start->get_value());
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_end_time_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_time_end(entry_end->get_value());
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_duration_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_duration(entry_dur->get_value());
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_focus_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_focus(entry_focus->get_value());
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_gamma_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);
	rend_desc_.set_gamma(Gamma(
		entry_gamma_r->get_value(),
		entry_gamma_g->get_value(),
		entry_gamma_b->get_value() ));
	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}

void
CanvasProperties::on_lock_changed()
{
	if (update_lock) return;
	UpdateLock lock(update_lock);

#define DO_TOGGLE(x,y)	if(toggle_ ## x->get_active()) \
		rend_desc_.set_flags(rend_desc_.get_flags()|RendDesc:: y); \
	else \
		rend_desc_.set_flags(rend_desc_.get_flags()&~RendDesc:: y)

	DO_TOGGLE(pxl_a, PX_ASPECT);
	DO_TOGGLE(pxl_w, PX_W);
	DO_TOGGLE(pxl_h, PX_H);

	DO_TOGGLE(img_a, IM_ASPECT);
	DO_TOGGLE(img_w, IM_W);
	DO_TOGGLE(img_h, IM_H);
	DO_TOGGLE(img_s, IM_SPAN);

#undef DO_TOGGLE

	refresh();
	dirty_rend_desc = true;
	signal_changed()();
}
