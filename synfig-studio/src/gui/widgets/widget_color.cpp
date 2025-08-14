/* === S Y N F I G ========================================================= */
/*!	\file widget_color.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/widgets/widget_color.h>

#include <gui/app.h>
#include <gui/exception_guard.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

void
studio::render_color_to_window(const Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &ca, const synfig::Color &color)
{
	const int height(ca.get_height());
	const int width(ca.get_width());

	const int square_size(height/2);

	Gamma gamma = App::get_selected_canvas_gamma().get_inverted();
	
	if(color.get_alpha()!=1.0)
	{
		// In this case we need to render the alpha squares

		const Color bg1 = gamma.apply(
			Color::blend(color,Color(0.75, 0.75, 0.75),1.0).clamped() );
		const Color bg2 = gamma.apply(
			Color::blend(color,Color(0.5, 0.5, 0.5),1.0).clamped() );

		bool toggle(false);
		for(int i=0;i<width;i+=square_size)
		{
			const int square_width(std::min(square_size,width-i));

			if(toggle)
			{
		        cr->set_source_rgb(bg1.get_r(), bg1.get_g(), bg1.get_b());
		        cr->rectangle(ca.get_x()+i, ca.get_y(), square_width, square_size);
		        cr->fill();

		        cr->set_source_rgb(bg2.get_r(), bg2.get_g(), bg2.get_b());
		        cr->rectangle(ca.get_x()+i, ca.get_y()+square_size, square_width, square_size);
		        cr->fill();
				toggle=false;
			}
			else
			{
		        cr->set_source_rgb(bg2.get_r(), bg2.get_g(), bg2.get_b());
		        cr->rectangle(ca.get_x()+i, ca.get_y(), square_width, square_size);
		        cr->fill();

		        cr->set_source_rgb(bg1.get_r(), bg1.get_g(), bg1.get_b());
		        cr->rectangle(ca.get_x()+i, ca.get_y()+square_size, square_width, square_size);
		        cr->fill();
				toggle=true;
			}
		}
	}
	else
	{
		synfig::Color c = gamma.apply(color.clamped());
        cr->set_source_rgb(c.get_r(), c.get_g(), c.get_b());
        cr->rectangle(ca.get_x(), ca.get_y(), width-1, height-1);
        cr->fill();
	}

	cr->set_source_rgb(1.0, 1.0, 1.0);
    cr->rectangle(ca.get_x()+1, ca.get_y()+1, width-3, height-3);
    cr->stroke();

    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->rectangle(ca.get_x(), ca.get_y(), width-1, height-1);
    cr->stroke();
}

/* === C L A S S E S ======================================================= */


/* === M E T H O D S ======================================================= */

Widget_Color::Widget_Color()
{
	color=Color(0,0,0,0);
	set_size_request(-1,16);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	App::signal_canvas_view_focus().connect(
		sigc::hide(sigc::mem_fun(*this,&studio::Widget_Color::queue_draw)) );
}

Widget_Color::~Widget_Color()
{
}

void 
Widget_Color::push_mouse_binding(const synfig::String& action, const std::pair<GdkModifierType, int>& mouse_binding)
{
	widget_mouse_bindings_.insert({ action, mouse_binding });
	widget_mouse_binding_signals_.insert({ action, sigc::signal<void>() });
}

void
Widget_Color::set_value(const synfig::Color &x)
{
	assert(x.is_valid());
	color=x;
	queue_draw();
}

const synfig::Color &
Widget_Color::get_value() const
{
	assert(color.is_valid());
	return color;
}

bool
Widget_Color::on_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
	switch(event->type)
	{
	case GDK_BUTTON_PRESS:

		for(auto&& mouse_binding : widget_mouse_bindings_)
		{
			synfig::String mouse_binding_action_name = mouse_binding.first;
			std::pair<GdkModifierType, int> mouse_binding_action_shortcut = mouse_binding.second;

			GdkModifierType modifiers = mouse_binding_action_shortcut.first;
			int button = mouse_binding_action_shortcut.second;

			if(event->button.button == guint(button) && event->button.state == modifiers)
			{
				if(widget_mouse_binding_signals_.find(mouse_binding_action_name) != widget_mouse_binding_signals_.end())
				{
					sigc::signal<void> mouse_binding_signal = widget_mouse_binding_signals_[mouse_binding_action_name];
					mouse_binding_signal();
					return true;
				}
				else
				{
					std::cerr << "Mouse binding for the action \'" << mouse_binding_action_name << "\' not found!";
					return false;
				}
			}
		}
		if(event->button.button==1)
		{
			if(event->button.state & state_flags)
			{
				signal_activate_with_modifier_();
				return true;
			}

			signal_activate_();
			return true;
		}
		if(event->button.button==2)
		{
			signal_middle_click_();
			return true;
		}
		if(event->button.button==3)
		{
			signal_right_click_();
			return true;
		}
		break;

	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
Widget_Color::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	render_color_to_window(cr, Gdk::Rectangle(0,0,get_width(),get_height()), color);
	return true;
}
