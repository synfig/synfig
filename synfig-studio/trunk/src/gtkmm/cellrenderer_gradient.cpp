/* === S I N F G =========================================================== */
/*!	\file cellrenderer_gradient.cpp
**	\brief Template File
**
**	$Id: cellrenderer_gradient.cpp,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
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
#include "cellrenderer_gradient.h"
#include "widget_gradient.h"
#include "app.h"

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

CellRenderer_Gradient::CellRenderer_Gradient():
	Glib::ObjectBase	(typeid(CellRenderer_Gradient)),
	Gtk::CellRendererText	(),
	property_gradient_(*this,"gradient",sinfg::Gradient())
{
	//CellRendererText::signal_edited().connect(sigc::mem_fun(*this,&studio::CellRenderer_Gradient::string_edited_));
}

CellRenderer_Gradient::~CellRenderer_Gradient()
{
}


void
CellRenderer_Gradient::render_vfunc(
		const Glib::RefPtr<Gdk::Drawable>& window,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& ca,
		const Gdk::Rectangle& expose_area,
		Gtk::CellRendererState flags)
{
	if(!window)
		return;
	render_gradient_to_window(window,ca,property_gradient_.get_value());
}


Gtk::CellEditable*
CellRenderer_Gradient::start_editing_vfunc(
	GdkEvent* event,
	Gtk::Widget& widget,
	const Glib::ustring& path,
	const Gdk::Rectangle& background_area,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState flags)
{
	return 0;
}
