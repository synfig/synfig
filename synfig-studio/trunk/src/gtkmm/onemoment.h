/*! ========================================================================
** Synfig
** Template Header File
** $Id: onemoment.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_GTKMM_ONEMOMENT_H
#define __SYNFIG_GTKMM_ONEMOMENT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/window.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class ProgressCallback; };

namespace studio {

class OneMoment : public Gtk::Window
{
public:
	
	OneMoment();
	~OneMoment();
};

}

/* === E N D =============================================================== */

#endif
