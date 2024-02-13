/* === S Y N F I G ========================================================= */
/*!	\file mptr_magickpp.cpp
**	\brief Magick++ Importer (magickpp_mptr)
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2024      Synfig Contributors
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
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mptr_magickpp.h"

#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/os.h>

#include <Magick++.h>

#endif

/* === M A C R O S ========================================================= */

using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_IMPORTER_INIT(magickpp_mptr);
SYNFIG_IMPORTER_SET_NAME(magickpp_mptr,"magick++");
SYNFIG_IMPORTER_SET_EXT(magickpp_mptr,"svg");
SYNFIG_IMPORTER_SET_VERSION(magickpp_mptr,"0.1");
SYNFIG_IMPORTER_SET_SUPPORTS_FILE_SYSTEM_WRAPPER(magickpp_mptr, false);

/* === M E T H O D S ======================================================= */

magickpp_mptr::magickpp_mptr(const synfig::FileSystem::Identifier& identifier)
	: synfig::Importer(identifier), animation_repetitions_(0)
{
	Magick::InitializeMagick(synfig::OS::get_binary_path().u8_str());

	try {
		std::string filename = identifier.file_system->get_real_filename(identifier.filename.u8string());
		Magick::Image image;
		image.ping(filename+"[-1]");
		auto n_frames = image.scene() + 1;
		if (n_frames <= 1) {
			animation_repetitions_ = 0;
		} else {
			frame_time_list_.resize(n_frames);
			synfig::Time current_time;
			for (size_t i = 0; i < n_frames; ++i) {
				image.ping(strprintf("%s[%zu]", filename.c_str(), i));
				frame_time_list_[i] = current_time;
				current_time += image.animationDelay() * 0.01; // The delay is in multiple of 10ms
			}
			animation_length_ = current_time;
			animation_repetitions_ = image.animationIterations();
		}
	} catch (Magick::Error& err) {
		// because 'Error' is derived from the standard C++ exception, it has a 'what()' method
		synfig::error(_("Magick++ importer: error occurred reading a file: %s"), err.what());
	} catch (...) {
		synfig::error(_("Magick++ importer: an unhandled error has occurred on reading file %s"), identifier.filename.u8_str());
	}
}

magickpp_mptr::~magickpp_mptr()
{
}

bool
magickpp_mptr::is_animated()
{
	return frame_time_list_.size() > 0;
}

bool
magickpp_mptr::get_frame(synfig::Surface& surface, const synfig::RendDesc& /*renddesc*/, synfig::Time time, synfig::ProgressCallback* /*callback*/)
{
	const std::string filename = identifier.file_system->get_real_filename(identifier.filename.u8string());

	try {
		Magick::Image image;
		if (!is_animated()) {
			image.read(filename);
		} else {
			int repetition_n = 0;
			while (time >= animation_length_) {
				++repetition_n;
				time -= animation_length_;
			}

			auto frame_index = frame_time_list_.size() - 1;
			if (animation_repetitions_ == 0 || repetition_n < animation_repetitions_) {
				for (frame_index = frame_time_list_.size() - 1; frame_index > 0; --frame_index) {
					if (time >= frame_time_list_[frame_index])
						break;
				}
			}

			image.read(strprintf("%s[%zu]", filename.c_str(), frame_index));
		}
		const auto width = image.size().width();
		const auto height = image.size().height();
		surface.set_wh(width, height);

#if MagickLibVersion >= 0x701 // MAGICKCORE_CHECK_VERSION(7,0,0) does not work!
		const bool has_alpha = image.alpha();
		constexpr MagickCore::ImageType TrueColorAlphaType = Magick::TrueColorAlphaType;
#else
		const bool has_alpha = image.matte();
		constexpr MagickCore::ImageType TrueColorAlphaType = Magick::TrueColorMatteType;
#endif

		if (has_alpha)
			image.type(TrueColorAlphaType);
		else
			image.type(Magick::TrueColorType);

		const auto packet = image.getConstPixels(0, 0, width, height);

		if (!packet) {
			synfig::error(_("Magick++ importer: couldn't get pixel packet"));
			return false;
		}
// Sadly Magick++ API is a mess with namespaces...
{
	using namespace Magick;
	using namespace MagickCore;


		constexpr synfig::Color::value_type factor = QuantumRange;

		for (size_t y = 0, i = 0; y < height; ++y) {
			for (size_t x = 0; x < width; ++x) {
#if MagickLibVersion >= 0x701 // MAGICKCORE_CHECK_VERSION(7,0,0) does not work!
				if (has_alpha) {
					surface[y][x] = synfig::Color(packet[i] / factor, packet[i+1] / factor, packet[i+2] / factor, packet[i+3] / factor);
					i += 4;
				} else {
					surface[y][x] = synfig::Color(packet[i] / factor, packet[i+1] / factor, packet[i+2] / factor, 1.);
					i += 3;
				}
#else
				const auto& color = packet[i++];
				if (has_alpha)
					surface[y][x] = synfig::Color(color.red / factor, color.green / factor, color.blue / factor, 1. - color.opacity / factor);
				else
					surface[y][x] = synfig::Color(color.red / factor, color.green / factor, color.blue / factor, 1.);
#endif
			}
		}
} // end Magick++ namespace mess
	} catch (Magick::Error& err) {
		synfig::error(_("Magick++ importer: error occurred fetching pixels: %s"), err.what());
		return false;
	} catch (...) {
		synfig::error(_("Magick++ importer: an unhandled error has occurred on fetching pixels from file %s"), identifier.filename.u8_str());
		return false;
	}

	return true;
}
