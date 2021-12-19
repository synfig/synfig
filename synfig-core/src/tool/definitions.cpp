/* === S Y N F I G ========================================================= */
/*!	\file tool/definitions.cpp
**	\brief Implementation of the definitions header file for synfig tool
**
**	\legal
**  Copyright (c) 2014 Diego Barrios Romero
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

#include "definitions.h"
#include "synfigtoolexception.h"
#include <synfig/general.h>

SynfigToolGeneralOptions* SynfigToolGeneralOptions::instance() {
	static SynfigToolGeneralOptions instance;
	return &instance;
}

SynfigToolGeneralOptions::SynfigToolGeneralOptions()
{
	_verbosity = 0;
	_should_be_quiet = false;
	_should_print_benchmarks = false;
	_threads = 1;
}

std::string SynfigToolGeneralOptions::get_binary_path() const
{
	return _binary_path;
}

void SynfigToolGeneralOptions::set_binary_path(const std::string& path) {
	_binary_path = synfig::get_binary_path(path);
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
	synfig::synfig_quiet_mode = be_quiet;
}

bool SynfigToolGeneralOptions::should_print_benchmarks() const
{
	return _should_print_benchmarks;
}

void SynfigToolGeneralOptions::set_should_print_benchmarks(bool print_benchmarks)
{
	_should_print_benchmarks = print_benchmarks;
}
