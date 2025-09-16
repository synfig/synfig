/* === S Y N F I G ========================================================= */
/*!	\file gui/main.cpp
**	\brief Synfig Studio Entrypoint
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

#include <synfig/os.h>

#include <gui/app.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>

#include <iostream>

#ifdef __APPLE__
#include <unistd.h>
#include <libgen.h>
#include <cstring>
#include <mach-o/dyld.h>
#include <cstdlib>
#include <sys/stat.h>
#endif

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

int main(int argc, char **argv)
{
#ifdef __APPLE__
	// Set up macOS app bundle environment
	char exe_path_c[PATH_MAX];
	uint32_t size = sizeof(exe_path_c);
	if (_NSGetExecutablePath(exe_path_c, &size) == 0) {
		// Use Glib for safer and more readable path manipulation
		std::string exe_path(exe_path_c);
		std::string dir = Glib::path_get_dirname(exe_path);
		std::string resources_path = Glib::build_filename(dir, "..", "Resources");
		std::string frameworks_path = Glib::build_filename(dir, "..", "Frameworks");

		// Set DYLD_LIBRARY_PATH
		Glib::setenv("DYLD_LIBRARY_PATH", frameworks_path);

		// Set non-library environment variables
		Glib::setenv("LTDL_LIBRARY_PATH", Glib::build_filename(resources_path, "synfig", "modules"));

		// Handle XDG_DATA_DIRS, appending our path if it already exists
		std::string share_path = Glib::build_filename(resources_path, "share");
		std::string xdg_data_dirs = Glib::getenv("XDG_DATA_DIRS");
		if (!xdg_data_dirs.empty()) {
			xdg_data_dirs = share_path + G_SEARCHPATH_SEPARATOR_S + xdg_data_dirs;
		} else {
			xdg_data_dirs = share_path;
		}
		Glib::setenv("XDG_DATA_DIRS", xdg_data_dirs);
		
		// GTK and other application-specific environment variables
		Glib::setenv("GTK_EXE_PREFIX", resources_path);
		Glib::setenv("GTK_DATA_PREFIX", share_path);
		Glib::setenv("GSETTINGS_SCHEMA_DIR", Glib::build_filename(share_path, "glib-2.0", "schemas"));
		Glib::setenv("GDK_PIXBUF_MODULEDIR", Glib::build_filename(resources_path, "lib", "gdk-pixbuf-2.0", "2.10.0", "loaders"));

		// Handle GDK Pixbuf module file
		std::string home_dir = Glib::getenv("HOME");
		if (!home_dir.empty()) {
			std::string gdk_pixbuf_module_file = Glib::build_filename(home_dir, ".synfig-gdk-loaders");
			Glib::setenv("GDK_PIXBUF_MODULE_FILE", gdk_pixbuf_module_file);

			// Generate the loaders file
			struct stat buffer;
			if (stat(gdk_pixbuf_module_file.c_str(), &buffer) == 0) {
				remove(gdk_pixbuf_module_file.c_str());
			}
			std::string gdk_query_loaders_path = Glib::build_filename(resources_path, "bin", "gdk-pixbuf-query-loaders");
			std::string command = "\"" + gdk_query_loaders_path + "\" > \"" + gdk_pixbuf_module_file + "\"";
			system(command.c_str());
		}

		Glib::setenv("SYNFIG_MODULE_LIST", Glib::build_filename(resources_path, "etc", "synfig_modules.cfg"));
		Glib::setenv("FONTCONFIG_PATH", Glib::build_filename(resources_path, "etc", "fonts"));
		Glib::setenv("MLT_DATA", Glib::build_filename(share_path, "mlt") + "/");
		Glib::setenv("MLT_REPOSITORY", Glib::build_filename(resources_path, "lib", "mlt") + "/");

		// Handle PATH, prepending our paths
		std::string old_path = Glib::getenv("PATH");
		std::string new_path = Glib::build_filename(resources_path, "bin") + G_SEARCHPATH_SEPARATOR_S +
		                     Glib::build_filename(resources_path, "synfig-production", "bin");
		if (!old_path.empty()) {
			new_path += G_SEARCHPATH_SEPARATOR_S + old_path;
		}
		Glib::setenv("PATH", new_path);

		Glib::setenv("SYNFIG_ROOT", resources_path + "/");

		// Python environment variables
		setenv("PYTHONHOME", resources_path, 1);

		// ImageMagick environment variables
		Glib::setenv("MAGICK_CONFIGURE_PATH", Glib::build_filename(resources_path, "lib", "ImageMagick-7", "config-Q16HDRI"));
		Glib::setenv("MAGICK_CODER_MODULE_PATH", Glib::build_filename(resources_path, "lib", "ImageMagick-7", "modules-Q16HDRI", "coders"));
		Glib::setenv("MAGICK_CODER_FILTER_PATH", Glib::build_filename(resources_path, "lib", "ImageMagick-7", "modules-Q16HDRI", "filters"));
	}
#endif

	synfig::OS::fallback_binary_path = filesystem::Path(Glib::filename_to_utf8(argv[0]));
	const filesystem::Path rootpath = synfig::OS::get_binary_path().parent_path().parent_path();
	
#ifdef ENABLE_NLS
	filesystem::Path locale_dir;
	locale_dir = rootpath / filesystem::Path("share/locale");
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, locale_dir.u8_str() );
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif
	
	std::cout << std::endl;
	std::cout << "   " << _("synfig studio -- starting up application...") << std::endl << std::endl;

	SYNFIG_EXCEPTION_GUARD_BEGIN()
	
	Glib::RefPtr<studio::App> app = studio::App::instance();

	app->signal_startup().connect([app, rootpath]() {
		app->init(rootpath.u8string());
	});

	Glib::set_prgname("org.synfig.SynfigStudio");

	app->register_application();
	if (app->is_remote()) {
		std::cout << std::endl;
		std::cout << "   " << _("synfig studio is already running") << std::endl << std::endl;
		std::cout << "   " << _("the existing process will be used") << std::endl << std::endl;
	}

	int exit_code = app->run(argc, argv);

	std::cerr << "Application appears to have terminated successfully" << std::endl;

	return exit_code;

	SYNFIG_EXCEPTION_GUARD_END_INT(0)
}
