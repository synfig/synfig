/* === S Y N F I G ========================================================= */
/*!	\file trees/keyframetree.h
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

#ifndef __SYNFIG_STUDIO_KEYFRAMETREE_H
#define __SYNFIG_STUDIO_KEYFRAMETREE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/value_desc.h>
#include "trees/keyframetreestore.h"
#include <synfig/keyframe.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CellRenderer_Time;

class KeyframeTree : public Gtk::TreeView
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	enum ColumnID
	{
		COLUMNID_TIME,
		COLUMNID_DESCRIPTION,
		COLUMNID_JUMP,

		COLUMNID_END			//!< \internal
	};

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	KeyframeTreeStore::Model model;
	synfig::Keyframe selected_kf;


	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	Glib::RefPtr<KeyframeTreeStore> keyframe_tree_store_;

	CellRenderer_Time *cell_renderer_time;

	CellRenderer_Time *cell_renderer_time_delta;

	Gtk::CellRendererText *cell_renderer_description;

	sigc::signal<void,synfig::Keyframe> signal_edited_;

	sigc::signal<void,synfig::Keyframe,synfig::Time> signal_edited_time_;

	sigc::signal<void,synfig::Keyframe,synfig::String> signal_edited_description_;

	sigc::signal<void, int, Gtk::TreeRow, ColumnID> signal_user_click_;

	bool editable_;

	bool send_selection;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_keyframe_toggle(const Glib::ustring& path_string);

	void on_edited_time(const Glib::ustring&path_string,synfig::Time time);

	void on_edited_time_delta(const Glib::ustring&path_string,synfig::Time time);

	void on_edited_description(const Glib::ustring&path_string,const Glib::ustring &description);

	virtual bool on_event(GdkEvent *event);

	void on_rend_desc_changed();

	//! Action performed when a keyframe is selected in the widget
	//! This is where keyframe selected signal is fired
	void on_selection_changed();

	//! Signal handler for select keyframe signal from canvas interface
	void on_keyframe_selected(synfig::Keyframe);
	sigc::connection	keyframeselected;

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	KeyframeTree();
	~KeyframeTree();
	//! Assign the model and connect signals from the canvas interface
	void set_model(Glib::RefPtr<KeyframeTreeStore> keyframe_tree_store_);

	void set_editable(bool x=true);

	bool get_editable()const { return editable_; }

	//! Signal called when a keyframe has been edited in any way
	sigc::signal<void,synfig::Keyframe>& signal_edited() { return signal_edited_; }

	//! Signal called when a time has been edited.
	sigc::signal<void,synfig::Keyframe,synfig::Time>& signal_edited_time() { return signal_edited_time_; }

	//! Signal called when a description has been edited.
	sigc::signal<void,synfig::Keyframe,synfig::String>& signal_edited_description() { return signal_edited_description_; }

	sigc::signal<void,int, Gtk::TreeRow, ColumnID>& signal_user_click() { return signal_user_click_; }

	/*
 -- ** -- P R O T E C T E D   M E T H O D S ---------------------------------------
	*/

	protected:

}; // END of KeyframeTree

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
