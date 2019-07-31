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

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef VERSION
#define VERSION	"unknown"
#endif

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"png"
#endif

//#define stringify(x) #x
#define stringify(x) (x)
//#define stringify(x) (std::to_string(x).c_str())

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

	set_copyright(_("Copyright (c) 2001-2018\nSynfig developers & contributors"));
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
	set_artists(artists);

	// TRANSLATORS: change this to your name, separate multiple names with \n
	set_translator_credits(_("translator-credits"));

	std::string imagepath = ResourceHelper::get_image_path("synfig_icon." IMAGE_EXT);

	Gtk::Image *Logo = manage(new class Gtk::Image());
	
	Logo->set(imagepath);
	Logo->set_parent(*this);
	set_logo(Logo->get_pixbuf());

#ifdef SHOW_EXTRA_INFO

	string extra_info = get_comments() + "\n";

	#ifdef DEVEL_VERSION
		extra_info += strprintf(_("\nDevelopment version:\n%s\n"),DEVEL_VERSION);
	#endif

	extra_info += "\n";

	extra_info += strprintf(_("Built on %s" /* at %s */ "\n"), __DATE__ /* , __TIME__ */ );

	extra_info += "\n";

	extra_info += strprintf(_("Built with:\n"));
	extra_info += strprintf(_("ETL %s\n"), ETL_VERSION);
	extra_info += strprintf(_("Synfig API %s\n"), stringify(SYNFIG_VERSION));
	extra_info += strprintf(_("Synfig library %d\n"), SYNFIG_LIBRARY_VERSION);
	extra_info += strprintf(_("GTK+ %d.%d.%d\n"), GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
	#ifdef __GNUC__
		extra_info += strprintf(_("GNU G++ %d.%d.%d\n"),__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__);
	#endif

	extra_info += "\n";

	extra_info += strprintf(_("Using:\n"), synfig::get_version());
	extra_info += strprintf(_("Synfig %s\n"), synfig::get_version());
	extra_info += strprintf(_("GTK+ %d.%d.%d"),gtk_major_version,gtk_minor_version,gtk_micro_version);

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
