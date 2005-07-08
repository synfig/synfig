/* === S Y N F I G ========================================================= */
/*!	\file dialog_tooloptions.cpp
**	\brief Template File
**
**	$Id: dialog_tooloptions.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <synfig/general.h>
#include "dialog_tooloptions.h"
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

Dialog_ToolOptions::Dialog_ToolOptions():
	Dockable("tool_options",_("Tool Options"),Gtk::StockID("synfig-normal")),
	empty_label(_("This tool has no options"))
{
	//scrolled_.add(sub_vbox_);
	//scrolled_.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	//scrolled_.show();	
	//get_vbox()->pack_start(scrolled_);

	add(sub_vbox_);
	
	set_widget(empty_label);
	empty_label.show();
}

Dialog_ToolOptions::~Dialog_ToolOptions()
{
}

void
Dialog_ToolOptions::clear()
{
	Dockable::clear();
	set_local_name(_("Tool Options"));
	add(sub_vbox_);
	set_widget(empty_label);
	empty_label.show();

	set_stock_id(Gtk::StockID("synfig-normal"));
}

void
Dialog_ToolOptions::set_widget(Gtk::Widget&x)
{
	if(!sub_vbox_.children().empty())
		sub_vbox_.children().clear();

	sub_vbox_.show();
	sub_vbox_.pack_start(x,false,false);
	x.show();
}

void
Dialog_ToolOptions::set_name(const synfig::String& name)
{
	set_stock_id(Gtk::StockID("synfig-"+name));
}
