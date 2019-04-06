/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/renddesc.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**  Copyright (c) 2010 Carlos López
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

#include <synfig/general.h>

#include "renddesc.h"
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/alignment.h>
#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <ETL/misc>
//#include <gtkmm/separator.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef SYNFIG_MAX_PIXEL_WIDTH
#define SYNFIG_MAX_PIXEL_WIDTH	(~(1<<31))
#endif

#ifndef SYNFIG_MAX_PIXEL_HEIGHT
#define SYNFIG_MAX_PIXEL_HEIGHT	(~(1<<31))
#endif

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

Widget_RendDesc::Widget_RendDesc():
	Gtk::Notebook(),
	adjustment_width(Gtk::Adjustment::create(1,1,SYNFIG_MAX_PIXEL_WIDTH)),
	adjustment_height(Gtk::Adjustment::create(1,1,SYNFIG_MAX_PIXEL_HEIGHT)),
	adjustment_xres(Gtk::Adjustment::create(0,0.0000000001,10000000)),
	adjustment_yres(Gtk::Adjustment::create(0,0.0000000001,10000000)),
	adjustment_phy_width(Gtk::Adjustment::create(0,0.0000000001,10000000)),
	adjustment_phy_height(Gtk::Adjustment::create(0,0.0000000001,10000000)),
	adjustment_fps(Gtk::Adjustment::create(0,0.0000000001,10000000)),
	adjustment_span(Gtk::Adjustment::create(0,0.0000000001,10000000))
{
	update_lock=0;

	create_widgets();
	connect_signals();

	Gtk::Label *image_tab_label = manage(new Gtk::Label(_("Image")));
	Gtk::Label *time_tab_label = manage(new Gtk::Label(_("Time")));
	Gtk::Label *other_tab_label = manage(new Gtk::Label(_("Other")));
	Gtk::Widget *imageTab = create_image_tab();
	Gtk::Widget *timeTab = create_time_tab();
	Gtk::Widget *otherTab = create_other_tab();
	append_page(*imageTab, *image_tab_label);
	append_page(*timeTab, *time_tab_label);
	append_page(*otherTab, *other_tab_label);
}

Widget_RendDesc::~Widget_RendDesc()
{
}

void Widget_RendDesc::set_rend_desc(const synfig::RendDesc &rend_desc)
{
	if(update_lock)return;

	rend_desc_=rend_desc;
	refresh();
}

void
Widget_RendDesc::refresh()
{
	UpdateLock lock(update_lock);
	//Image Tab
	adjustment_width->set_value(rend_desc_.get_w());
	adjustment_height->set_value(rend_desc_.get_h());
	adjustment_phy_width->set_value(METERS2INCHES(rend_desc_.get_physical_w()));
	adjustment_phy_height->set_value(METERS2INCHES(rend_desc_.get_physical_h()));
	adjustment_xres->set_value(DPM2DPI(rend_desc_.get_x_res()));
	adjustment_yres->set_value(DPM2DPI(rend_desc_.get_y_res()));
	toggle_wh_ratio->set_active((bool)(rend_desc_.get_flags()&RendDesc::LINK_IM_ASPECT));
	toggle_res_ratio->set_active((bool)(rend_desc_.get_flags()&RendDesc::LINK_RES));

	int w_ratio, h_ratio;
	rend_desc_.get_pixel_ratio_reduced(w_ratio, h_ratio);
	std::ostringstream px_ratio_str;
	px_ratio_str << _("Image Size Ratio : ") << w_ratio << '/' << h_ratio;
	pixel_ratio_label->set_label(px_ratio_str.str());

	entry_tl->set_value(rend_desc_.get_tl());
	entry_br->set_value(rend_desc_.get_br());
	adjustment_span->set_value(rend_desc_.get_span());

	//Time Tab
	entry_start_time->set_fps(rend_desc_.get_frame_rate());
	entry_start_time->set_value(rend_desc_.get_time_start());
	entry_end_time->set_fps(rend_desc_.get_frame_rate());
	entry_end_time->set_value(rend_desc_.get_time_end());
	entry_duration->set_fps(rend_desc_.get_frame_rate());
	entry_duration->set_value(rend_desc_.get_duration());

	adjustment_fps->set_value(rend_desc_.get_frame_rate());

	//Other Tab
	toggle_px_aspect->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_ASPECT));
	toggle_px_width->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_W));
	toggle_px_height->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_H));

	toggle_im_aspect->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_ASPECT));
	toggle_im_width->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_W));
	toggle_im_height->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_H));
	toggle_im_span->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_SPAN));
	entry_focus->set_value(rend_desc_.get_focus());
}

