/*!	\file gui/widgets/widget_hsv_plane.h
**	\brief Widget that displays a HSV plane for color choosing
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

#ifndef STUDIO_WIDGET_HSV_PLANE_H
#define STUDIO_WIDGET_HSV_PLANE_H

#include <glibmm/property.h>
#include <gtkmm/drawingarea.h>
#include <mutex>
#include <synfig/color.h>
#include <synfig/real.h>

namespace studio {

/**
 * The Widget_HSV_Plane shows a rectangle for user visually selects via Saturation
 * and Value, for a given Hue (HSV color representation).
 *
 * This widget doesn't let user change Hue. It should be done programatically via set_color()
 */
class Widget_HSV_Plane : public Gtk::DrawingArea
{
	void init();
public:
	Widget_HSV_Plane();

	/// Change the currently selected color (and the Hue used on widget)
	void set_color(synfig::Color new_value);
	/// Return the current color
	synfig::Color get_color() const;

	Glib::Property<Gdk::RGBA> property_color;

	using type_signal_activated = sigc::signal<void>;
	type_signal_activated signal_activated();
	using type_signal_editing_started = sigc::signal<void>;
	type_signal_editing_started signal_editing_started();
	using type_signal_editing_done = sigc::signal<void>;
	type_signal_editing_done signal_editing_done();
	using type_signal_editing_canceled = sigc::signal<void>;
	type_signal_editing_canceled signal_editing_canceled();
	using type_signal_value_changed = sigc::signal<void>;
	/// Emitted every time color changes, even while editing
	type_signal_value_changed signal_value_changed();

protected:
	virtual void get_preferred_height_vfunc(int & minimum_height, int & natural_height) const override;
	virtual void get_preferred_width_vfunc(int & minimum_width, int & natural_width) const override;
	virtual void get_preferred_height_for_width_vfunc(int width, int & minimum_height, int & natural_height) const override;
	virtual void get_preferred_width_for_height_vfunc(int height, int & minimum_width, int & natural_width) const override;

	virtual bool on_button_press_event(GdkEventButton* button_event) override;
	virtual bool on_button_release_event(GdkEventButton* button_event) override;
	virtual bool on_configure_event(GdkEventConfigure* configure_event) override;
	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
	virtual bool on_motion_notify_event(GdkEventMotion* motion_event) override;

private:
	synfig::Real hue;
	synfig::Color color;
	synfig::Color previous_color; ///< Color before user starting dragging

	bool queued_redraw_pixbuf;
	bool pixbuf_invalid;
	std::mutex pixbuf_mutex;
	Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	void queue_redraw_pixbuf();
	void redraw_pixbuf();

	bool dragging;

	type_signal_activated signal_activated_;
	type_signal_editing_started signal_editing_started_;
	type_signal_editing_done signal_editing_done_;
	type_signal_editing_canceled signal_editing_canceled_;
	type_signal_value_changed signal_value_changed_;

	synfig::Color get_color_at(double x, double y) const;

// Glade & GtkBuilder related
public:
	Widget_HSV_Plane(BaseObjectType* cobject);
	static Glib::ObjectBase* wrap_new(GObject* o);
	static void register_type();
private:
	static GType gtype;
};

}
#endif // STUDIO_WIDGET_HSV_PLANE_H
