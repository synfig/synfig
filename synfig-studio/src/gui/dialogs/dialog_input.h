/* === S Y N F I G ========================================================= */
/*!	\file dialog_input.h
**	\brief Input dialog class
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_STUDIO_DIALOG_INPUT_H
#define __SYNFIG_STUDIO_DIALOG_INPUT_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/dialog.h>
#include <gui/dialogsettings.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk {
class ScrolledWindow;
}

namespace studio {

class Dialog_Input : public Gtk::Dialog
{
private:
	struct DeviceOptions;
	DialogSettings dialog_settings;

	sigc::signal<void> signal_apply_;
	DeviceOptions *options;

	Gtk::ScrolledWindow *scrolled_window;

	void take_options();
	void create_widgets();

public:
	Dialog_Input(Gtk::Window& parent);
	~Dialog_Input();

	void reset();
	void apply();
	void apply_and_hide();

	sigc::signal<void>& signal_apply() { return signal_apply_; }

protected:
	virtual void on_response(int id);
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
