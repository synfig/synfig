/* === S Y N F I G ========================================================= */
/*!	\file dockmanager.h
**	\brief Template Header
**
**	$Id: dockmanager.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_DOCKMANAGER_H
#define __SYNFIG_DOCKMANAGER_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <list>
#include <synfig/string.h>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include <ETL/smart_ptr>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dockable;
class DockDialog;
class DockSettings;
	
class DockManager : public sigc::trackable
{
	friend class Dockable;
	friend class DockDialog;
	friend class DockSettings;
		
	std::list<Dockable*> dockable_list_;
	std::list<DockDialog*> dock_dialog_list_;

	sigc::signal<void,Dockable*> signal_dockable_registered_;
	
	etl::smart_ptr<DockSettings> dock_settings;

public:
	DockManager();
	~DockManager();

	DockDialog& find_dock_dialog(int id);
	const DockDialog& find_dock_dialog(int id)const;

	sigc::signal<void,Dockable*>& signal_dockable_registered() { return signal_dockable_registered_; }

	void register_dockable(Dockable& x);
	bool unregister_dockable(Dockable& x);
	Dockable& find_dockable(const synfig::String& x);
	void present(synfig::String x);
	
}; // END of class DockManager

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
