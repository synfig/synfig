/* === S I N F G =========================================================== */
/*!	\file cellrenderer_time.cpp
**	\brief Template File
**
**	$Id: cellrenderer_time.cpp,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
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

#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include "cellrenderer_time.h"
#include "app.h"
#include "widget_time.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

#if ! defined(_)
#define _(x)	(x)
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CellRenderer_Time::CellRenderer_Time():
	Glib::ObjectBase	(typeid(CellRenderer_Time)),
	Gtk::CellRendererText	(),
	property_time_(*this,"time",sinfg::Time(0)),
	property_fps_(*this,"fps", float(0))
{
	CellRendererText::signal_edited().connect(sigc::mem_fun(*this,&studio::CellRenderer_Time::string_edited_));
}

CellRenderer_Time::~CellRenderer_Time()
{
	sinfg::info("CellRenderer_Time::~CellRenderer_Time(): deleted");
}

void
CellRenderer_Time::string_edited_(const Glib::ustring&path,const Glib::ustring&str)
{
	signal_edited_(path,Time((String)str,(Real)Time(property_fps_)));
}

void
CellRenderer_Time::render_vfunc(
		const Glib::RefPtr<Gdk::Drawable>& window,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& ca,
		const Gdk::Rectangle& expose_area,
		Gtk::CellRendererState flags)
{
	if(!window)
		return;
	//int	height = ca.get_height();

	Gtk::StateType state = Gtk::STATE_INSENSITIVE;
	if(property_editable())
		state = Gtk::STATE_NORMAL;
	if((flags & Gtk::CELL_RENDERER_SELECTED) != 0)
		state = (widget.has_focus()) ? Gtk::STATE_SELECTED : Gtk::STATE_ACTIVE;

	const Time time(property_time_);
	const float fps((Real)Time(property_fps_));
	
	property_text()=(Glib::ustring)time.get_string(fps,App::get_time_format());

	CellRendererText::render_vfunc(window,widget,background_area,ca,expose_area,flags);
}


Gtk::CellEditable*
CellRenderer_Time::start_editing_vfunc(
	GdkEvent* event,
	Gtk::Widget& widget,
	const Glib::ustring& path,
	const Gdk::Rectangle& background_area,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState flags)
{
	// If we aren't editable, then there is nothing to do
	if(!property_editable())
		return 0;

	const Time time(property_time_);
	const float fps((Real)Time(property_fps_));
	
	property_text()=(Glib::ustring)time.get_string(fps,App::get_time_format()|Time::FORMAT_FULL);
#if 0
	Widget_Time* widget_time(manage(new Widget_Time));
	widget_time->set_fps(fps);
	widget_time->set_value(time);
	widget_time->signal_editing_done().connect(sigc::mem_fun(*this, &CellRenderer_Time::on_value_editing_done));
	return widget_time;
#else
	return CellRendererText::start_editing_vfunc(event,widget,path,background_area,cell_area,flags);
#endif
}
