/* === S Y N F I G ========================================================= */
/*!	\file renddesc.cpp
**	\brief Template File
**
**	$Id: renddesc.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "renddesc.h"
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <ETL/misc>
#include <synfig/general.h>
//#include <gtkmm/seperator.h>

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

#if ! defined(_)
#define _(x)	(x)
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
	Gtk::Table(1,2,false),
	adjustment_width(1,1,SYNFIG_MAX_PIXEL_WIDTH),
	adjustment_height(1,1,SYNFIG_MAX_PIXEL_HEIGHT),
	adjustment_xres(0,0,10000000),
	adjustment_yres(0,0,10000000),
	adjustment_phy_width(0,0,10000000),
	adjustment_phy_height(0,0,10000000),
	adjustment_fps(0,0,10000000),
	adjustment_span(0,0,10000000)
{
	update_lock=0;
	
	Gtk::Frame *size_frame=manage(new Gtk::Frame(_("Image Size")));
	Gtk::Frame *area_frame=manage(new Gtk::Frame(_("Image Area")));
	time_frame=manage(new Gtk::Frame(_("Time")));
	
	Gtk::Table *size_table=manage(new Gtk::Table(2,2,false));
	size_frame->add(*size_table);

	Gtk::Table *area_table=manage(new Gtk::Table(2,2,false));
	area_frame->add(*area_table);

	time_table=manage(new Gtk::Table(2,2,false));
	time_frame->add(*time_table);

	Gtk::Frame *other_frame=manage(new Gtk::Frame(_("Other")));
	Gtk::Table *other_table=manage(new Gtk::Table(2,2,false));
	other_frame->add(*other_table);

	Gtk::Frame *lock_frame=manage(new Gtk::Frame(_("Locks and Links")));
	Gtk::Table *lock_table=manage(new Gtk::Table(2,2,false));
	lock_frame->add(*lock_table);
	
	entry_width=manage(new Gtk::SpinButton(adjustment_width,1,0));
	entry_height=manage(new Gtk::SpinButton(adjustment_height,1,0));	
	entry_width->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_width_changed));
	entry_height->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_height_changed));
	size_table->attach(*manage(new Gtk::Label(_("Width"))), 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*manage(new Gtk::Label(_("Height"))), 0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*entry_width, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*entry_height, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	
	entry_xres=manage(new Gtk::SpinButton(adjustment_xres,0.5,1));
	entry_yres=manage(new Gtk::SpinButton(adjustment_yres,0.5,1));	
	entry_xres->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_xres_changed));
	entry_yres->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_yres_changed));
	size_table->attach(*manage(new Gtk::Label(_("XRes"))), 2, 3, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*manage(new Gtk::Label(_("YRes"))), 2, 3, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*entry_xres, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*entry_yres, 3, 4, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	

	entry_phy_width=manage(new Gtk::SpinButton(adjustment_phy_width,0.25,2));
	entry_phy_height=manage(new Gtk::SpinButton(adjustment_phy_height,0.25,2));	
	entry_phy_width->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_phy_width_changed));
	entry_phy_height->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_phy_height_changed));
	size_table->attach(*manage(new Gtk::Label(_("PhyWidth"))), 4, 5, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*manage(new Gtk::Label(_("PhyHeight"))), 4, 5, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*entry_phy_width, 5, 6, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*entry_phy_height, 5, 6, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	


	entry_span=manage(new Gtk::SpinButton(adjustment_span,0.1,4));	
	entry_span->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_span_changed));
	size_table->attach(*manage(new Gtk::Label(_("Span"))), 0, 1, 2, 3, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	size_table->attach(*entry_span, 1, 2, 2, 3, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	

	entry_tl=manage(new Widget_Vector());
	entry_br=manage(new Widget_Vector());
	entry_tl->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_tl_changed));
	entry_br->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_br_changed));
	area_table->attach(*manage(new Gtk::Label(_("Top-Left"))), 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	area_table->attach(*manage(new Gtk::Label(_("Bottom-Right"))), 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	area_table->attach(*entry_tl, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	area_table->attach(*entry_br, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	

	entry_fps=manage(new Gtk::SpinButton(adjustment_fps,1,5));
	entry_start_time=manage(new Widget_Time());
	entry_end_time=manage(new Widget_Time());	
	//entry_start_frame=manage(new Gtk::SpinButton(adjustment_start_frame,1,0));
	//entry_end_frame=manage(new Gtk::SpinButton(adjustment_end_frame,1,0));	
	entry_fps->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_fps_changed));
	entry_start_time->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_start_time_changed));
	entry_end_time->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_end_time_changed));
	//entry_start_frame->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_start_frame_changed));
	//entry_end_frame->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_end_frame_changed));
	time_table->attach(*manage(new Gtk::Label(_("FPS"))), 0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	time_table->attach(*manage(new Gtk::Label(_("Start Time"))), 2, 3, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	time_table->attach(*manage(new Gtk::Label(_("End Time"))), 2, 3, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	//time_table->attach(*manage(new Gtk::Label(_("Start Frame"))), 4, 5, 0, 1, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	//time_table->attach(*manage(new Gtk::Label(_("End Frame"))), 4, 5, 1, 2, Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	time_table->attach(*entry_fps, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	time_table->attach(*entry_start_time, 3, 4, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	time_table->attach(*entry_end_time, 3, 4, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	//time_table->attach(*entry_start_frame, 5, 6, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	//time_table->attach(*entry_end_frame, 5, 6, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	

	entry_focus=manage(new Widget_Vector());
	entry_focus->signal_value_changed().connect(sigc::mem_fun(*this,&studio::Widget_RendDesc::on_focus_changed));
	other_table->attach(*manage(new Gtk::Label(_("Focus Point"))), 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	other_table->attach(*entry_focus, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	



	toggle_px_aspect=manage(new Gtk::CheckButton(_("Pixel Aspect")));
	toggle_px_width=manage(new Gtk::CheckButton(_("Pixel Width")));
	toggle_px_height=manage(new Gtk::CheckButton(_("Pixel Height")));

	toggle_im_aspect=manage(new Gtk::CheckButton(_("Image Aspect")));
	toggle_im_width=manage(new Gtk::CheckButton(_("Image Width")));
	toggle_im_height=manage(new Gtk::CheckButton(_("Image Height")));
	toggle_im_span=manage(new Gtk::CheckButton(_("Image Span")));

	toggle_px_aspect->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_px_width->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_px_height->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_im_aspect->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_im_width->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_im_height->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));
	toggle_im_span->signal_toggled().connect(sigc::mem_fun(*this, &studio::Widget_RendDesc::on_lock_changed));

	lock_table->attach(*toggle_px_aspect, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	lock_table->attach(*toggle_px_width, 0, 1, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	lock_table->attach(*toggle_px_height, 0, 1, 2, 3, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	

	lock_table->attach(*toggle_im_aspect, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	lock_table->attach(*toggle_im_width, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	lock_table->attach(*toggle_im_height, 1, 2, 2, 3, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	
	lock_table->attach(*toggle_im_span, 1, 2, 3, 4, Gtk::SHRINK, Gtk::SHRINK|Gtk::FILL, 0, 0);	


	lock_frame->show_all();
	other_frame->show_all();
	size_frame->show_all();
	area_frame->show_all();
	time_frame->show_all();
	attach(*size_frame, 0, 1, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	attach(*area_frame, 0, 1, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	attach(*lock_frame, 0, 1, 2,3, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	attach(*time_frame, 0, 1, 3, 4, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
	attach(*other_frame, 0, 1, 4, 5, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);	
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
	adjustment_width.set_value(rend_desc_.get_w());
	adjustment_height.set_value(rend_desc_.get_h());
	adjustment_phy_width.set_value(METERS2INCHES(rend_desc_.get_physical_w()));
	adjustment_phy_height.set_value(METERS2INCHES(rend_desc_.get_physical_h()));
	adjustment_xres.set_value(DPM2DPI(rend_desc_.get_x_res()));
	adjustment_yres.set_value(DPM2DPI(rend_desc_.get_y_res()));
	entry_start_time->set_fps(rend_desc_.get_frame_rate());
	entry_start_time->set_value(rend_desc_.get_time_start());
	entry_end_time->set_fps(rend_desc_.get_frame_rate());
	entry_end_time->set_value(rend_desc_.get_time_end());

	adjustment_fps.set_value(rend_desc_.get_frame_rate());
	adjustment_span.set_value(rend_desc_.get_span());
	entry_tl->set_value(rend_desc_.get_tl());
	entry_br->set_value(rend_desc_.get_br());
	entry_focus->set_value(rend_desc_.get_focus());

	toggle_px_aspect->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_ASPECT));
	toggle_px_width->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_W));
	toggle_px_height->set_active((bool)(rend_desc_.get_flags()&RendDesc::PX_H));

	toggle_im_aspect->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_ASPECT));
	toggle_im_width->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_W));
	toggle_im_height->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_H));
	toggle_im_span->set_active((bool)(rend_desc_.get_flags()&RendDesc::IM_SPAN));	
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
	rend_desc_.set_w(round_to_int(adjustment_width.get_value()));
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
	rend_desc_.set_h(round_to_int(adjustment_height.get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_phy_width_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_physical_w(INCHES2METERS(adjustment_phy_width.get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_phy_height_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_physical_h(INCHES2METERS(adjustment_phy_height.get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_xres_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_x_res(DPI2DPM(adjustment_xres.get_value()));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_yres_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_y_res(DPI2DPM(adjustment_yres.get_value()));
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

/*
void
Widget_RendDesc::on_start_frame_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_frame_start((int)(adjustment_start_frame.get_value()+0.5));
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::on_end_frame_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_frame_end((int)(adjustment_end_frame.get_value()+0.5));
	refresh();
	signal_changed()();
}
*/

void
Widget_RendDesc::on_fps_changed()
{
	if(update_lock)return;
	UpdateLock lock(update_lock);
	rend_desc_.set_frame_rate((int)(adjustment_fps.get_value()+0.5));
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
	rend_desc_.set_span(adjustment_span.get_value());
	refresh();
	signal_changed()();
}

void
Widget_RendDesc::disable_time_section()
{
	time_frame->set_sensitive(false);
	
/*
	Gtk::Table::TableList &list=time_table->children();
	Gtk::Table::TableList::iterator iter;
	for(iter=list.begin();iter!=list.end();iter++)
		iter->get_widget()->set_sensitive(false);
*/	
}
	
void
Widget_RendDesc::enable_time_section()
{
	time_frame->set_sensitive(true);

/*
	Gtk::Table::TableList &list=time_table->children();
	Gtk::Table::TableList::iterator iter;
	for(iter=list.begin();iter!=list.end();iter++)
		iter->get_widget()->set_sensitive(true);

*/
}
