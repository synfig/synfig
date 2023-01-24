/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_spritesheetparam.h
**	\brief SpriteSheetParam
**
**	\legal
**	Copyright (c) 2015 Denis Zdorovtsov
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

#ifndef __SYNFIG_STUDIO_DIALOG_SPRITESHEETPARAM_H
#define __SYNFIG_STUDIO_DIALOG_SPRITESHEETPARAM_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>

#include <gui/dialogs/dialog_targetparam.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
class Dialog_SpriteSheetParam: public Dialog_TargetParam
{
	public:
		Dialog_SpriteSheetParam(Gtk::Window &parent);
		~Dialog_SpriteSheetParam();

protected:
	virtual void init();
	virtual void write_tparam(synfig::TargetParam & tparam); 

private:
	Gtk::SpinButton * offset_x_box;
	Gtk::SpinButton * offset_y_box;
	Gtk::SpinButton * rows_box;
	Gtk::SpinButton * columns_box;
	Gtk::CheckButton * check_button;
	Gtk::ComboBoxText * direction_box;

	int frame_count;
	
	void on_dir_change();
	void on_rows_change();
	void on_cols_change();
};

}; //studio

/* === E N D =============================================================== */

#endif 
