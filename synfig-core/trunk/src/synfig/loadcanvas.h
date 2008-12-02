/* === S Y N F I G ========================================================= */
/*!	\file loadcanvas.h
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_LOADCANVAS_H
#define __SYNFIG_LOADCANVAS_H

/* === H E A D E R S ======================================================= */

#include "string.h"
#include "canvas.h"
#include "valuenode.h"
#include "vector.h"
#include "value.h"
#include "valuenode_subtract.h"
#include "valuenode_animated.h"
#include "valuenode_composite.h"
#include "valuenode_staticlist.h"
#include "valuenode_dynamiclist.h"
#include "keyframe.h"
#include "guid.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace xmlpp { class Node; class Element; };

namespace synfig {

/*!	\class CanvasParser
**	\todo writeme
*/
class CanvasParser
{
	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:

	int max_warnings_;

    int total_warnings_;

    int total_errors_;

	bool allow_errors_;

	String filename;

	String path;

	String errors_text;
	String warnings_text;

	GUID guid_;

	/*
 --	** -- C O N S T R U C T O R S ---------------------------------------------
	*/

public:

	CanvasParser():
		max_warnings_	(1000),
		total_warnings_	(0),
		total_errors_	(0),
		allow_errors_	(false)
	{ }

	/*
 --	** -- M E M B E R   F U N C T I O N S -------------------------------------
	*/

public:

	//! \todo writeme
	CanvasParser &set_allow_errors(bool x) { allow_errors_=x; return *this; }

	//! Sets the maximum number of warnings before a fatal error is thrown
	CanvasParser &set_max_warnings(int i) { max_warnings_=i; return *this; }

	//! Returns the maximum number of warnings before a fatal_error is thrown
	int get_max_warnings() { return max_warnings_; }

	//! Returns the number of errors in the last parse
	int error_count()const { return total_errors_; }

	//! Returns the number of warnings in the last parse
	int warning_count()const { return total_warnings_; }

	void set_path(const synfig::String& x) { path=x; }

	const synfig::String& get_path()const { return path; }

	const synfig::String& get_errors_text()const { return errors_text; }
	const synfig::String& get_warnings_text()const { return warnings_text; }

	static void register_canvas_in_map(Canvas::Handle canvas, String as);

#ifdef _DEBUG
	static void show_canvas_map(String file, int line, String text);
#endif	// _DEBUG

	//! \todo writeme
	Canvas::Handle parse_from_file_as(const String &filename,const String &as,String &errors);

	static std::set<String> loading_;

private:

	// Error/Warning handling functions

	void error(xmlpp::Node *node,const String &text);
	void fatal_error(xmlpp::Node *node,const String &text);
	void warning(xmlpp::Node *node,const String &text);
	void error_unexpected_element(xmlpp::Node *node,const String &got, const String &expected);
	void error_unexpected_element(xmlpp::Node *node,const String &got);

	// Parsing Functions

	Canvas::Handle parse_canvas(xmlpp::Element *node,Canvas::Handle parent=0,bool inline_=false, String path=".");
	void parse_canvas_defs(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<Layer> parse_layer(xmlpp::Element *node,Canvas::Handle canvas);
	ValueBase parse_value(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<ValueNode> parse_value_node(xmlpp::Element *node,Canvas::Handle canvas);

	// ValueBase Parsing Functions

	Real parse_real(xmlpp::Element *node);
	Time parse_time(xmlpp::Element *node,Canvas::Handle canvas);
	int parse_integer(xmlpp::Element *node);
	Vector parse_vector(xmlpp::Element *node);
	Color parse_color(xmlpp::Element *node);
	Angle parse_angle(xmlpp::Element *node);
	String parse_string(xmlpp::Element *node);
	bool parse_bool(xmlpp::Element *node);
	Segment parse_segment(xmlpp::Element *node);
	ValueBase parse_list(xmlpp::Element *node,Canvas::Handle canvas);
	Gradient parse_gradient(xmlpp::Element *node);
	BLinePoint parse_bline_point(xmlpp::Element *node);
	Bone parse_bone(xmlpp::Element *node);

	Keyframe parse_keyframe(xmlpp::Element *node,Canvas::Handle canvas);

	// ValueNode Parsing Functions

	etl::handle<ValueNode_Animated> parse_animated(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<ValueNode_Subtract> parse_subtract(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<LinkableValueNode> parse_linkable_value_node(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<ValueNode_StaticList> parse_static_list(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<ValueNode_DynamicList> parse_dynamic_list(xmlpp::Element *node,Canvas::Handle canvas);

}; // END of CanvasParser

/* === E X T E R N S ======================================================= */

//!	Loads a canvas from \a filename
/*!	\return	The Canvas's handle on success, an empty handle on failure */
extern Canvas::Handle open_canvas(const String &filename,String &errors,String &warnings);
extern Canvas::Handle open_canvas_as(const String &filename,const String &as,String &errors,String &warnings);

std::map<synfig::String, etl::loose_handle<Canvas> >& get_open_canvas_map();

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
