/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_webpparam.cpp
**	\brief Implementation for the WebpParam Dialog
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
//https://developers.google.com/speed/webp/docs/cwebp
//http://underpop.online.fr/f/ffmpeg/help/libwebp.htm.gz
/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/dialogs/dialog_webpparam.h>
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

// Presets   name/id,  description
static const std::map<const char*, const char*> preset_types = {
	{"none",    _("Do not use a preset."                            )},
	{"default", _("Use the encoder default."                        )},
	{"picture", _("Digital picture, like portrait, inner shot"      )},
	{"photo",   _("Outdoor photograph, with natural lighting"       )},
	{"drawing", _("Hand or line drawing, with high-contrast details")},
	{"icon",    _("Small-sized colorful images"                     )},
	{"text",    _("Text-like"                                       )}
};

// Codec     name/id,  description
static const std::map<const char*, const char*> codec_types = {
	{"libwebp_anim",  _("Better for transparent animations"         )},
	{"libwebp",       _("Standard"                                  )}
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_WebpParam::Dialog_WebpParam(Gtk::Window &parent):
	Dialog_TargetParam(parent, _("Webp Parameters"))
{
	this->set_resizable(false);

	//<!-- Video codec
	rb_wanim = Gtk::manage( new Gtk::RadioButton( "libwebp_anim" ) );
	rb_wanim->set_active();
	rb_wanim->signal_toggled().connect(
	        sigc::mem_fun( *this, &Dialog_WebpParam::on_webp_wanim_toggled )
	        );

	rb_webp  = Gtk::manage( new Gtk::RadioButton( "libwebp"      ) );
	rb_webp->join_group(*rb_wanim);
	rb_webp->signal_toggled().connect(
	        sigc::mem_fun( *this, &Dialog_WebpParam::on_webp_wanim_toggled )
	        );

	webp_desc = Gtk::manage( new Gtk::Entry() );
	webp_desc->set_placeholder_text( _("Codec description") );
	webp_desc->set_sensitive(false);

	on_webp_wanim_toggled(); //Initialize
	//->  Video codec

	rb_lossless = Gtk::manage( new Gtk::RadioButton( _("Lossless") ) );
	rb_lossless->set_active();
	rb_lossless->signal_toggled().connect(
	        sigc::mem_fun( *this, &Dialog_WebpParam::on_lossy_lossless_toggled )
	        );

	rb_lossy    = Gtk::manage( new Gtk::RadioButton( _("Lossy"   ) ) );
	rb_lossy->join_group(*rb_lossless);
	rb_lossy->signal_toggled().connect(
	        sigc::mem_fun( *this, &Dialog_WebpParam::on_lossy_lossless_toggled )
	        );

	//Checkbox
	cb_loop     = Gtk::manage( new Gtk::CheckButton( _("Loop animation"), true) );

	//Quality
	l_quality   = Gtk::manage( new Gtk::Label( _("Quality (0-100):") ) );
	l_quality->set_halign( Gtk::ALIGN_START  );
	l_quality->set_valign( Gtk::ALIGN_CENTER );

	sb_quality  = Gtk::manage( new Gtk::SpinButton( Gtk::Adjustment::create( 75.0, 0.0,100.0 ) ) );
	sb_quality->set_alignment(1.0);

	//Compression Level
	l_comp_lvl  = Gtk::manage( new Gtk::Label( _("Compression Level (0-6):" ) ) );
	l_comp_lvl->set_halign( Gtk::ALIGN_START  );
	l_comp_lvl->set_valign( Gtk::ALIGN_CENTER );

	sb_comp_lvl = Gtk::manage( new Gtk::SpinButton(Gtk::Adjustment::create(4, 0, 6)) );
	sb_comp_lvl->set_alignment(1.0);

	//Preset
	l_preset    = Gtk::manage( new Gtk::Label( _("Preset:") ) );
	l_preset->set_halign( Gtk::ALIGN_START  );
	l_preset->set_valign( Gtk::ALIGN_CENTER );

	preset_desc = Gtk::manage( new Gtk::Entry() );
	preset_desc->set_placeholder_text( _("Preset Description") );
	preset_desc->set_sensitive(false);

	cbt_preset  = Gtk::manage( new Gtk::ComboBoxText() );
	// Appends the preset id and display to the Combo Box
	for (const auto& item : preset_types)
		cbt_preset->append( item.first, item.first ); //Not a mistake!

	cbt_preset->signal_changed().connect(
	        sigc::mem_fun( *this, &Dialog_WebpParam::on_preset_change )
	        );

	separator1 = Gtk::manage( new Gtk::Separator() );
	separator2 = Gtk::manage( new Gtk::Separator() );
	separator3 = Gtk::manage( new Gtk::Separator() );

	//Grid
	grid       = Gtk::manage( new Gtk::Grid() );

	//It is easier to use a row index instead of modify row in grid in case
	//of change of layout
	int row = 0;
	//                           X,    Y,  W,  H
	grid->attach( *rb_wanim,     0,  row,  2,  1 );
	grid->attach( *rb_webp,      2,  row,  2,  1 );
	row++;
	grid->attach( *webp_desc,    0,  row,  5,  1 );
	row++;

	grid->attach( *separator1,   0,  row,  5,  1 );
	row++;

	grid->attach( *rb_lossless,  0,  row,  2,  1 );
	grid->attach( *rb_lossy,     2,  row,  2,  1 );
	row++;

	grid->attach( *l_quality,    0,  row,  2,  1 );
	grid->attach( *sb_quality,   3,  row,  2,  1 );
	row++;

	grid->attach( *l_comp_lvl,   0,  row,  2,  1 );
	grid->attach( *sb_comp_lvl,  3,  row,  2,  1 );
	row++;

	grid->attach( *separator2,   0,  row,  5,  1 );
	row++;

	grid->attach( *l_preset,     0,  row,  5,  1 );
	row++;

	grid->attach( *cbt_preset,   0,  row,  5,  1 );
	row++;

	grid->attach( *preset_desc,  0,  row,  5,  1 );
	row++;

	grid->attach( *separator3,   0,  row,  5,  1 );
	row++;

	grid->attach( *cb_loop,      0,  row,  5,  1 );
	row++;

	grid->set_row_spacing   (4);
	grid->set_column_spacing(2);
	grid->set_border_width  (8);
	grid->show_all();

	get_content_area()->pack_start( *grid, true, true, 0 );
	get_content_area()->show_all();
}

Dialog_WebpParam::~Dialog_WebpParam()
{
}

void
Dialog_WebpParam::init()
{
	// By default, set the active text to the preset
	cbt_preset->set_active_id( get_tparam().preset );
	for (const auto& item : preset_types) {
		if ( cbt_preset->get_active_id() == item.first  ) {
			cbt_preset ->set_active_id( item.first  );
			preset_desc->set_text     ( item.second );
			break;
		}
	}

	sb_comp_lvl->set_value (   int( get_tparam().comp_lvl ));
	cb_loop    ->set_active(  bool( get_tparam().loop     ));
	rb_lossless->set_active(  bool( get_tparam().lossless ));
	rb_lossy   ->set_active(  bool(!get_tparam().lossless ));
	sb_quality ->set_value ( float( get_tparam().quality  ));

	//By default video_codec could be something else, like mpeg4
	bool is_anim = !("libwebp" == get_tparam().video_codec);
	rb_wanim   ->set_active(  is_anim );
	rb_webp    ->set_active( !is_anim );


}

void
Dialog_WebpParam::write_tparam( synfig::TargetParam & tparam_ )
{
	tparam_.comp_lvl = sb_comp_lvl->get_value();
	tparam_.lossless = rb_lossless->get_active();
	tparam_.loop     = cb_loop    ->get_active();
	tparam_.preset   = cbt_preset ->get_active_id();
	tparam_.quality  = sb_quality ->get_value();

	tparam_.video_codec = rb_wanim->get_active()
	                    ? "libwebp_anim"
	                    : "libwebp";
}

void
Dialog_WebpParam::on_preset_change()
{
	for (const auto& item : preset_types) {
		if ( cbt_preset->get_active_id() == item.first ) {
			cbt_preset ->set_active_id( item.first  );
			preset_desc->set_text     ( item.second );
			break;
		}
	}
}

void
Dialog_WebpParam::on_webp_wanim_toggled()
{
	std::string current_codec = rb_wanim->get_active()
	                          ? "libwebp_anim"
	                          : "libwebp";

	for (const auto& item : codec_types) {
		if ( current_codec == item.first ) {
			webp_desc->set_text( item.second );
			break;
		}
	}
}

void
Dialog_WebpParam::on_lossy_lossless_toggled()
{
}
