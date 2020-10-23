/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_ruler.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_STUDIO_WIDGET_RULER_H
#define __SYNFIG_STUDIO_WIDGET_RULER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/drawingarea.h>
#include <pangomm/layout.h>

#include <synfig/real.h>
#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Ruler : public Gtk::DrawingArea
{
private:
	bool is_vertical;
	Glib::RefPtr<Pango::Layout> layout;
	synfig::Real min;
	synfig::Real max;
	synfig::Real position;

	void draw_line(
		const ::Cairo::RefPtr< ::Cairo::Context>& cr,
		synfig::Real position,
		synfig::Real size,
		const Gdk::RGBA &color,
		synfig::Real width,
		synfig::Real height );

	void draw_text(
		const ::Cairo::RefPtr< ::Cairo::Context>& cr,
		synfig::Real position,
		const synfig::String &text,
		int size,
		const Gdk::RGBA &color,
		synfig::Real offset,
		synfig::Real width,
		synfig::Real height );

public:
	Widget_Ruler(bool is_vertical);
	~Widget_Ruler();

	synfig::Real get_screen_min() const
		{ return 0.0; }
	synfig::Real get_screen_max() const
		{ return (synfig::Real)(is_vertical ? get_height() : get_width()); }

	synfig::Real position_to_screen(synfig::Real value) const
		{ return (value - min)/(max - min)*(get_screen_max()-get_screen_min()) + get_screen_min(); }
	synfig::Real position_from_screen(synfig::Real value) const
		{ return (value - get_screen_min())/(get_screen_max()-get_screen_min())*(max - min) + min; }

	synfig::Real distance_to_screen(synfig::Real value) const
		{ return value/(max - min)*(get_screen_max()-get_screen_min()); }
	synfig::Real distance_from_screen(synfig::Real value) const
		{ return value/(get_screen_max()-get_screen_min())*(max - min); }

	synfig::Real get_min() const { return min; }
	void set_min(synfig::Real value);

	synfig::Real get_max() const { return max; }
	void set_max(synfig::Real value);

	synfig::Real get_position() const { return position; }
	void set_position(synfig::Real value);

	synfig::Real get_screen_position() const
		{ return position_to_screen(get_position()); }
	void set_screen_position(synfig::Real value)
		{ set_position(position_from_screen(value)); }

	bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context>& cr);
}; // END of class Widget_Ruler

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
