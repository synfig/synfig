/* === S Y N F I G ========================================================= */
/*!	\file pybind11.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <pybind11/embed.h> // everything needed for embedding
#include "synfig/general.h"

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(synfig, m) {

	m.doc() = "synfig pybind11 example module"; // optional module docstring

	m.def("add", [](int i, int j) {
		return i + j;
	});

	m.def("info",    static_cast<void (*)(const std::string&)>(&synfig::info),    "print info");
	m.def("warning", static_cast<void (*)(const std::string&)>(&synfig::warning), "print warning");
	m.def("error",   static_cast<void (*)(const std::string&)>(&synfig::error),   "print error");
}


