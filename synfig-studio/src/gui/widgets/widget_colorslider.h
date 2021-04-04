/*!	\file gui/widgets/widget_colorslider.h
**	\brief Widget for choosing the value of a color component
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

#ifndef STUDIO_WIDGET_COLORSLIDER_H
#define STUDIO_WIDGET_COLORSLIDER_H

#include <gtkmm/drawingarea.h>
#include <synfig/color.h>

namespace studio {

/**
 * ColorSlider let user visually change a given color component by sliding/clicking
 *
 * Some color representation and its components are supported: RGB, YUV, HS.
 *
 * Alpha is supported too.
 */
class ColorSlider : public Gtk::DrawingArea
{
public:
	enum Type
	{
		TYPE_R,
		TYPE_G,
		TYPE_B,
		TYPE_Y,
		TYPE_U,
		TYPE_V,
		TYPE_HUE,
		TYPE_SAT,
		TYPE_A,

		TYPE_END
	};

private:

	sigc::signal<void,Type,float> signal_slider_moved_;
	sigc::signal<void> signal_activated_;

	Type type;
	synfig::Color color_;

public:

	Type get_type()const { return type; }

	void set_color(synfig::Color x);
	const synfig::Color& get_color()const { return color_; }

	sigc::signal<void,Type,float>& signal_slider_moved() { return signal_slider_moved_; }
	sigc::signal<void>& signal_activated() { return signal_activated_; }

	/// Modify the given color by changing the color component type to amount
	static void adjust_color(Type type, synfig::Color &color, float amount);

private:
	typedef void (*slider_color_func)(synfig::Color &,float);

	static void slider_color_TYPE_R(synfig::Color &color, float amount);
	static void slider_color_TYPE_G(synfig::Color &color, float amount);
	static void slider_color_TYPE_B(synfig::Color &color, float amount);
	static void slider_color_TYPE_Y(synfig::Color &color, float amount);
	static void slider_color_TYPE_U(synfig::Color &color, float amount);
	static void slider_color_TYPE_V(synfig::Color &color, float amount);
	static void slider_color_TYPE_HUE(synfig::Color &color, float amount);
	static void slider_color_TYPE_SAT(synfig::Color &color, float amount);
	static void slider_color_TYPE_A(synfig::Color &color, float amount);

	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	bool on_event(GdkEvent *event);

    ///@brief Draw face to face contrasted arrows
	void draw_arrow(
		const Cairo::RefPtr<Cairo::Context> &cr,
		double x, double y,
		double width, double height,
		int size,
		bool fill);
}; // END of class ColorSlider

}; // END of namespace studio

#endif // STUDIO_WIDGET_COLORSLIDER_H
