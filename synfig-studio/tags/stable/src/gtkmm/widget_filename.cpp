/* === S I N F G =========================================================== */
/*!	\file widget_filename.cpp
**	\brief Template File
**
**	$Id: widget_filename.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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
#include "widget_filename.h"
#include "app.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
//using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

#if ! defined(_)
#define _(x)	(x)
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Filename::Widget_Filename()
{
	entry_filename=manage(new Gtk::Entry());
	button_choose=manage(new Gtk::Button(_("Find")));

	pack_start(*entry_filename);
	pack_start(*button_choose);
	entry_filename->show();
	button_choose->show();

	button_choose->signal_clicked().connect(sigc::mem_fun(*this, &studio::Widget_Filename::on_button_choose_pressed));
	//entry_filename->signal_value_changed().connect(sigc::mem_fun(*this, &studio::Widget_Filename::on_value_changed));
	entry_filename->signal_activate().connect(sigc::mem_fun(*this, &studio::Widget_Filename::on_value_changed));
}

Widget_Filename::~Widget_Filename()
{
}

void
Widget_Filename::set_has_frame(bool x)
{
	entry_filename->set_has_frame(x);
}


void
Widget_Filename::set_value(const string &data)
{
	entry_filename->set_text(data);
}

string
Widget_Filename::get_value() const
{
	try
	{
		return entry_filename->get_text();
	}
	catch(...)
	{
		throw string("Caught unknown exception");
	}
}

void
Widget_Filename::on_value_changed()
{
	signal_value_changed()();
}

void
Widget_Filename::on_button_choose_pressed()
{
	string filename=entry_filename->get_text();
	if(filename.empty())
		filename=".";
	if(App::dialog_open_file(_("Choose File"),filename))
		entry_filename->set_text(filename);
}