void Widget_RendDesc::apply_rend_desc(const synfig::RendDesc &rend_desc)
{
	set_rend_desc(rend_desc);
}

const synfig::RendDesc &
Widget_RendDesc::get_rend_desc()
{
	return rend_desc_;
}

void
Widget_RendDesc::on_width_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_w(round_to_int(adjustment_width->get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_lock_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);

#define DO_TOGGLE(x,y)	if(toggle_ ## x->get_active()) \
		rend_desc_.set_flags(rend_desc_.get_flags()|RendDesc:: y); \
	else \
		rend_desc_.set_flags(rend_desc_.get_flags()&~RendDesc:: y)

	DO_TOGGLE(px_aspect,PX_ASPECT);
	DO_TOGGLE(px_width,PX_W);
	DO_TOGGLE(px_height,PX_H);

	DO_TOGGLE(im_aspect,IM_ASPECT);
	DO_TOGGLE(im_width,IM_W);
	DO_TOGGLE(im_height,IM_H);
	DO_TOGGLE(im_span,IM_SPAN);

#undef DO_TOGGLE

	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_height_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_h(round_to_int(adjustment_height->get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_phy_width_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_physical_w(INCHES2METERS(adjustment_phy_width->get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_phy_height_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_physical_h(INCHES2METERS(adjustment_phy_height->get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_xres_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_x_res(DPI2DPM(adjustment_xres->get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_yres_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_y_res(DPI2DPM(adjustment_yres->get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_start_time_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_time_start(entry_start_time->get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_end_time_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_time_end(entry_end_time->get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_duration_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_duration(entry_duration->get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_fps_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_frame_rate(adjustment_fps->get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_tl_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_tl(entry_tl->get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_br_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_br(entry_br->get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_focus_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_focus(entry_focus->get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_span_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_span(adjustment_span->get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::disable_time_section()
{
	time_frame->set_sensitive(false);
}

void
Widget_RendDesc::enable_time_section()
{
	time_frame->set_sensitive(true);
}

void
Widget_RendDesc::on_ratio_wh_toggled()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);

	if(toggle_wh_ratio->get_active())
	{
		rend_desc_.set_pixel_ratio(adjustment_width->get_value(), adjustment_height->get_value());
		rend_desc_.set_flags(rend_desc_.get_flags()|RendDesc::LINK_IM_ASPECT);
		rend_desc_.set_flags(rend_desc_.get_flags()&~RendDesc::PX_ASPECT);
	}
	else
	{
		rend_desc_.set_flags(rend_desc_.get_flags()&~RendDesc::LINK_IM_ASPECT);
		rend_desc_.set_flags(rend_desc_.get_flags()|RendDesc::PX_ASPECT);
	}
}

void
Widget_RendDesc::on_ratio_res_toggled()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);

	if(toggle_res_ratio->get_active())
	{
		rend_desc_.set_res_ratio(adjustment_xres->get_value(), adjustment_yres->get_value());
		rend_desc_.set_flags(rend_desc_.get_flags()|RendDesc::LINK_RES);
	}
	else
	{
		rend_desc_.set_flags(rend_desc_.get_flags()&~RendDesc::LINK_RES);
	}
}

void
Widget_RendDesc::create_widgets()
{
	entry_width=manage(new Gtk::SpinButton(adjustment_width,1,0));
	entry_width->set_alignment(1);
	entry_height=manage(new Gtk::SpinButton(adjustment_height,1,0));
	entry_height->set_alignment(1);
	entry_xres=manage(new Gtk::SpinButton(adjustment_xres,0.5,1));
	entry_xres->set_alignment(1);
	entry_yres=manage(new Gtk::SpinButton(adjustment_yres,0.5,1));
	entry_yres->set_alignment(1);
	entry_phy_width=manage(new Gtk::SpinButton(adjustment_phy_width,0.25,2));
	entry_phy_width->set_alignment(1);
	entry_phy_height=manage(new Gtk::SpinButton(adjustment_phy_height,0.25,2));
	entry_phy_height->set_alignment(1);
	entry_span=manage(new Gtk::SpinButton(adjustment_span,0.1,4));
	entry_span->set_alignment(1);
	entry_tl=manage(new Widget_Vector());
	entry_tl->set_digits(4);
	entry_br=manage(new Widget_Vector());
	entry_br->set_digits(4);
	entry_fps=manage(new Gtk::SpinButton(adjustment_fps,1,5));
	entry_start_time=manage(new Widget_Time());
	entry_end_time=manage(new Widget_Time());
	entry_duration=manage(new Widget_Time());
	entry_focus=manage(new Widget_Vector());
	toggle_px_aspect=manage(new Gtk::CheckButton(_("_Pixel Aspect"), true));
	toggle_px_aspect->set_alignment(0, 0.5);
	toggle_px_width=manage(new Gtk::CheckButton(_("Pi_xel Width"), true));
	toggle_px_width->set_alignment(0, 0.5);
	toggle_px_height=manage(new Gtk::CheckButton(_("Pix_el Height"), true));
	toggle_px_height->set_alignment(0, 0.5);
	toggle_im_aspect=manage(new Gtk::CheckButton(_("Image _Aspect"), true));
	toggle_im_aspect->set_alignment(0, 0.5);
	toggle_im_width=manage(new Gtk::CheckButton(_("Image _Width"), true));
	toggle_im_width->set_alignment(0, 0.5);
	toggle_im_height=manage(new Gtk::CheckButton(_("Image _Height"), true));
	toggle_im_height->set_alignment(0, 0.5);
	toggle_im_span=manage(new Gtk::CheckButton(_("Image _Span"), true));
	toggle_im_span->set_alignment(0, 0.5);

	toggle_wh_ratio=manage(new Widget_Link(_("Link width and height"), _("Unlink width and height")));
	toggle_res_ratio=manage(new Widget_Link(_("Link x and y resolution"), _("Unlink x and y resolution")));

	pixel_ratio_label=manage(new Gtk::Label("", 0, 0.5, false));
}

void
Widget_RendDesc::connect_signals()
{
	entry_width->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_width_changed));
	entry_height->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_height_changed));
	entry_xres->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_xres_changed));
	entry_yres->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_yres_changed));
	entry_phy_width->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_phy_width_changed));
	entry_phy_height->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_phy_height_changed));
	entry_span->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_span_changed));
	entry_tl->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_tl_changed));
	entry_br->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_br_changed));
	entry_fps->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_fps_changed));
	entry_start_time->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_start_time_changed));
	entry_end_time->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_end_time_changed));
	entry_duration->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_duration_changed));
	entry_focus->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_focus_changed));
	toggle_px_aspect->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_px_width->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_px_height->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_im_aspect->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_im_width->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_im_height->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_im_span->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));

	toggle_wh_ratio->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_ratio_wh_toggled));
	toggle_res_ratio->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_ratio_res_toggled));
}

