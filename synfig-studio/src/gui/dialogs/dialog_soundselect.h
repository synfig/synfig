/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_soundselect.h
**	\brief Sound Select Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 David Roden
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
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_DIALOG_SOUNDSELECT_H
#define __SYNFIG_DIALOG_SOUNDSELECT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>

#include <gui/widgets/widget_filename.h>
#include <gui/widgets/widget_time.h>

#include <synfigapp/canvasinterface.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

struct AudioBaseInfo
{
	std::string		file;
	synfig::Time		offset;
};

class Dialog_SoundSelect : public Gtk::Dialog
{
	Widget_Filename		soundfile;
	Widget_Time			offset;
	Gtk::Button			*okbutton;

	etl::handle<synfigapp::CanvasInterface> canvas_interface;

	sigc::signal<void,const std::string &>	signal_file_changed_;
	sigc::signal<void,const synfig::Time &>	signal_offset_changed_;

	void on_file();
	void on_offset();
	void on_ok();

public:
	Dialog_SoundSelect(Gtk::Window &parent,etl::handle<synfigapp::CanvasInterface> ci );
	~Dialog_SoundSelect();

	//float get_global_fps() const { return globalfps; }
	void set_global_fps(float f);

	synfig::Time get_offset() const { return offset.get_value(); }
	void set_offset(const synfig::Time &t) {offset.set_value(t); }

	std::string get_file() const { return soundfile.get_value(); }
	void set_file(const std::string &f) {soundfile.set_value(f); }

	sigc::signal<void,const std::string &> &signal_file_changed() { return signal_file_changed_; }
	sigc::signal<void,const synfig::Time &> &signal_offset_changed() { return signal_offset_changed_; }
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
