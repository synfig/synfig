/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_canvastimeslider.h
**	\brief Canvas Time Slider Widget Header
**
**	$Id$
**
**	\legal
**	......... ... 2018 Ivan Mahohnin
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

#ifndef __SYNFIG_WIDGET_CANVASTIMESLIDER_H
#define __SYNFIG_WIDGET_CANVASTIMESLIDER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/drawingarea.h>
#include <gtkmm/window.h>

#include <gui/widgets/widget_timeslider.h>

#include <synfig/vector.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CanvasView;
class LockDucks;

class Widget_CanvasTimeslider : public Widget_Timeslider
{
protected:
	sigc::connection rendering_change;
	etl::loose_handle<CanvasView> canvas_view;
	etl::handle<LockDucks> lock_ducks;
	Gtk::Window tooltip;
	Gtk::DrawingArea thumb;
	Cairo::RefPtr<Cairo::SurfacePattern> thumb_background;
	Cairo::RefPtr<Cairo::ImageSurface> thumb_surface;

	virtual void draw_background(const Cairo::RefPtr<Cairo::Context> &cr);

	bool draw_thumb(const Cairo::RefPtr<Cairo::Context> &cr);
	void show_tooltip(const synfig::Point &p, const synfig::Point &root);

	virtual bool on_motion_notify_event(GdkEventMotion* event);
	virtual bool on_button_press_event(GdkEventButton *event);
	virtual bool on_button_release_event(GdkEventButton *event);
	virtual bool on_leave_notify_event(GdkEventCrossing* event);

public:
	Widget_CanvasTimeslider();
	~Widget_CanvasTimeslider();

	const etl::loose_handle<CanvasView>& get_canvas_view() const { return canvas_view; }
	void set_canvas_view(const etl::loose_handle<CanvasView> &x);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
