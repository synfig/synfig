/* === S Y N F I G ========================================================= */
/*!	\file tool/main.cpp
**	\brief SYNFIG Tool
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2015 Diego Barrios Romero
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

#include <iostream>
#include <string>
#include <list>


#include <glibmm.h>

#include <synfig/localization.h>
#include <synfig/main.h>
#include <synfig/version.h>
#include <autorevision.h>
#include "definitions.h"
#include "progress.h"
#include "job.h"
#include "synfigtoolexception.h"
#include "optionsprocessor.h"
#include "joblistprocessor.h"
#include "printing_functions.h"

#endif


int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");
	Glib::init(); // need to use Gio functions before app is started

#ifdef _WIN32
	// to be able to open files whose name is not latin (eg. arabic)
	class ArgVGuard {
		char **modified_argv;
	public:
		ArgVGuard(char ***argv) { modified_argv = *argv = g_win32_get_command_line(); }
		~ArgVGuard() { g_strfreev(modified_argv); }
	} argv_guard(&argv);
 #endif

	SynfigToolGeneralOptions::instance()->set_fallback_binary_path(argv[0]); // on MS Windows, already converted to UTF-8 via ArgVGuarg

	const std::string binary_path =
		SynfigToolGeneralOptions::instance()->get_binary_path();
	const std::string root_path = get_absolute_path(binary_path + "/../../");

#ifdef ENABLE_NLS
	std::string locale_path = root_path + "/share/locale";
	bindtextdomain("synfig", Glib::locale_from_utf8(locale_path).c_str() );
	bind_textdomain_codeset("synfig", "UTF-8");
	textdomain("synfig");
#endif

	if(!SYNFIG_CHECK_VERSION())
	{
		std::cerr << _("FATAL: Synfig Version Mismatch") << std::endl;
		return SYNFIGTOOL_BADVERSION;
	}

	try
	{
		if(argc == 1)
		{
			throw (SynfigToolException(SYNFIGTOOL_MISSINGARGUMENT));
		}

        //OptionsProcessor op(vm, po_visible);
		SynfigCommandLineParser parser;
		parser.parse(argc, argv);

        // Switch options ---------------------------------------------
        parser.process_settings_options();

#ifdef _DEBUG
		// DEBUG options ----------------------------------------------
		parser.process_debug_options();
#endif

		// Trivial info options -----------------------------------------------
		parser.process_trivial_info_options();

		// Synfig Main initialization needs to be after verbose and
		// before any other where it's used
		Progress p(binary_path.c_str());
		synfig::Main synfig_main(root_path, &p);

		// Info options -----------------------------------------------
		parser.process_info_options();

		std::list<Job> job_list;

		// Processing --------------------------------------------------
		Job job;
		job = parser.extract_job();
		job.desc = job.canvas->rend_desc() = parser.extract_renddesc(job.canvas->rend_desc());

		if (job.extract_alpha) {
			job.alpha_mode = synfig::TARGET_ALPHA_MODE_REDUCE;
			job_list.push_front(job);
			job.alpha_mode = synfig::TARGET_ALPHA_MODE_EXTRACT;
			job.outfilename.add_suffix("-alpha");
			job_list.push_front(job);
		} else {
			job_list.push_front(job);
		}

		process_job_list(job_list, parser.extract_targetparam());

		return SYNFIGTOOL_OK;

    }
    catch (SynfigToolException& e) {
    	exit_code code = e.get_exit_code();
    	if (code != SYNFIGTOOL_HELP && code != SYNFIGTOOL_OK)
    		std::cerr << e.get_message().c_str() << std::endl;
    	if (code == SYNFIGTOOL_MISSINGARGUMENT)
    		print_usage();

    	return code;
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }

}
