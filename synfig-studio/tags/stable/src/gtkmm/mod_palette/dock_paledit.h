/* === S I N F G =========================================================== */
/*!	\file dialog_palette.h
**	\brief Template Header
**
**	$Id: dock_paledit.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_STUDIO_DOCK_PAL_EDIT_H
#define __SINFG_STUDIO_DOCK_PAL_EDIT_H

/* === H E A D E R S ======================================================= */

#include <gtk/gtk.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/checkbutton.h>

#include <sinfg/gamma.h>
#include <sinfg/time.h>

#include "../widget_coloredit.h"

#include <sinfgapp/value_desc.h>
#include <sinfg/time.h>

#include "../dockable.h"
#include <vector>
#include <gtkmm/actiongroup.h>

#include <sinfg/palette.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {
class CanvasInterface;
};

namespace studio {

class Widget_Color;
class PaletteSettings;
	
class Dock_PalEdit : public Dockable
{
	friend class PaletteSettings;

	Glib::RefPtr<Gtk::ActionGroup> action_group;

	sinfg::Palette palette_;
	
	Gtk::Table table;
	
	void on_add_pressed();
	
	void show_menu(int i);

	sigc::signal<void> signal_changed_;


private:
	int add_color(const sinfg::Color& x);
	void set_color(sinfg::Color x, int i);
	void erase_color(int i);

	void select_color(int i);
	sinfg::Color get_color(int i)const;
	void edit_color(int i);
public:
	void set_palette(const sinfg::Palette& x);
	const sinfg::Palette& get_palette()const { return palette_; }

	int size()const;

	void set_default_palette();
	
	void refresh();

	const sigc::signal<void>& signal_changed() { return signal_changed_; }
		
	Dock_PalEdit();
	~Dock_PalEdit();
}; // END of Dock_PalEdit

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
