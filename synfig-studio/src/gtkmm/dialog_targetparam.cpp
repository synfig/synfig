/* === S Y N F I G ========================================================= */
/*!	\file dialog_targetparam.cpp
**	\brief Implementation for the TargetParam Dialog
**
**	$Id$
**
**	\legal
**	Copyright (c) 2010 Carlos López González
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "dialog_targetparam.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Dialog_TargetParam::Dialog_TargetParam(synfig::TargetParam &tparam)
{
	set_title("TargetParam Dialog");
	set_tparam(tparam);
	// Available Video Codecs Combo Box Text.
	vcodec = Gtk::manage(new Gtk::ComboBoxText());
	Gtk::Label* label(manage(new Gtk::Label(_("Available Video Codecs:"))));
	label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
	get_vbox()->pack_start(*label, true, true, 0);
	vcodec->append_text("flv");
	vcodec->append_text("h263p");
	vcodec->append_text("huffyuv");
	vcodec->append_text("libtheora");
	vcodec->append_text("libx264");
	vcodec->append_text("libxvid");
	vcodec->append_text("mjpeg");
	vcodec->append_text("mpeg2video");
	vcodec->append_text("mpeg4");
	vcodec->append_text("msmpeg4");
	vcodec->append_text("msmpeg4v1");
	vcodec->append_text("msmpeg4v2");
	vcodec->append_text("wmv1");
	vcodec->append_text("wmv2");
	vcodec->set_active_text(get_tparam().video_codec);
	get_vbox()->pack_start(*vcodec, true, true, 0);

	//Bitrate Spin Button
	Gtk::Adjustment* bradj(manage(new class Gtk::Adjustment(double(tparam.bitrate), 10.0,1000.0)));
	bitrate = Gtk::manage(new class Gtk::SpinButton(*bradj));
	Gtk::Label* label2(manage(new Gtk::Label(_("Video Bit Rate:"))));
	label2->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
	get_vbox()->pack_start(*label2, true, true, 0);
	get_vbox()->pack_start(*bitrate,true, true, 0);

	get_vbox()->show_all();

	ok_button = manage(new class Gtk::Button(Gtk::StockID("gtk-ok")));
	ok_button->show();
	add_action_widget(*ok_button,Gtk::RESPONSE_OK);
	ok_button->signal_clicked().connect(sigc::mem_fun(*this,&Dialog_TargetParam::on_ok));

	cancel_button = manage(new class Gtk::Button(Gtk::StockID("gtk-cancel")));
	cancel_button->show();
	add_action_widget(*cancel_button,Gtk::RESPONSE_CANCEL);
	cancel_button->signal_clicked().connect(sigc::mem_fun(*this,&Dialog_TargetParam::on_cancel));

}

void
Dialog_TargetParam::on_ok()
{
	tparam_.video_codec=vcodec->get_active_text();
	tparam_.bitrate=bitrate->get_value();
	hide();
}

void
Dialog_TargetParam::on_cancel()
{
	hide();
}

Dialog_TargetParam::~Dialog_TargetParam()
{
}

