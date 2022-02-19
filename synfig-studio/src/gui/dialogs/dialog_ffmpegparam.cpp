/* === S Y N F I G ========================================================= */
/*!	\file dialogs/dialog_ffmpegparam.cpp
**	\brief Implementation for the FFmpegParam Dialog
**
**	\legal
**	Copyright (c) 2010 Carlos López González
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gui/dialogs/dialog_ffmpegparam.h>

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

static const char* CUSTOM_VCODEC_DESCRIPTION = _("Custom Video Codec");
static const char* CUSTOM_VCODEC = _("write your video codec here");

/// FFMPEG video-codec name -> user-readable name / description
static const std::map<const char*, const char*> known_video_codecs = {
	{"flv",              _("Flash Video (FLV) / Sorenson Spark / Sorenson H.263")},
	{"h263p",            _("H.263+ / H.263-1998 / H.263 version 2")},
	{"huffyuv",          _("Huffyuv / HuffYUV")},
	{"libtheora",        _("libtheora Theora")},
	{"libx264",          _("H.264 / AVC / MPEG-4 AVC")},
	{"libx264-lossless", _("H.264 / AVC / MPEG-4 AVC (LossLess)")},
	{"mjpeg",            _("MJPEG (Motion JPEG)")},
	{"mpeg1video",       _("raw MPEG-1 video")},
	{"mpeg2video",       _("raw MPEG-2 video")},
	{"mpeg4",            _("MPEG-4 part 2. (XviD/DivX)")},
	{"msmpeg4",          _("MPEG-4 part 2 Microsoft variant version 3")},
	{"msmpeg4v1",        _("MPEG-4 part 2 Microsoft variant version 1")},
	{"msmpeg4v2",        _("MPEG-4 part 2 Microsoft variant version 2")},
	{"wmv1",             _("Windows Media Video 7")},
	{"wmv2",             _("Windows Media Video 8")},
};

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_FFmpegParam::Dialog_FFmpegParam(Gtk::Window &parent):
	Dialog_TargetParam(parent, _("FFmpeg parameters"))
{
	this->set_resizable(false);
	// Custom Video Codec Entry
	Gtk::Label* custom_label(manage(new Gtk::Label(std::string(CUSTOM_VCODEC_DESCRIPTION)+":")));
	custom_label->set_halign(Gtk::ALIGN_START);
	custom_label->set_valign(Gtk::ALIGN_CENTER);
	customvcodec=Gtk::manage(new Gtk::Entry());
	customvcodec->set_placeholder_text(CUSTOM_VCODEC);
	// Available Video Codecs Combo Box Text.
	Gtk::Label* label(manage(new Gtk::Label(_("Available Video Codecs:"))));
	label->set_halign(Gtk::ALIGN_START);
	label->set_valign(Gtk::ALIGN_CENTER);
	vcodec = Gtk::manage(new Gtk::ComboBoxText());
	// Appends the codec descriptions to the Combo Box
	for (const auto& item : known_video_codecs)
		vcodec->append(item.first, item.second);
	vcodec->append(CUSTOM_VCODEC, CUSTOM_VCODEC_DESCRIPTION);
	//Adds the Combo Box and the Custom Video Codec entry to the box
	get_content_area()->pack_start(*label, true, true, 0);
	get_content_area()->pack_start(*vcodec, true, true, 0);
	get_content_area()->pack_start(*custom_label, true, true, 0);
	get_content_area()->pack_start(*customvcodec, true, true, 0);

	// Connect the signal change to the handler
	vcodec->signal_changed().connect(sigc::mem_fun(*this, &Dialog_FFmpegParam::on_vcodec_change));

	//Bitrate Spin Button
	bitrate = Gtk::manage(new Gtk::SpinButton(Gtk::Adjustment::create(0.0, 10.0,100000.0)));
	Gtk::Label* label2(manage(new Gtk::Label(_("Video Bit Rate:"))));
	label2->set_halign(Gtk::ALIGN_START);
	label2->set_valign(Gtk::ALIGN_CENTER);
	get_content_area()->pack_start(*label2, true, true, 0);
	get_content_area()->pack_start(*bitrate,true, true, 0);

	get_content_area()->show_all();
}

Dialog_FFmpegParam::~Dialog_FFmpegParam()
{
}

void
Dialog_FFmpegParam::init()
{
	// By default, set the active text to the Custom Video Codec
	vcodec->set_active_id(CUSTOM_VCODEC);
	customvcodec->set_text(get_tparam().video_codec);
	//Compare the passed vcodec to the available and set it active if found
	for (const auto& item : known_video_codecs) {
		if(get_tparam().video_codec.compare(item.first) == 0)
		{
			vcodec->set_active_id(item.first);
			customvcodec->set_text(item.first);
			break;
		}
	}
	//Bitrate
	bitrate->set_value(double(get_tparam().bitrate));
}

void
Dialog_FFmpegParam::write_tparam(synfig::TargetParam & tparam_)
{
	tparam_.video_codec = customvcodec->get_text();
	tparam_.bitrate = bitrate->get_value();
}

void
Dialog_FFmpegParam::on_vcodec_change()
{
	std::string codecname = vcodec->get_active_id();
	bool is_custom_codec = codecname == CUSTOM_VCODEC;
	customvcodec->set_sensitive(is_custom_codec);
	if (!is_custom_codec)
		customvcodec->set_text(codecname);
}
