/* === S Y N F I G ========================================================= */
/*!	\file tool/definitions.h
**	\brief Definitions for synfig tool
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2012, 2014 Diego Barrios Romero
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

#ifndef __SYNFIG_DEFINITIONS_H
#define __SYNFIG_DEFINITIONS_H

/* === M A C R O S ========================================================= */

#ifdef ENABLE_NLS
#undef _
#define _(x) gettext(x)
#else
#undef _
#define _(x) (x)
#endif

#ifndef VERSION
#define VERSION "unknown"
#define PACKAGE "synfig-tool"
#endif

#ifdef DEFAULT_QUALITY
#undef DEFAULT_QUALITY
#endif

#define DEFAULT_QUALITY		2
#define VERBOSE_OUT(x) if (SynfigToolGeneralOptions::instance()->get_verbosity() >= (x)) std::cerr

#define SYNFIG_LICENSE "\
**	Synfig is free software: you can redistribute it and/or modify\n\
**	it under the terms of the GNU General Public License as published by\n\
**	the Free Software Foundation, either version 2 of the License, or\n\
**	(at your option) any later version.\n\
**\n\
**	"

enum exit_code
{
	SYNFIGTOOL_OK				= 0,
	SYNFIGTOOL_FILENOTFOUND		= 1,
	SYNFIGTOOL_BORED			= 2,
	SYNFIGTOOL_HELP				= 3,
	SYNFIGTOOL_UNKNOWNARGUMENT	= 4,
	SYNFIGTOOL_UNKNOWNERROR		= 5,
	SYNFIGTOOL_INVALIDTARGET	= 6,
	SYNFIGTOOL_RENDERFAILURE	= 7,
	SYNFIGTOOL_BLANK			= 8,
	SYNFIGTOOL_BADVERSION		= 9,
	SYNFIGTOOL_MISSINGARGUMENT	=10,
	SYNFIGTOOL_INVALIDJOB       =11,
	SYNFIGTOOL_INVALIDOUTPUT    =12
};

#include <string>
#include <memory>

class SynfigToolGeneralOptions
{
public:
	//! \throw exception in case the instance already existed
	static void create_singleton_instance(const char* argv0);

	static SynfigToolGeneralOptions* instance();

	std::string get_binary_path() const;

	size_t get_threads() const;

	void set_threads(size_t threads);

	int get_verbosity() const;

	void set_verbosity(int verbosity);

	bool should_be_quiet() const;

	void set_should_be_quiet(bool be_quiet);

	bool should_print_benchmarks() const;

	void set_should_print_benchmarks(bool print_benchmarks);

private:
	SynfigToolGeneralOptions(const char* argv0);

	std::string _binary_path;
	int _verbosity;
	size_t _threads;
	bool _should_be_quiet,
		 _should_print_benchmarks;

	static std::shared_ptr<SynfigToolGeneralOptions> _instance;
};

#endif
