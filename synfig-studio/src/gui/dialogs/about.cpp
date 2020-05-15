/* === S Y N F I G ========================================================= */
/*!	\file about.cpp
**	\brief About dialog implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2008 Paul Wise
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

#include <vector>

#include <gtk/gtk.h>

#include <gtkmm/aboutdialog.h>

#include <ETL/stringf>

#include <synfig/general.h>

// This is generated at make time from .svn or .git/svn or autorevision.conf
#include <autorevision.h>

#include "about.h"
#include "app.h"

#include <gui/localization.h>

#include "gui/resourcehelper.h"

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef PACKAGE_NAME
#define  PACKAGE_NAME "Synfig Studio"
#endif

#ifndef VERSION
#define VERSION	"unknown"
#endif

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"png"
#endif

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

About::About()
{
	set_transient_for((Gtk::Window&)(*App::main_window));
	set_program_name(PACKAGE_NAME);
	set_version(VERSION);
	set_comments(_("2D vector animation studio"));

	set_website("https://synfig.org/");
	set_website_label(_("Visit the Synfig website"));

	set_copyright(_("Copyright (c) 2001-2020\nSynfig developers & contributors"));
	Glib::ustring license =
		"This program is free software; you can redistribute it and/or modify "
		"it under the terms of the GNU General Public License as published by "
		"the Free Software Foundation; either version 2 of the License, or "
		"(at your option) any later version.\n\n"

		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		"GNU General Public License for more details.\n\n"

		"You should have received a copy of the GNU General Public License along "
		"with this program; if not, write to the Free Software Foundation, Inc., "
		"51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or visit: http://www.gnu.org/";
	set_license(license);
	set_wrap_license(true);

	std::vector<Glib::ustring> authors;
	authors.push_back(_("Original developers:"));
	authors.push_back("");
	authors.push_back("Robert B. Quattlebaum Jr (darco)");
	authors.push_back("Adrian Bentley");
	authors.push_back("");
	authors.push_back(_("Contributors:"));
	authors.push_back("");
	authors.push_back("Adrian Winchell (SnapSilverlight)");
	authors.push_back("Andreas Jochens");
	authors.push_back("Brendon Higgins");
	authors.push_back("Carlos López González (genete)");
	authors.push_back("Carlos A. Sosa Navarro");
	authors.push_back("caryoscelus");
	authors.push_back("Chris Moore (dooglus)");
	authors.push_back("Chris Norman (pixelgeek)");
	authors.push_back("Cyril Brulebois (KiBi)");
	authors.push_back("Daniel Fort");
	authors.push_back("Daniel Hornung (rubikcube)");
	authors.push_back("David Roden (Bombe)");
	authors.push_back("Denis Zdorovtsov (trizer)");
	authors.push_back("Dmitriy Pomerantsev (Atrus)");
	authors.push_back("Douglas Lau");
	authors.push_back("Evgenij Katunov");
	authors.push_back("Gerald Young (Yoyobuae)");
	authors.push_back("Gerco Ballintijn");
	authors.push_back("IL'dar AKHmetgaleev (AkhIL)");
	authors.push_back("Ivan Mahonin");
	authors.push_back("Jerome Blanchi (d.j.a.y.)");
	authors.push_back("Konstantin Dmitriev (zelgadis)");
	authors.push_back("Luka Pravica");
	authors.push_back("Nikita Kitaev (nikitakit)");
	authors.push_back("Martin Michlmayr (tbm)");
	authors.push_back("Max May (Permutatrix)");
	authors.push_back("Miguel Gea Milvaques (xerakko)");
	authors.push_back("Paul Wise (pabs)");
	authors.push_back("Ralf Corsepius");
	authors.push_back("Ramon Miranda");
	authors.push_back("Ray Frederikson");
	authors.push_back("Timo Paulssen (timonator)");
	authors.push_back("Yu Chen (jcome)");
	authors.push_back("Yue Shi Lai");
	set_authors(authors);

	std::vector<Glib::ustring> artists;
	artists.push_back("Aurore D (rore)");
	artists.push_back("Bertrand Grégoire (berteh)");
	artists.push_back("Carl-Christian Gehl (Razputin)");
	artists.push_back("Carlos López González (genete)");
	artists.push_back("Chris Norman (pixelgeek)");
	artists.push_back("Daniel Hornung (rubikcube)");
	artists.push_back("David Rylander (rylleman)");
	artists.push_back("Franco Iacomella (Yaco)");
	artists.push_back("Gerald Young (Yoyobuae)");
	artists.push_back("Henrique Lopes Barone");
	artists.push_back("Konstantin Dmitriev (zelgadis)");
	artists.push_back("Madeleine Crubellier (mad0)");
	artists.push_back("Nikolai Mamashev (solkap)");
	artists.push_back("Robert B. Quattlebaum Jr. (darco)");
	artists.push_back("Thimotee Guiet (satrip)");
	artists.push_back("Yu Chen (jcome)");
	artists.push_back("");
	set_artists(artists);

	Glib::ustring section_name = "Vectorizer feature";
	std::vector<Glib::ustring> credits;
	credits.push_back("Credits:");
	Glib::ustring credit_header =
		"Vectorizer feature is ported from OpenToonz open-source animation\n"
		"software, which is developed from Toonz, a software originally\n"
		"created by Digital Video, S.p.A., Rome Italy.";

	credits.push_back(credit_header);
	credits.push_back("https://github.com/opentoonz/opentoonz/\n");
	
	credits.push_back("Copyright (c) 2016 - 2019, DWANGO Co., Ltd.");
	credits.push_back("Copyright (c) 2016 Toshihiro Shimizu https://github.com/meso");
	credits.push_back("Copyright (c) 2016 Shinya Kitaoka https://github.com/skitaoka");
	credits.push_back("Copyright (c) 2016 shun-iwasawa https://github.com/shun-iwasawa");
	credits.push_back("Copyright (c) 2016 Campbell Barton https://github.com/ideasman42");
	credits.push_back("Copyright (c) 2019 luzpaz https://github.com/luzpaz");
	credits.push_back("Copyright (c) 2019 - 2020, Ankit Kumar Dwivedi https://github.com/ankit-kumar-dwivedi");
	
	Glib::ustring credit_footer =
		"\n== Vectorizer License ==\n\n"
		"Vectorizer feature code is licensed under BSD 3-Clause \"New\" or \"Revised\"   \n"
		"License. Redistribution and use in source and binary forms, with or             \n"
		"without modification, are permitted provided that the following conditions      \n"
		"are met:\n"
		"\t1. Redistributions of source code must retain the above copyright notice,     \n"
		"\t    this list of conditions and the following disclaimer.\n"
		"\t2. Redistributions in binary form must reproduce the above copyright\n"
		"\t    notice, this list of conditions and the following disclaimer in the\n"
		"\t    documentation and/or other materials provided with the distribution.      \n"
		"\t3. Neither the name of the copyright holder nor the names of its\n"
		"\t    contributors may be used to endorse or promote products derived \n"
		"\t    from this software without specific prior written permission.\n"
		"\n"
		"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND\n"
		"CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED\n"
		"WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
		"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"
		"PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
		"HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,\n"
		"INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
		"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE\n"
		"GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS\n"
		"INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n"
		"WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
		"NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
		"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF \n"
		"SUCH DAMAGE.\n\n"
		"== Vectorizer License End ==\n";

	credits.push_back(credit_footer);
	add_credit_section(section_name,credits);
	
	// TRANSLATORS: change this to your name, separate multiple names with \n
	set_translator_credits(_("translator-credits"));

	std::string imagepath = ResourceHelper::get_image_path("synfig_icon." IMAGE_EXT);

	Gtk::Image *logo = manage(new class Gtk::Image());
	
	logo->set(imagepath);
	logo->set_parent(*this);
	set_logo(logo->get_pixbuf());

#ifdef SHOW_EXTRA_INFO

	std::string extra_info = get_comments() + "\n";

	#ifdef DEVEL_VERSION
		extra_info += etl::strprintf(_("\nDevelopment version:\n%s\n"),DEVEL_VERSION);
	#endif

	extra_info += "\n";

	extra_info += etl::strprintf(_("Built on %s" /* at %s */ "\n\n"), __DATE__ /* , __TIME__ */ );

	extra_info += etl::strprintf(_("Built with:\n"));
	extra_info += etl::strprintf(_("ETL %s\n"), ETL_VERSION);
	extra_info += etl::strprintf(_("Synfig API %s\n"), SYNFIG_VERSION);
	extra_info += etl::strprintf(_("Synfig library %d\n"), SYNFIG_LIBRARY_VERSION);
	extra_info += etl::strprintf(_("GTK+ %d.%d.%d\n"), GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
	#ifdef __GNUC__
		extra_info += etl::strprintf(_("GNU G++ %d.%d.%d\n"),__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__);
	#endif

	#ifdef _MSC_VER
		extra_info += etl::strprintf("Microsoft Visual C/C++ (%d)\n", _MSC_VER);
	#endif

	extra_info += "\n";

	extra_info += etl::strprintf(_("Using:\n"));
	extra_info += etl::strprintf(_("Synfig %s\n"), synfig::get_version());
	extra_info += etl::strprintf(_("GTK+ %d.%d.%d"),gtk_major_version,gtk_minor_version,gtk_micro_version);

	#ifdef _DEBUG
		extra_info += "\n\nDEBUG BUILD";
	#endif

	set_comments(extra_info);

#endif

	// Hide the dialog when you click close
	signal_response().connect(sigc::mem_fun(*this, &About::close));
}

void About::close(int){
	hide();
}

