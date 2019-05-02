/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_targetparam.h
**	\brief Targetparam Dialog Header
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

#ifndef __SYNFIG_STUDIO_DIALOG_TARGETPARAM_H
#define __SYNFIG_STUDIO_DIALOG_TARGETPARAM_H

/* === H E A D E R S ======================================================= */
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>

#include <synfig/targetparam.h>
#include <synfig/renddesc.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

///@brief Abstract parent for extra target parameters dialogs. 	
class Dialog_TargetParam: public Gtk::Dialog
{
public:
	Dialog_TargetParam(Gtk::Window &parent, const char* title);
	virtual ~Dialog_TargetParam(){};

	synfig::TargetParam get_tparam() const { return tparam_;}
	void set_tparam(const synfig::TargetParam &tp) {this->tparam_=tp; }
	void set_desc(const synfig::RendDesc & desc) {this->desc = desc;}
	int run();
	
protected:
	virtual void init() = 0;
	virtual void write_tparam(synfig::TargetParam & tparam_) = 0;
	const synfig::RendDesc & get_desc() {return desc;}
	
private:
	synfig::TargetParam tparam_;
	synfig::RendDesc desc;
	Gtk::Button *ok_button;
	Gtk::Button *cancel_button;
	
	void on_ok();
	void on_cancel();
};

}; // END of namespace studio

/* === E N D =============================================================== */

#endif



