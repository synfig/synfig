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
	char exe_path[PATH_MAX];
	uint32_t size = sizeof(exe_path);
	if (_NSGetExecutablePath(exe_path, &size) == 0) {
		char* dir = dirname(exe_path);
		char resources_path[PATH_MAX];
		snprintf(resources_path, sizeof(resources_path), "%s/../Resources", dir);
		
		// Set DYLD_LIBRARY_PATH
		char frameworks_path[PATH_MAX];
		snprintf(frameworks_path, sizeof(frameworks_path), "%s/../Frameworks", dir);
		setenv("DYLD_LIBRARY_PATH", frameworks_path, 1);

		// Set non-library environment variables
		char modules_path[PATH_MAX];
		snprintf(modules_path, sizeof(modules_path), "%s/synfig/modules", resources_path);
		setenv("LTDL_LIBRARY_PATH", modules_path, 1);
		
		char share_path[PATH_MAX];
		snprintf(share_path, sizeof(share_path), "%s/share", resources_path);
		setenv("XDG_DATA_DIRS", share_path, 1);
		
		// GTK and other application-specific environment variables
		setenv("GTK_EXE_PREFIX", resources_path, 1);
		char gtk_data_prefix_path[PATH_MAX];
		snprintf(gtk_data_prefix_path, sizeof(gtk_data_prefix_path), "%s/share", resources_path);
		setenv("GTK_DATA_PREFIX", gtk_data_prefix_path, 1);

		char gsettings_schema_dir_path[PATH_MAX];
		snprintf(gsettings_schema_dir_path, sizeof(gsettings_schema_dir_path), "%s/share/glib-2.0/schemas/", resources_path);
		setenv("GSETTINGS_SCHEMA_DIR", gsettings_schema_dir_path, 1);

		char gdk_pixbuf_moduledir_path[PATH_MAX];
		snprintf(gdk_pixbuf_moduledir_path, sizeof(gdk_pixbuf_moduledir_path), "%s/lib/gdk-pixbuf-2.0/2.10.0/loaders/", resources_path);
		setenv("GDK_PIXBUF_MODULEDIR", gdk_pixbuf_moduledir_path, 1);

		char* home_dir = getenv("HOME");
		if (home_dir) {
			char gdk_pixbuf_module_file_path[PATH_MAX];
			snprintf(gdk_pixbuf_module_file_path, sizeof(gdk_pixbuf_module_file_path), "%s/.synfig-gdk-loaders", home_dir);
			setenv("GDK_PIXBUF_MODULE_FILE", gdk_pixbuf_module_file_path, 1);

			// Generate the loaders file
			struct stat buffer;
			if (stat(gdk_pixbuf_module_file_path, &buffer) == 0) {
				remove(gdk_pixbuf_module_file_path);
			}
			char command[PATH_MAX * 2];
			snprintf(command, sizeof(command), "\"%s/bin/gdk-pixbuf-query-loaders\" > \"%s\"", resources_path, gdk_pixbuf_module_file_path);
			system(command);
		}

		char synfig_module_list_path[PATH_MAX];
		snprintf(synfig_module_list_path, sizeof(synfig_module_list_path), "%s/etc/synfig_modules.cfg", resources_path);
		setenv("SYNFIG_MODULE_LIST", synfig_module_list_path, 1);

		char fontconfig_path[PATH_MAX];
		snprintf(fontconfig_path, sizeof(fontconfig_path), "%s/etc/fonts", resources_path);
		setenv("FONTCONFIG_PATH", fontconfig_path, 1);

		char mlt_data_path[PATH_MAX];
		snprintf(mlt_data_path, sizeof(mlt_data_path), "%s/share/mlt/", resources_path);
		setenv("MLT_DATA", mlt_data_path, 1);

		char mlt_repository_path[PATH_MAX];
		snprintf(mlt_repository_path, sizeof(mlt_repository_path), "%s/lib/mlt/", resources_path);
		setenv("MLT_REPOSITORY", mlt_repository_path, 1);

		char path_env[PATH_MAX * 2];
		snprintf(path_env, sizeof(path_env), "%s/bin:%s/synfig-production/bin:%s", resources_path, resources_path, getenv("PATH") ? getenv("PATH") : "");
		setenv("PATH", path_env, 1);

		setenv("SYNFIG_ROOT", resources_path, 1);

		// Python environment variables
		setenv("PYTHONHOME", resources_path, 1);

		// ImageMagick environment variables
		char magick_configure_path[PATH_MAX];
		snprintf(magick_configure_path, sizeof(magick_configure_path), "%s/lib/ImageMagick-7/config-Q16HDRI", resources_path);
		setenv("MAGICK_CONFIGURE_PATH", magick_configure_path, 1);

		char magick_coder_module_path[PATH_MAX];
		snprintf(magick_coder_module_path, sizeof(magick_coder_module_path), "%s/lib/ImageMagick-7/modules-Q16HDRI/coders", resources_path);
		setenv("MAGICK_CODER_MODULE_PATH", magick_coder_module_path, 1);

		char magick_coder_filter_path[PATH_MAX];
		snprintf(magick_coder_filter_path, sizeof(magick_coder_filter_path), "%s/lib/ImageMagick-7/modules-Q16HDRI/filters", resources_path);
		setenv("MAGICK_CODER_FILTER_PATH", magick_coder_filter_path, 1);
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
