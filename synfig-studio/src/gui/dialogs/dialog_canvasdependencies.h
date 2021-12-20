/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_canvasdependencies.h
**	\brief Header for Dialog that shows external dependencies for a canvas
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2021 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_DIALOG_CANVASDEPENDENCIES_H
#define SYNFIG_STUDIO_DIALOG_CANVASDEPENDENCIES_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/treestore.h>

#include <synfig/canvas.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
class Label;
}

namespace studio {

class Dialog_CanvasDependencies : public Gtk::Dialog
{
	const Glib::RefPtr<Gtk::Builder> builder;

	Glib::RefPtr<Gtk::TreeStore> external_canvas_model;
	Glib::RefPtr<Gtk::TreeStore> external_resource_model;
	Gtk::Label * canvas_filepath_label;

	synfig::Canvas::Handle canvas;

public:
	Dialog_CanvasDependencies(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	static Dialog_CanvasDependencies * create(Gtk::Window& parent);
	virtual ~Dialog_CanvasDependencies();

	void set_canvas(synfig::Canvas::Handle canvas);

private:
	void refresh();
};

};

#endif // SYNFIG_STUDIO_DIALOG_CANVASDEPENDENCIES_H
