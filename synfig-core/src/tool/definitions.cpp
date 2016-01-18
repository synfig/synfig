/* === S Y N F I G ========================================================= */
/*!	\file tool/definitions.cpp
**	\brief Implementation of the definitions header file for synfig tool
**
**	$Id$
**
**	\legal
**  Copyright (c) 2014 Diego Barrios Romero
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

#include "definitions.h"
#include "synfigtoolexception.h"

#include <synfig/localization.h>
#include <synfig/general.h>

#include <synfig/main.h>

boost::shared_ptr<SynfigToolGeneralOptions> SynfigToolGeneralOptions::_instance;

void SynfigToolGeneralOptions::create_singleton_instance(const char* argv0)
{
	_instance = boost::shared_ptr<SynfigToolGeneralOptions>(
					new SynfigToolGeneralOptions(argv0));
}

SynfigToolGeneralOptions* SynfigToolGeneralOptions::instance()
{
	if (_instance.get() == NULL)
	{
		throw SynfigToolException(SYNFIGTOOL_UNKNOWNERROR,
								  _("Uninitialized Synfig tool general options singleton."));
	}

	return _instance.get();
}

SynfigToolGeneralOptions::SynfigToolGeneralOptions(const char* argv0)
{
	_binary_path = synfig::get_binary_path(argv0);

	_verbosity = 0;
	_should_be_quiet = false;
	_should_print_benchmarks = false;
	_threads = 1;
}

boost::filesystem::path SynfigToolGeneralOptions::get_binary_path() const
{
	return _binary_path;
}

size_t SynfigToolGeneralOptions::get_threads() const
{
	return _threads;
}

void SynfigToolGeneralOptions::set_threads(size_t threads)
{
	_threads = threads;
}

int SynfigToolGeneralOptions::get_verbosity() const
{
	return _verbosity;
}

void SynfigToolGeneralOptions::set_verbosity(int verbosity)
{
	_verbosity = verbosity;
}

bool SynfigToolGeneralOptions::should_be_quiet() const
{
	return _should_be_quiet;
}

void SynfigToolGeneralOptions::set_should_be_quiet(bool be_quiet)
{
	_should_be_quiet = be_quiet;
}

bool SynfigToolGeneralOptions::should_print_benchmarks() const
{
	return _should_print_benchmarks;
}

void SynfigToolGeneralOptions::set_should_print_benchmarks(bool print_benchmarks)
{
	_should_print_benchmarks = print_benchmarks;
}
