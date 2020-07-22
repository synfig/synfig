/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_gammapattern.h
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

#ifndef __SYNFIG_STUDIO_WIDGET_GAMMAPATTERN_H
#define __SYNFIG_STUDIO_WIDGET_GAMMAPATTERN_H

#include <gtkmm/drawingarea.h>

#include <synfig/color/gamma.h>

namespace studio {

class Widget_GammaPattern : public Gtk::DrawingArea
{
	synfig::Gamma gamma;
	int tile_w, tile_h, gradient_h;
	Cairo::RefPtr<Cairo::SurfacePattern> pattern;

	void init();

	// Glade & GtkBuilder related
	static GType gtype;
public:
	Widget_GammaPattern();

	void set_gamma(const synfig::Gamma &x);
	const synfig::Gamma &get_gamma() const;

	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

	// Glade & GtkBuilder related
	Widget_GammaPattern(BaseObjectType *cobject);
	static Glib::ObjectBase *wrap_new(GObject *o);
	static void register_type();
};

};

#endif
