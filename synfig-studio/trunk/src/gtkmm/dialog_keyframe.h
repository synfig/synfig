/* === S I N F G =========================================================== */
/*!	\file dialog_keyframe.h
**	\brief Template Header
**
**	$Id: dialog_keyframe.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_STUDIO_DIALOG_KEYFRAME_H
#define __SINFG_STUDIO_DIALOG_KEYFRAME_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gtkmm/window.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>

#include <sinfgapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio
{

class Widget_WaypointModel;

class Dialog_Keyframe : public Gtk::Dialog
{
	Gtk::Tooltips tooltips_;
	etl::handle<sinfgapp::CanvasInterface> canvas_interface;
	
	sinfg::Keyframe keyframe_;

	Gtk::Entry entry_description;
	
	Widget_WaypointModel* widget_waypoint_model;

	void on_ok_pressed();
	void on_apply_pressed();
	void on_delete_pressed();

public:
	Dialog_Keyframe(Gtk::Window& parent,etl::handle<sinfgapp::CanvasInterface> canvas_interface);
	~Dialog_Keyframe();

	const sinfg::Keyframe& get_keyframe()const;
	void set_keyframe(const sinfg::Keyframe& x);

private:

}; // END of class RenderSettings
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
