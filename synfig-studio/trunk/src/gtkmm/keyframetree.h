/* === S Y N F I G ========================================================= */
/*!	\file keyframetree.h
**	\brief Template Header
**
**	$Id: keyframetree.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STUDIO_KEYFRAMETREE_H
#define __SYNFIG_STUDIO_KEYFRAMETREE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <synfigapp/canvasinterface.h>
#include <synfigapp/value_desc.h>
#include "keyframetreestore.h"
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

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:
	
	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_edited_time(const Glib::ustring&path_string,synfig::Time time);

	void on_edited_time_delta(const Glib::ustring&path_string,synfig::Time time);

	void on_edited_description(const Glib::ustring&path_string,const Glib::ustring &description);

	bool on_event(GdkEvent *event);

	void on_rend_desc_changed();

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	
	KeyframeTree();
	~KeyframeTree();

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
}; // END of KeyframeTree

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
