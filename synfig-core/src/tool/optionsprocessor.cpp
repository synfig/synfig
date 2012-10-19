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
#include <synfig/layer.h>
#include <synfig/module.h>
#include <synfig/importer.h>

#include "definitions.h"
#include "job.h"
#include "printing_functions.h"
#include "optionsprocessor.h"

using namespace std;
using namespace synfig;

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

	if (_vm.count("layers"))
	{
		synfig::Layer::Book::iterator iter =
			synfig::Layer::book().begin();
		for(; iter != synfig::Layer::book().end(); iter++)
			if (iter->second.category != CATEGORY_DO_NOT_USE)
				cout << iter->first << endl;

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("layer-info"))
	{
		Layer::Handle layer =
			synfig::Layer::create(_vm["layer-info"].as<string>());

		cout << _("Layer Name: ") << layer->get_name() << endl;
		cout << _("Localized Layer Name: ")
			 << layer->get_local_name() << endl;
		cout << _("Version: ") << layer->get_version() << endl;

		Layer::Vocab vocab = layer->get_param_vocab();
		for(; !vocab.empty(); vocab.pop_front())
		{
			cout << _("param - ") << vocab.front().get_name();
			if(!vocab.front().get_critical())
				cout << _(" (not critical)");
			cout << endl << _("\tLocalized Name: ")
				 << vocab.front().get_local_name() << endl;

			if(!vocab.front().get_description().empty())
				cout << _("\tDescription: ")
					 << vocab.front().get_description() << endl;

			if(!vocab.front().get_hint().empty())
				cout << _("\tHint: ")
					 << vocab.front().get_hint() << endl;
		}

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("modules"))
	{
		synfig::Module::Book::iterator iter =
			synfig::Module::book().begin();
		for(; iter != synfig::Module::book().end(); iter++)
			cout << iter->first << endl;

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("targets"))
	{
		synfig::Target::Book::iterator iter =
			synfig::Target::book().begin();
		for(; iter != synfig::Target::book().end(); iter++)
			cout << iter->first << endl;

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("valuenodes"))
	{
		synfig::LinkableValueNode::Book::iterator iter =
			synfig::LinkableValueNode::book().begin();
		for(; iter != synfig::LinkableValueNode::book().end(); iter++)
			cout << iter->first << endl;

		return SYNFIGTOOL_HELP;
	}

	if (_vm.count("importers"))
	{
		synfig::Importer::Book::iterator iter =
			synfig::Importer::book().begin();
		for(; iter != synfig::Importer::book().end(); iter++)
			cout << iter->first << endl;

		return SYNFIGTOOL_HELP;
	}

	return SYNFIGTOOL_OK;
}