Gtk::Widget *
Widget_RendDesc::create_image_tab()
{
	Gtk::Alignment *paddedPanel = manage(new Gtk::Alignment(0, 0, 1, 1));
	paddedPanel->set_padding(12, 12, 12, 12);

	Gtk::Box *panelBox = manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 12));
	panelBox->set_homogeneous(false);
	paddedPanel->add(*panelBox);

	Gtk::Frame *imageSizeFrame = manage(new Gtk::Frame(_("Image Size")));
	imageSizeFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) imageSizeFrame->get_label_widget())->set_markup(_("<b>Image Size</b>"));
//	panelBox->pack_start(*imageSizeFrame, false, false, 0);
	panelBox->pack_start(*imageSizeFrame, Gtk::PACK_SHRINK);

	Gtk::Alignment *tableSizePadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	tableSizePadding->set_padding(6, 0, 24, 0);
	Gtk::Grid *imageSizeGrid = manage(new Gtk::Grid());

	tableSizePadding->add(*imageSizeGrid);
	imageSizeFrame->add(*tableSizePadding);

	Gtk::Label *size_width_label = manage(new Gtk::Label(_("_Width"), 0, 0.5, true));
	size_width_label->set_mnemonic_widget(*entry_width);

	Gtk::Label *size_height_label = manage(new Gtk::Label(_("_Height"), 0, 0.5, true));
	size_height_label->set_mnemonic_widget(*entry_height);

	Gtk::Label *size_xres_label = manage(new Gtk::Label(_("_XRes"), 0, 0.5, true));
	size_xres_label->set_mnemonic_widget(*entry_xres);

	Gtk::Label *size_yres_label = manage(new Gtk::Label(_("_YRes"), 0, 0.5, true));
	size_yres_label->set_mnemonic_widget(*entry_yres);

	Gtk::Label *size_physwidth_label = manage(new Gtk::Label(_("_Physical Width"), 0, 0.5, true));
	size_physwidth_label->set_mnemonic_widget(*entry_phy_width);

	Gtk::Label *size_physheight_label = manage(new Gtk::Label(_("Phy_sical Height"), 0, 0.5, true));
	size_physheight_label->set_mnemonic_widget(*entry_phy_height);

	imageSizeGrid->set_row_spacing(6);

	imageSizeGrid->attach(*size_width_label, 		0, 0, 1, 1);
	imageSizeGrid->attach(*size_height_label, 		0, 1, 1, 1);
	entry_width->set_hexpand(true);
	entry_height->set_hexpand(true);
	imageSizeGrid->attach(*entry_width, 			1, 0, 1, 1);
	imageSizeGrid->attach(*entry_height, 			1, 1, 1, 1);
	toggle_wh_ratio->set_margin_right(6);
	imageSizeGrid->attach(*toggle_wh_ratio, 		2, 0, 1, 2);

	imageSizeGrid->attach(*size_xres_label, 		3, 0, 1, 1);
	imageSizeGrid->attach(*size_yres_label, 		3, 1, 1, 1);
	entry_xres->set_hexpand(true);
	entry_yres->set_hexpand(true);
	imageSizeGrid->attach(*entry_xres, 				4, 0, 1, 1);
	imageSizeGrid->attach(*entry_yres, 				4, 1, 1, 1);
	toggle_res_ratio->set_margin_right(6);
	imageSizeGrid->attach(*toggle_res_ratio,		5, 0, 1, 2);

	imageSizeGrid->attach(*size_physwidth_label,	6, 0, 1, 1);
	imageSizeGrid->attach(*size_physheight_label,	6, 1, 1, 1);
	entry_phy_width->set_hexpand(true);
	entry_phy_height->set_hexpand(true);
	imageSizeGrid->attach(*entry_phy_width,			7, 0, 1, 1);
	imageSizeGrid->attach(*entry_phy_height,		7, 1, 1, 1);

	imageSizeGrid->attach(*pixel_ratio_label,		0, 3, 3, 1);

	Gtk::Frame *imageAreaFrame = manage(new Gtk::Frame(_("Image Area")));
	imageAreaFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) imageAreaFrame->get_label_widget())->set_markup(_("<b>Image Area</b>"));
	//panelBox->pack_start(*imageAreaFrame, false, false, 0);
	panelBox->pack_start(*imageAreaFrame, Gtk::PACK_SHRINK);

	Gtk::Alignment *imageAreaPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	imageAreaPadding->set_padding(6, 0, 24, 0);
	imageAreaFrame->add(*imageAreaPadding);

	Gtk::Box *imageAreaBox = manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,12));
	Gtk::Box *imageAreaTlbrLabelBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,6));
	Gtk::Box *imageAreaTlbrBox = manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,6));
	Gtk::Box *imageAreaSpanBox = manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,6));
	imageAreaPadding->add(*imageAreaBox);

	Gtk::Label *imageAreaTopLeftLabel = manage(new Gtk::Label(_("_Top Left"), 0, 0.5, true));
	imageAreaTopLeftLabel->set_mnemonic_widget(*entry_tl);

	Gtk::Label *imageAreaBottomRightLabel = manage(new Gtk::Label(_("_Bottom Right"), 0, 0.5, true));
	imageAreaBottomRightLabel->set_mnemonic_widget(*entry_br);

	Gtk::Label *size_span = manage(new Gtk::Label(_("I_mage Span"), 0, 0.5, true));
	size_span->set_mnemonic_widget(*entry_span);

	imageAreaTlbrLabelBox->pack_start(*imageAreaTopLeftLabel);
	imageAreaTlbrLabelBox->pack_start(*imageAreaBottomRightLabel);
	imageAreaTlbrBox->pack_start(*entry_tl);
	imageAreaTlbrBox->pack_start(*entry_br);

	imageAreaSpanBox->pack_start(*size_span);
	imageAreaSpanBox->pack_start(*entry_span);

	imageAreaBox->pack_start(*imageAreaTlbrLabelBox);
	imageAreaBox->pack_start(*imageAreaTlbrBox);
	imageAreaBox->pack_start(*imageAreaSpanBox);

	paddedPanel->show_all();
	return paddedPanel;
}

