/* === S I N F G =========================================================== */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: iconcontroler.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_ICONCONTROLER_H
#define __SINFG_STUDIO_ICONCONTROLER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/stock.h>
#include <gtkmm/iconfactory.h>
#include <gtkmm/iconset.h>
#include <gdkmm/cursor.h>

#include <sinfg/value.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg { class ValueNode; class Layer; }

namespace sinfgapp { namespace Action { class BookEntry; };};

namespace studio {


class IconControler
{
	Glib::RefPtr<Gtk::IconFactory> icon_factory;
public:
	IconControler(const sinfg::String& basepath);
	~IconControler();

	static Gdk::Cursor get_normal_cursor();
	static Gdk::Cursor get_tool_cursor(const Glib::ustring& name,const Glib::RefPtr<Gdk::Window>& window);
};

Gtk::StockID layer_icon(const sinfg::String &layer);
Glib::RefPtr<Gdk::Pixbuf> get_tree_pixbuf_layer(const sinfg::String &layer);

Gtk::StockID value_icon(sinfg::ValueBase::Type type);
Gtk::StockID valuenode_icon(etl::handle<sinfg::ValueNode> value_node);
Glib::RefPtr<Gdk::Pixbuf> get_tree_pixbuf(sinfg::ValueBase::Type type);
Gtk::StockID get_action_stock_id(const sinfgapp::Action::BookEntry& action);

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
