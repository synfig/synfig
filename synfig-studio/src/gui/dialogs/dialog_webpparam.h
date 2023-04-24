/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_webpparam.h
**	\brief FFmpegParam Dialog header
**
**	\legal
**	Copyright (c) 2010 Carlos López González
**	Copyright (c) 2015 Denis Zdorovtsov
**	Copyright (c) 2022 BobSynfig
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

#ifndef __SYNFIG_STUDIO_DIALOG_WEBPPARAM_H
#define __SYNFIG_STUDIO_DIALOG_WEBPPARAM_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/separator.h>

#include <gui/dialogs/dialog_targetparam.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dialog_WebpParam : public Dialog_TargetParam
{
public:
	Dialog_WebpParam(Gtk::Window &parent);
	~Dialog_WebpParam();

protected:
	virtual void init();
	virtual void write_tparam(synfig::TargetParam & tparam);

private:
	Gtk::Grid         *grid;

	Gtk::Label        *l_quality;
	Gtk::Label        *l_comp_lvl;
	Gtk::Label        *l_preset;

	Gtk::Separator    *separator1;
	Gtk::Separator    *separator2;
	Gtk::Separator    *separator3;
	Gtk::Entry        *preset_desc;

	Gtk::RadioButton  *rb_wanim;
	Gtk::RadioButton  *rb_webp;
	Gtk::RadioButton  *rb_lossless;
	Gtk::RadioButton  *rb_lossy;
	Gtk::Entry        *webp_desc;

	Gtk::CheckButton  *cb_loop;     //loop
	Gtk::SpinButton   *sb_quality;  //quality  qscale float  (0-100 def 75)
	Gtk::SpinButton   *sb_comp_lvl; //compression level int  (0-6   def  4)
	Gtk::ComboBoxText *cbt_preset;  //

	void on_preset_change();
	void on_lossy_lossless_toggled();
	void on_webp_wanim_toggled();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif //__SYNFIG_STUDIO_DIALOG_WEBPPARAM_H
