/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_timetrack2.h
**	\brief Dock to displaying layer parameters timetrack
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2020 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_DOCK_TIMETRACK2_H
#define SYNFIG_STUDIO_DOCK_TIMETRACK2_H

#include <gtkmm/grid.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/toolpalette.h>
#include <gtkmm/radiotoolbutton.h>

#include <gui/docks/dock_canvasspecific.h>
#include <gui/widgets/widget_canvastimeslider.h>
#include <gui/widgets/widget_keyframe_list.h>

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
	Gtk::Scrollbar vscrollbar;
	Gtk::Scrollbar hscrollbar;
	Gtk::ToolPalette tool_palette;

	void on_update_header_height(int height);

	void on_widget_timetrack_waypoint_clicked(synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint,std::less<synfig::UniqueID>> waypoint_set, int button);
	void on_widget_timetrack_waypoint_double_clicked(synfigapp::ValueDesc value_desc, std::set<synfig::Waypoint,std::less<synfig::UniqueID>> waypoint_set, int button);

	void setup_tool_palette();
	void update_tool_palette_action();
	std::map<std::string, Gtk::RadioToolButton*> action_button_map;
};

}

#endif // SYNFIG_STUDIO_DOCK_TIMETRACK2_H