Gtk::Widget *
Widget_RendDesc::create_time_tab()
{
	Gtk::Alignment *paddedPanel = manage(new Gtk::Alignment(0, 0, 1, 1));
	paddedPanel->set_padding(12, 12, 12, 12);

	Gtk::Box *panelBox = manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 12));  // for future widgets
	panelBox->set_homogeneous(false);
	paddedPanel->add(*panelBox);

	time_frame = manage(new Gtk::Frame(_("Time Settings")));
	time_frame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) time_frame->get_label_widget())->set_markup(_("<b>Time Settings</b>"));
	panelBox->pack_start(*time_frame, Gtk::PACK_SHRINK);

	Gtk::Alignment *timeFramePadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	timeFramePadding->set_padding(6, 0, 24, 0);
	time_frame->add(*timeFramePadding);

	Gtk::Grid *timeFrameGrid = manage(new Gtk::Grid());
	timeFramePadding->add(*timeFrameGrid);
	timeFrameGrid->set_row_spacing(6);
	timeFrameGrid->set_column_spacing(250);

	Gtk::Label *timeFPSLabel = manage(new Gtk::Label(_("_Frames per second"), 0, 0.5, true));
	timeFPSLabel->set_mnemonic_widget(*entry_fps);
	timeFrameGrid->attach(*timeFPSLabel, 0, 0, 1, 1);
	entry_fps->set_hexpand(true);
	timeFrameGrid->attach(*entry_fps, 1, 0, 1, 1);

	Gtk::Label *timeStartLabel = manage(new Gtk::Label(_("_Start Time"), 0, 0.5, true));
	timeStartLabel->set_mnemonic_widget(*entry_start_time);
	timeFrameGrid->attach(*timeStartLabel, 0, 1, 1, 1);
	timeFrameGrid->attach(*entry_start_time, 1, 1, 1, 1);

	Gtk::Label *timeEndLabel = manage(new Gtk::Label(_("_End Time"), 0, 0.5, true));
	timeEndLabel->set_mnemonic_widget(*entry_end_time);
	timeFrameGrid->attach(*timeEndLabel, 0, 2, 1, 1);
	timeFrameGrid->attach(*entry_end_time, 1, 2, 1, 1);

	Gtk::Label *timeDurationLabel = manage(new Gtk::Label(_("_Duration"), 0, 0.5, true));
	timeDurationLabel->set_mnemonic_widget(*entry_duration);
	timeFrameGrid->attach(*timeDurationLabel, 0, 3, 1, 1);
	timeFrameGrid->attach(*entry_duration, 1, 3, 1, 1);

	paddedPanel->show_all();
	return paddedPanel;
}

