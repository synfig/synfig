/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_template.h
**	\brief Dialog design list and panel template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2016 Jerome Blanchi
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

#ifndef __SYNFIG_STUDIO_DIALOG_TEMPLATE_H
#define __SYNFIG_STUDIO_DIALOG_TEMPLATE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gtkmm/grid.h>
#include <gtkmm/notebook.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */


/* === C L A S S E S & S T R U C T S ======================================= */
namespace studio {

/*! \class Dialog Template
    \brief A dialog design template class.

    Use this abstract class to build in a generic way the synfig studio
    dialogs to keep design consistency.
*/
class Dialog_Template : public Gtk::Dialog
{

	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	//Child widgets:
	Gtk::Notebook *notebook;
	Gtk::Grid main_grid;
	Gtk::ScrolledWindow categories_scrolledwindow;
	Gtk::TreeView categories_treeview;
	Glib::RefPtr<Gtk::TreeStore> categories_reftreemodel;

	//Preferences Categories Tree model columns:
	class Categories : public Gtk::TreeModel::ColumnRecord
	{
		public:

		Categories() { add(category_id); add(category_name); }

		Gtk::TreeModelColumn<int> category_id;
		Gtk::TreeModelColumn<Glib::ustring> category_name;
	};
	Categories categories;

	int page_index;

	/*
 -- ** -- P R O T E C T E D   D A T A -----------------------------------------------
	*/
protected:
	// Style for title(s)
	Pango::AttrList title_attrlist;
	Pango::AttrList section_attrlist;

	struct PageInfo
	{
		Gtk::Grid* grid;
		Gtk::TreeRow row;
	};

	/*
 -- ** -- P U B L I C   D A T A -----------------------------------------------
	*/

public:

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	void on_treeviewselection_changed ();

	/*
 -- ** -- P R O T E C T E D   M E T H O D S ---------------------------------------
	*/

protected:

	//User Interface Design
	//! \Brief Set the main title of the page
	void attach_label_title(Gtk::Grid *grid, synfig::String str);
	//! \Brief Add a new section (col 0) at specified row
	void attach_label_section(Gtk::Grid *grid, synfig::String str, guint row);
	//! \Brief Add a single label (col 0) at specified row
	void attach_label(Gtk::Grid *grid, synfig::String str, guint row);
	//! \Brief Add a single label at specified row and col
	//! \return Gtk::Label* for further change
	Gtk::Label* attach_label(Gtk::Grid *grid, synfig::String str, guint row, guint col, bool endstring=true);

	//! \Brief Add a new page to the Notebook and Treeview collection
	//! \return PageInfo used to fill with widget the new page
	PageInfo add_page(synfig::String page_title);
	//! \Brief Add a new child page to the Notebook and Treeview collection
	//! \return PageInfo used to fill with widget the new page
	PageInfo add_child_page(synfig::String page_title, Gtk::TreeRow parentrow);

	/*
 -- ** -- S I G N A L S -------------------------------------------------------
	*/
	//Signal handlers dialog
	virtual void on_ok_pressed() ;
	virtual void on_apply_pressed() = 0;
	virtual void on_restore_pressed() {}

	/*
 -- ** -- P U B L I C   M E T H O D S ---------------------------------------
	*/

public:

	Dialog_Template(Gtk::Window& parent, synfig::String dialog_title);
	~Dialog_Template();

}; // END of Dialog_Template

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
