/* === S Y N F I G ========================================================= */
/*!	\file docks/dockdialog.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_DOCK_DIALOG_H
#define __SYNFIG_STUDIO_DOCK_DIALOG_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/window.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Box; class Paned;  };
namespace studio {

class DockManager;
class DockBook;
class Dockable;
class Widget_CompSelect;
class CanvasView;

class DockDialog : public Gtk::Window
{
	friend class DockManager;
	friend class DockBook;
	friend class Dockable;

	sigc::connection empty_sig;

	bool is_deleting;

private:
	int id_;

	bool on_delete_event(GdkEventAny *event);
	bool on_key_press_event(GdkEventKey *evkey);
	void set_id(int x) { id_=x; }

public:
	bool close();

	int get_id()const { return id_; }

	DockDialog();
	~DockDialog();
}; // END of studio::DockDialog

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
