/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_fontfamily.cpp
**	\brief Widget to select font family
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2020 Rodolfo Ribeiro Gomes
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

#include <gui/widgets/widget_fontfamily.h>

#ifdef WITH_FONTCONFIG
#include <fontconfig/fontconfig.h>
#include <mutex>
#include <glibmm.h>
#endif

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Glib::RefPtr<Gtk::ListStore> Widget_FontFamily::enum_TreeModel;

Widget_FontFamily::Model::Model()
{
	add(value);
}

#ifdef WITH_FONTCONFIG
struct FamilyFontConfigWrap {
	static const std::vector<std::string>& get() {
		static FamilyFontConfigWrap obj;

		return obj.get_families();
	}

	FamilyFontConfigWrap(FamilyFontConfigWrap const&) = delete;
	void operator=(FamilyFontConfigWrap const&) = delete;

private:
	std::vector<std::string> families;
	mutable std::mutex fam_mutex;

	FamilyFontConfigWrap()
	{
		refresh();
	}

	const std::vector<std::string>& get_families() const {
		std::lock_guard<std::mutex> lock(fam_mutex);
		return families;
	}

	void refresh()
	{
		std::lock_guard<std::mutex> lock(fam_mutex);
		families.clear();
		FcConfig* config = FcInitLoadConfigAndFonts();
		if (!config)
			return;
#ifdef _WIN32
		// Windows 10 (1809) Added local user fonts installed to C:\Users\%USERNAME%\AppData\Local\Microsoft\Windows\Fonts
		std::string localdir = Glib::getenv("LOCALAPPDATA");
		if (!localdir.empty()) {
			localdir.append("\\Microsoft\\Windows\\Fonts\\");
			FcConfigAppFontAddDir(config, (const FcChar8 *)localdir.c_str());
		}
#endif

		FcPattern* pattern = FcPatternCreate();
		if (pattern) {
			FcObjectSet* os = FcObjectSetBuild (FC_FAMILY, nullptr);
			if (os) {
				FcFontSet* font_set = FcFontList(config, pattern, os);
				if (font_set) {
					families.resize(font_set->nfont);
					for (int i=0, n_font = font_set->nfont; font_set && i < n_font; ++i) {
						FcPattern* font = font_set->fonts[i];
						FcChar8 *family;

						if (FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch) {
							families.push_back((char*)family);
//							FcStrFree(family); // it crashes synfig if we do free. No memory leaks detected via valgrind though
						}
//						FcPatternDestroy(font); // it crashes synfig if we do free (MacOS 10.9 - MacOS 10.14 (Mojave)). No memory leaks detected via valgrind though
					}
					FcFontSetDestroy(font_set);

					std::sort( families.begin(), families.end() );
					families.erase( std::unique( families.begin(), families.end() ), families.end() );
				}
				FcObjectSetDestroy(os);
			}
			FcPatternDestroy(pattern);
		}
		FcConfigDestroy(config);
	}
};
#endif

Widget_FontFamily::Widget_FontFamily()
	: Gtk::ComboBoxText(true)
{
	if (!enum_TreeModel) {
		enum_TreeModel = Gtk::ListStore::create(enum_model);

#ifdef WITH_FONTCONFIG
	const std::vector<std::string>& families = FamilyFontConfigWrap::get();
	for (const std::string &family : families) {
		Gtk::TreeModel::Row row = *(enum_TreeModel->append());
		row[enum_model.value] = family;
	}
#endif
	}

	set_model(enum_TreeModel);
	set_wrap_width(1); // https://github.com/synfig/synfig/issues/650

	if (get_has_entry())
		static_cast<Gtk::Entry*>(get_child())->signal_activate().connect([=](){ signal_activate().emit(); });
	signal_changed().connect([=]() {
		if (!get_has_entry() || get_active_row_number() != -1)
			signal_activate().emit();
	});
}

Widget_FontFamily::~Widget_FontFamily()
{
}

void
Widget_FontFamily::set_value(const std::string& data)
{
	value=data;
	set_active_text(data);
	if (get_active_row_number() == -1)
		if (get_has_entry())
			static_cast<Gtk::Entry*>(get_child())->set_text(data);
}

std::string
Widget_FontFamily::get_value() const
{
	if (get_active_row_number() == -1)
		if (get_has_entry())
			return static_cast<const Gtk::Entry*>(get_child())->get_text();
	return value;
}

void
Widget_FontFamily::on_changed()
{
	Gtk::TreeModel::iterator iter = get_active();
	if(iter)
	{
		Gtk::TreeModel::Row row = *iter;
		value = row.get_value(enum_model.value);
	}
}
