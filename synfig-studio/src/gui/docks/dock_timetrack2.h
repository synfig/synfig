/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_timetrack2.h
**	\brief Dock to displaying layer parameters timetrack
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2020 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_DOCK_TIMETRACK2_H
#define SYNFIG_STUDIO_DOCK_TIMETRACK2_H

#include "dock_canvasspecific.h"

#include <widgets/widget_canvastimeslider.h>
#include <widgets/widget_keyframe_list.h>

#include <gtkmm/grid.h>
#include <gtkmm/box.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/toolpalette.h>
#include <gtkmm/radiotoolbutton.h>

namespace studio {

class Widget_Timetrack;

class Dock_Timetrack2 : public Dock_CanvasSpecific
{
public:
	Dock_Timetrack2();

protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

private:
	Gtk::Grid grid;

	Widget_Keyframe_List widget_kf_list;
	Widget_CanvasTimeslider widget_timeslider;
	Widget_Timetrack *current_widget_timetrack;
	Gtk::VScrollbar vscrollbar;
	Gtk::HScrollbar hscrollbar;
	Gtk::ToolPalette tool_palette;

	void on_update_header_height(int height);

	void setup_tool_palette();
	void update_tool_palette_action();
	std::map<std::string, Gtk::RadioToolButton*> action_button_map;
};

}

#endif // SYNFIG_STUDIO_DOCK_TIMETRACK2_H
