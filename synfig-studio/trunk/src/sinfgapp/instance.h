/* === S I N F G =========================================================== */
/*!	\file instance.h
**	\brief writeme
**
**	$Id: instance.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_APP_INSTANCE_H
#define __SINFG_APP_INSTANCE_H

/* === H E A D E R S ======================================================= */

#include "action.h"
#include <ETL/handle>
#include <sinfg/canvas.h>
#include <sinfg/string.h>
#include <list>
#include <sigc++/signal.h>
#include <sigc++/object.h>
#include "action_system.h"
#include "selectionmanager.h"
#include "cvs.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class CanvasInterface;


class Instance : public Action::System , public CVSInfo
{
	friend class PassiveGrouper;
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	typedef std::list< etl::handle<CanvasInterface> > CanvasInterfaceList;

	using etl::shared_object::ref;
	using etl::shared_object::unref;

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:
	
	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:
	//! Handle for root canvas
	sinfg::Canvas::Handle canvas_;


	CanvasInterfaceList canvas_interface_list_;

	sigc::signal<void> signal_filename_changed_;
	sigc::signal<void> signal_saved_;
	etl::handle<SelectionManager> selection_manager_;
	
protected:
	Instance(etl::handle<sinfg::Canvas>);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	~Instance();

	void set_selection_manager(const etl::handle<SelectionManager> &sm) { assert(sm); selection_manager_=sm; }
	void unset_selection_manager() { selection_manager_=new NullSelectionManager(); }
	const etl::handle<SelectionManager> &get_selection_manager() { return selection_manager_; }	



	etl::handle<CanvasInterface> find_canvas_interface(sinfg::Canvas::Handle canvas);

	sinfg::Canvas::Handle get_canvas()const { return canvas_; }

	//! Saves the instance to filename_
	bool save()const;

	bool save_as(const sinfg::String &filename)const;

	bool save_as(const sinfg::String &filename);

public:	// Interfaces to internal information
	sigc::signal<void>& signal_filename_changed() { return signal_filename_changed_; }
	sigc::signal<void>& signal_saved() { return signal_saved_; }

	CanvasInterfaceList & canvas_interface_list() { return canvas_interface_list_; }
	const CanvasInterfaceList & canvas_interface_list()const { return canvas_interface_list_; }

	sinfg::String get_file_name()const;

	void set_file_name(const sinfg::String &name);

public:
	

public:	// Constructor interfaces
	static etl::handle<Instance> create(etl::handle<sinfg::Canvas> canvas);
}; // END class Instance

etl::handle<Instance> find_instance(etl::handle<sinfg::Canvas> canvas);

bool is_editable(sinfg::ValueNode::Handle value_node);

}; // END namespace studio

/* === E N D =============================================================== */

#endif
