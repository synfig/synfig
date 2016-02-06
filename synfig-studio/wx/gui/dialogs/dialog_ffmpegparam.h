/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_ffmpegparam.h
**	\brief FFmpegParam Dialog header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2010 Carlos López González
**	Copyright (c) 2015 Denis Zdorovtsov
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

#ifndef __SYNFIG_STUDIO_DIALOG_FFMPEGPARAM_H
#define __SYNFIG_STUDIO_DIALOG_FFMPEGPARAM_H

/* === H E A D E R S ======================================================= */
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>

#include <synfig/targetparam.h>
#include "dialogs/dialog_targetparam.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Dialog_FFmpegParam : public Dialog_TargetParam
{
public:
	Dialog_FFmpegParam(Gtk::Window &parent);
	~Dialog_FFmpegParam();

protected:
	virtual void init();
	virtual void write_tparam(synfig::TargetParam & tparam); 

private:
	Gtk::SpinButton *bitrate;
	Gtk::ComboBoxText *vcodec;
	Gtk::Entry *customvcodec;

	void on_vcodec_change();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif //__SYNFIG_STUDIO_DIALOG_FFMPEGPARAM_H
