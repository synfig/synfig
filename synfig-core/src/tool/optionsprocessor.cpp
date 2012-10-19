/* === S Y N F I G ========================================================= */
/*!	\file tool/optionsprocessor.cpp
**	\brief Synfig Tool Options Processor Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009-2012 Diego Barrios Romero
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

#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <autorevision.h>
#include <synfig/general.h>
#include <synfig/canvas.h>
#include <synfig/target.h>

#include "definitions.h"
#include "job.h"
#include "printing_functions.h"
#include "optionsprocessor.h"

using namespace std;

void OptionsProcessor::process_switch_options()
{
	if (_vm.count("verbose"))
	{
		verbosity = _vm["verbose"].as<int>();
		VERBOSE_OUT(1) << _("verbosity set to ") << verbosity
					   << endl;
	}

	if (_vm.count("benchmarks"))
		print_benchmarks=true;

	if (_vm.count("quiet"))
		be_quiet=true;
}

exit_code OptionsProcessor::process_info_options()
{
	if (_vm.count("help"))
	{
		print_usage();
		cout << _po_visible;

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("info"))
	{
		cout << PACKAGE "-" VERSION << endl;
#ifdef DEVEL_VERSION
			cout << endl << DEVEL_VERSION << endl << endl;
#endif
		cout << "Compiled on " __DATE__ /* " at "__TIME__ */;
#ifdef __GNUC__
		cout << " with GCC " << __VERSION__;
#endif
#ifdef _MSC_VER
		cout << " with Microsoft Visual C++ "
			 << (_MSC_VER>>8) << '.' << (_MSC_VER&255);
#endif
#ifdef __TCPLUSPLUS__
		cout << " with Borland Turbo C++ "
			 << (__TCPLUSPLUS__>>8) << '.'
			 << ((__TCPLUSPLUS__&255)>>4) << '.'
			 << (__TCPLUSPLUS__&15);
#endif
		cout << endl << SYNFIG_COPYRIGHT << endl;
		cout << endl;

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("version"))
	{
		cerr << PACKAGE << " " << VERSION << endl;

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("license"))
	{
		cerr << PACKAGE << " " << VERSION << endl;
		cout << SYNFIG_COPYRIGHT << endl << endl;
		cerr << SYNFIG_LICENSE << endl << endl;

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("target-video-codecs"))
	{
		print_target_video_codecs_help();

		return SYNFIGTOOL_HELP;
	}

	return SYNFIGTOOL_OK;
}

