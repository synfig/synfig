/* === S I N F G =========================================================== */
/*!	\file dialog_tooloptions.h
**	\brief Template Header
**
**	$Id: dialog_tooloptions.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SINFG_STUDIO_DIALOG_TOOLOPTIONS_H
#define __SINFG_STUDIO_DIALOG_TOOLOPTIONS_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include "dockable.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	
class Dialog_ToolOptions : public Dockable
{		
	Gtk::Label empty_label;
	Gtk::ScrolledWindow scrolled_;
	Gtk::VBox sub_vbox_;
	
public:
		
	void clear();
	void set_widget(Gtk::Widget&);
	void set_name(const sinfg::String& name);

	Dialog_ToolOptions();
	~Dialog_ToolOptions();
}; // END of Dialog_ToolOptions

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