Gtk::Widget *
Widget_RendDesc::create_other_tab()
{
	Gtk::Alignment *paddedPanel = manage(new Gtk::Alignment(0, 0, 1, 1));
	paddedPanel->set_padding(12, 12, 12, 12);

	Gtk::Box *panelBox = manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 12));
	panelBox->set_homogeneous(false);
	paddedPanel->add(*panelBox);

	Gtk::Frame *lockFrame = manage(new Gtk::Frame(_("Locks and Links")));
	lockFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) lockFrame->get_label_widget())->set_markup(_("<b>Locks and Links</b>"));
	panelBox->pack_start(*lockFrame, Gtk::PACK_SHRINK);

	Gtk::Alignment *lockPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	lockPadding->set_padding(6, 0, 24, 0);
	lockFrame->add(*lockPadding);

	Gtk::Grid *lockGrid = manage(new Gtk::Grid());
	lockGrid->set_row_spacing(6);
	lockGrid->set_column_spacing(12);
	lockPadding->add(*lockGrid);

	lockGrid->attach(*toggle_im_width,		0, 0, 1, 1);
	toggle_im_width->set_hexpand(true);
	lockGrid->attach(*toggle_im_height,		1, 0, 1, 1);
	toggle_im_height->set_hexpand(true);
	lockGrid->attach(*toggle_im_aspect,		2, 0, 1, 1);
	toggle_im_aspect->set_hexpand(true);
	lockGrid->attach(*toggle_im_span,		3, 0, 1, 1);
	toggle_im_span->set_hexpand(true);

	lockGrid->attach(*toggle_px_width,		0, 1, 1, 1);
	lockGrid->attach(*toggle_px_height,		1, 1, 1, 1);
	lockGrid->attach(*toggle_px_aspect,		2, 1, 1, 1);

	Gtk::Frame *focusFrame = manage(new Gtk::Frame(_("Focus Point")));
	focusFrame->set_shadow_type(Gtk::SHADOW_NONE);
	((Gtk::Label *) focusFrame->get_label_widget())->set_markup(_("<b>Focus Point</b>"));
	panelBox->pack_start(*focusFrame, Gtk::PACK_SHRINK);

	Gtk::Alignment *focusPadding = manage(new Gtk::Alignment(0, 0, 1, 1));
	focusPadding->set_padding(6, 0, 24, 0);
	focusFrame->add(*focusPadding);

	Gtk::Box *focusBox = manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
	focusPadding->add(*focusBox);

	Gtk::Label *focusLabel = manage(new Gtk::Label(_("_Focus Point"), 0, 0.5, true));
	focusLabel->set_mnemonic_widget(*entry_focus);
	focusBox->pack_start(*focusLabel, Gtk::PACK_SHRINK);
	focusBox->pack_start(*entry_focus, Gtk::PACK_EXPAND_WIDGET);

	paddedPanel->show_all();
	return paddedPanel;
}
