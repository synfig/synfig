/* === S Y N F I G ========================================================= */
/*!	\file gui/json_to_dialog_converter.h
**	\brief Fetch dialog data as JSON object or fill the dialog control widgets with JSON input
**
**	\legal
**	Copyright (c) 2023-2025 Synfig Contributors
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

/* === S T A R T =========================================================== */

#ifndef JSON_TO_DIALOG_CONVERTER_H
#define JSON_TO_DIALOG_CONVERTER_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <string>

#include <gtkmm/widget.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

/* === P R O C E D U R E S ================================================= */

namespace JSON {

/** Return a JSON object with dialog data (key: widget name -> value: widget data) */
std::map<std::string, std::string> parse_dialog(const Gtk::Widget& dialog_contents);

/** Set the dialog widget data with a JSON object values (key: widget name -> value: widget data) */
void hydrate_dialog(Gtk::Container* container, const std::map<std::string, std::string>& values);

/** Set the control widget data with a string value */
void set_widget_value(Gtk::Widget* widget, const std::string& value);
}

#endif // JSON_TO_DIALOG_CONVERTER_H
