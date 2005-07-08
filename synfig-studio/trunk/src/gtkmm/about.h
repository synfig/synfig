/*! ========================================================================
** Synfig
** Template Header File
** $Id: about.h,v 1.1.1.1 2005/01/07 03:34:35 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
** This software and associated documentation
** are CONFIDENTIAL and PROPRIETARY property of
** the above-mentioned copyright holder.
**
** You may not copy, print, publish, or in any
** other way distribute this software without
** a prior written agreement with
** the copyright holder.
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_ABOUT_H
#define __SYNFIG_GTKMM_ABOUT_H

/* === H E A D E R S ======================================================= */

//#include <gtk/gtk.h>
#include <gtkmm/window.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/progressbar.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class ProgressCallback; };

namespace studio {

class AboutProgress;
	
class About : public Gtk::Window
{
	friend class AboutProgress;
	
	AboutProgress *cb;
	
	Gtk::Tooltips _tooltips;

	Gtk::Label *tasklabel;
	Gtk::ProgressBar *progressbar;
	Gtk::Button *CloseButton;

	void close();

	bool can_self_destruct;
	
public:
	
	synfig::ProgressCallback *get_callback();	

	void set_can_self_destruct(bool x);

	About();
	~About();
};

}

/* === E N D =============================================================== */

#endif
