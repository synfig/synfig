/* === S Y N F I G ========================================================= */
/*!	\file workarearenderer.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_WORKAREARENDERER_H
#define __SYNFIG_WORKAREARENDERER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <sigc++/sigc++.h>
#include <synfig/vector.h>
#include <gdkmm/window.h>
#include <gdkmm/rectangle.h>

/* === M A C R O S ========================================================= */
#define GDK_COLOR_TO_RGB(color)               \
    color.get_red_p (), color.get_green_p (), color.get_blue_p ()

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class WorkArea;

class WorkAreaRenderer : public etl::shared_object, public sigc::trackable
{
public:
	typedef etl::handle<WorkAreaRenderer> Handle;
	typedef etl::loose_handle<WorkAreaRenderer> LooseHandle;

private:
	bool enabled_;
	int priority_;

	sigc::signal<void> signal_changed_;

	WorkArea* work_area_;

public:

	sigc::signal<void>& signal_changed() { return signal_changed_; }

public:
	int get_w()const;
	int get_h()const;

	float get_pw()const;
	float get_ph()const;

	//! Converts screen coords (ie: pixels) to composition coordinates
	synfig::Point screen_to_comp_coords(synfig::Point pos)const;

	//! Converts composition coordinates to screen coords (ie: pixels)
	synfig::Point comp_to_screen_coords(synfig::Point pos)const;

	WorkAreaRenderer();
	virtual ~WorkAreaRenderer();

	bool get_enabled()const { return get_enabled_vfunc(); }
	int get_priority()const { return priority_; }
	WorkArea* get_work_area()const { return work_area_; }

	void set_enabled(bool x);
	void set_priority(int x);
	void set_work_area(WorkArea* work_area_);

	virtual void render_vfunc(
		const Glib::RefPtr<Gdk::Window>& window,
		const Gdk::Rectangle& expose_area
	);

	virtual bool event_vfunc(
		GdkEvent* event
	);

protected:

	virtual bool get_enabled_vfunc()const;

public:
	bool operator<(const WorkAreaRenderer &rhs) const
		{ return priority_<rhs.priority_; }
};

inline bool operator<(const WorkAreaRenderer::Handle &lhs,const WorkAreaRenderer::Handle &rhs)
	{ return lhs->get_priority() < rhs->get_priority(); }

inline bool operator<(const WorkAreaRenderer::LooseHandle &lhs,const WorkAreaRenderer::LooseHandle &rhs)
	{ return lhs->get_priority() < rhs->get_priority(); }

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
