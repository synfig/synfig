/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_timetrack.h
**	\brief Widget to displaying layer parameter waypoints along time
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

#ifndef SYNFIG_STUDIO_WIDGET_TIMETRACK_H
#define SYNFIG_STUDIO_WIDGET_TIMETRACK_H


#include <gui/widgets/widget_timegraphbase.h>
#include <gui/selectdraghelper.h>

namespace studio {

class Widget_Timetrack : public Widget_TimeGraphBase
{
public:
	Widget_Timetrack();
	virtual ~Widget_Timetrack() override;

protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

private:

	// Handle mouse actions for panning/zooming/scrolling and waypoint selection
	struct WaypointSD : SelectDragHelper<int>
	{
		// SelectDragHelper interface
	public:
		WaypointSD() : SelectDragHelper<int>("Move waypoints") {}
		virtual ~WaypointSD() override {}
		virtual void get_item_position(const int& , Gdk::Point& ) override {}
		virtual bool find_item_at_position(int, int, int&) override { return false; }
		virtual bool find_items_in_rect(Gdk::Rectangle, std::vector<int>&) override { return false; }
		virtual void get_all_items(std::vector<int>&) override {}
		virtual void delta_drag(int, int, bool) override {}
	} waypoint_sd;

};

}

#endif // SYNFIG_STUDIO_WIDGET_TIMETRACK_H
