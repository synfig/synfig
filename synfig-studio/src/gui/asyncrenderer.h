/* === S Y N F I G ========================================================= */
/*!	\file asyncrenderer.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_ASYNCRENDERER_H
#define __SYNFIG_ASYNCRENDERER_H

/* === H E A D E R S ======================================================= */

#include <ctime>

#include <ETL/handle>
#include <synfig/target_scanline.h>
#include <synfig/target_tile.h>
#include <synfig/target_cairo.h>
#include <synfig/target_cairo_tile.h>
#include <synfig/surface.h>
#include <glibmm/main.h>
#include <ETL/ref_count>
#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>

#include "app.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class AsyncRenderer : public etl::shared_object, public sigc::trackable
{
	//! Signal emitted when target has been stopped or has finished
	sigc::signal<void> signal_finished_;
	//! Signal emitted when target has succeeded
	sigc::signal<void> signal_success_;

	//! Seems to be unused
	std::list<sigc::connection> activity_connection_list;
	
	//! The target that is going to be asynchronously rendered.
	etl::handle<synfig::Target> target;

	//! Set to true when target render fails.
	bool error;
	//! Set to true when target render succeeded
	bool success;

	synfig::ProgressCallback *cb;
	//! Signal to be emitted when the target is requested to stop
	sigc::signal<void> signal_stop_;

	Glib::Thread* render_thread;
	Glib::Dispatcher signal_done_;
	Glib::Mutex mutex;
	sigc::connection done_connection;

	clock_t start_clock;
	clock_t finish_clock;
	Glib::TimeVal start_time;
	Glib::TimeVal finish_time;

	/*
 --	** -- P A R E N T   M E M B E R S -----------------------------------------
	*/
public:

	AsyncRenderer(etl::handle<synfig::Target> target,synfig::ProgressCallback *cb=0);
	virtual ~AsyncRenderer();

	void start();
	void stop();
	void pause();
	void resume();
#ifdef SINGLE_THREADED
	void rendering_progress();
	bool updating;
#endif

	bool has_error()const { return error; }
	bool has_success()const { return success; }
	synfig::Real get_execution_time() const { return (finish_time - start_time).as_double(); }
	synfig::Real get_execution_clock() const { return (synfig::Real)(finish_clock - start_clock)/(synfig::Real)CLOCKS_PER_SEC; }

	sigc::signal<void>& signal_finished() { return signal_finished_; }
	sigc::signal<void>& signal_success() { return signal_success_; }

private:

	void render_target();
	void start_();

	/*
 --	** -- C H I L D   M E M B E R S -------------------------------------------
	*/

protected:

};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
