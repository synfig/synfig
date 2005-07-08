/* === S Y N F I G ========================================================= */
/*!	\file asyncrenderer.h
**	\brief Template Header
**
**	$Id: asyncrenderer.h,v 1.3 2005/01/12 07:03:42 darco Exp $
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

#ifndef __SYNFIG_ASYNCRENDERER_H
#define __SYNFIG_ASYNCRENDERER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>

#include <synfig/target_scanline.h>
#include <synfig/target_tile.h>
#include <synfig/surface.h>
#include <glibmm/main.h>
#include <ETL/ref_count>
#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class AsyncRenderer : public etl::shared_object, public sigc::trackable
{
	sigc::signal<void> signal_finished_;
	sigc::signal<void> signal_success_;

	std::list<sigc::connection> activity_connection_list;
	
	//etl::handle<synfig::Target_Scanline> target_scanline;
	//etl::handle<synfig::Target_Tile> target_tile;
	etl::handle<synfig::Target> target;

	bool error;
	bool success;
	
	synfig::ProgressCallback *cb;
	
	sigc::signal<void> signal_stop_;

	Glib::Thread* render_thread;
	Glib::Dispatcher signal_done_;
	Glib::Mutex mutex;
	sigc::connection done_connection;

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

	bool has_error()const { return error; }
	bool has_success()const { return success; }

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
