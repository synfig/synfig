/* === S Y N F I G ========================================================= */
/*!	\file metadatatreestore.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_METADATATREESTORE_H
#define __SYNFIG_STUDIO_METADATATREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treestore.h>
#include <synfigapp/canvasinterface.h>
#include <gdkmm/pixbuf.h>
#include <synfigapp/action.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp { class CanvasInterface; }

namespace studio {

class MetaDataTreeStore : virtual public Gtk::TreeStore
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	class Model : public Gtk::TreeModel::ColumnRecord
	{
	public:
	public:
		Gtk::TreeModelColumn<Glib::ustring> key;
		Gtk::TreeModelColumn<Glib::ustring> data;

		Model()
		{
			add(key);
			add(data);
		}
	};

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	const Model model;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void meta_data_changed(synfig::String key);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	~MetaDataTreeStore();

	etl::loose_handle<synfigapp::CanvasInterface> get_canvas_interface() { return canvas_interface_; }
	etl::loose_handle<const synfigapp::CanvasInterface> get_canvas_interface()const { return canvas_interface_; }
	const synfig::Canvas::Handle& get_canvas() const { return canvas_interface_->get_canvas(); }

	void rebuild();

	void refresh() { rebuild(); }

	/*
 -- ** -- P R O T E C T E D   M E T H O D S -----------------------------------
	*/

protected:
	MetaDataTreeStore(etl::loose_handle<synfigapp::CanvasInterface>);
	void get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const;
	void set_value_impl(const Gtk::TreeModel::iterator& iter, int column, const Glib::ValueBase& value);

public:

	static Glib::RefPtr<MetaDataTreeStore> create(etl::loose_handle<synfigapp::CanvasInterface>);

}; // END of class MetaDataTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
