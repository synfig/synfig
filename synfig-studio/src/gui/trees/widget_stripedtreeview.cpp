/* === S Y N F I G ========================================================= */
/*!	\file widget_stripedtreeview.cpp
**	\brief TBase class for TreeView widget with striped rows
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2023 Synfig contributors
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/trees/widget_stripedtreeview.h>

#include <gtkmm/cssprovider.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Widget_StripedTreeView::Widget_StripedTreeView()
	: Gtk::TreeView()
{
	auto provider = Gtk::CssProvider::create();
	provider->load_from_data(".synfig-special-treeview-transparent-background { background-color: rgba(0,0,0,0); }");
	get_style_context()->add_provider(provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

bool
Widget_StripedTreeView::on_draw(const ::Cairo::RefPtr<::Cairo::Context>& cr)
{
	Gtk::TreeViewColumn *col = this->get_column(0);

	Gtk::TreePath start_path, end_path;
	get_visible_range(start_path, end_path);

	auto context = get_style_context();

	bool will_hack_to_draw_odd_even_rows = !start_path.empty();

	if (!will_hack_to_draw_odd_even_rows) {
		Gtk::TreeView::on_draw(cr);
		return false;
	}

	// Alternate row color hack strategy:
	// We first paint the background color of each visible row,
	// adding extra classes "even" or "odd" for each row, to
	// properly color its background.
	// Then we ask Gtk to normally draw the Gtk::TreeView, but
	// with a hacked transparent widget background, in order to
	// not override our stripped background pattern of alternate
	// colors.

	auto selection = get_selection();
	int row_number = 0;

	get_model()->foreach_path([=, &row_number](const Gtk::TreeModel::Path& path) -> bool {

		++row_number;

		if (path < start_path)
			return false;
		if (!end_path.empty() && path > end_path)
			return true;

		Gdk::Rectangle rect;
		this->get_cell_area(path, *col, rect);

		if (rect.get_height() == 0) {
			--row_number;
			return false;
		}

		int wx, wy, ww, wh;
		convert_bin_window_to_widget_coords(0, rect.get_y(), wx, wy);
		convert_bin_window_to_widget_coords(get_width(), rect.get_y() + rect.get_height(), ww, wh);
		ww -= wx;
		wh -= wy;

		// alternate even-odd row colors
		{
			const std::string class_name = row_number % 2 == 0 ? "even" : "odd";
			context->add_class(class_name);
			context->render_background(cr, 0, wy, ww, wh);
			context->remove_class(class_name);
		}

		// draw selection
		if (selection->is_selected(path)) {
			auto previous_state = context->get_state();
			context->set_state(previous_state | Gtk::STATE_FLAG_SELECTED);
			context->render_background(cr, 0, wy, ww, wh);
			context->set_state(previous_state);
		}

		return false;
	});

	std::string transparent_background_class_name = "synfig-special-treeview-transparent-background";

	context->add_class(transparent_background_class_name);

	Gtk::TreeView::on_draw(cr);

	context->remove_class(transparent_background_class_name);

	return false;
}
