/* === S I N F G =========================================================== */
/*!	\file dialog_soundselect.h
**	\brief Sound Select Header
**
**	$Id: dialog_soundselect.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_DIALOG_SOUNDSELECT_H
#define __SINFG_DIALOG_SOUNDSELECT_H

/* === H E A D E R S ======================================================= */
#include "dockdialog.h"
#include "widget_filename.h"
#include "widget_time.h"

#include <sinfgapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

struct AudioBaseInfo
{
	std::string		file;
	sinfg::Time		offset;
};
	
class Dialog_SoundSelect : public Gtk::Dialog
{
	Widget_Filename		soundfile;
	Widget_Time			offset;
	Gtk::Button			okbutton;
	
	etl::handle<sinfgapp::CanvasInterface> canvas_interface;
	
	sigc::signal<void,const std::string &>	signal_file_changed_;
	sigc::signal<void,const sinfg::Time &>	signal_offset_changed_;
	
	void on_file();
	void on_offset();
	void on_ok();
	
public:
	Dialog_SoundSelect(Gtk::Window &parent,etl::handle<sinfgapp::CanvasInterface> ci );
	~Dialog_SoundSelect();

	//float get_global_fps() const { return globalfps; }
	void set_global_fps(float f);

	sinfg::Time get_offset() const { return offset.get_value(); }
	void set_offset(const sinfg::Time &t) {offset.set_value(t); }
	
	std::string get_file() const { return soundfile.get_value(); }
	void set_file(const std::string &f) {soundfile.set_value(f); }
	
	sigc::signal<void,const std::string &> &signal_file_changed() { return signal_file_changed_; }
	sigc::signal<void,const sinfg::Time &> &signal_offset_changed() { return signal_offset_changed_; }
};
	
}; // END of namespace studio

/* === E N D =============================================================== */

#endif
