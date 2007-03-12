/* === S Y N F I G ========================================================= */
/*!	\file loadcanvas.h
**	\brief writeme
**
**	$Id: loadcanvas.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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
#include "valuenode_const.h"
#include "valuenode_linear.h"
#include "valuenode_dynamiclist.h"
#include "valuenode_reference.h"
#include "valuenode_timedswap.h"
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

	//! \todo writeme
	Canvas::Handle parse_from_file(const String &filename);

	Canvas::Handle parse_from_file_as(const String &filename,const String &as);

	//! \todo writeme
	Canvas::Handle parse_from_string(const String &data);

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

	Real parse_real(xmlpp::Element *node,Canvas::Handle canvas);
	Time parse_time(xmlpp::Element *node,Canvas::Handle canvas);
	int parse_integer(xmlpp::Element *node,Canvas::Handle canvas);
	Vector parse_vector(xmlpp::Element *node,Canvas::Handle canvas);
	Color parse_color(xmlpp::Element *node,Canvas::Handle canvas);
	Angle parse_angle(xmlpp::Element *node,Canvas::Handle canvas);
	String parse_string(xmlpp::Element *node,Canvas::Handle canvas);
	bool parse_bool(xmlpp::Element *node,Canvas::Handle canvas);
	Segment parse_segment(xmlpp::Element *node,Canvas::Handle canvas);
	ValueBase parse_list(xmlpp::Element *node,Canvas::Handle canvas);
	Gradient parse_gradient(xmlpp::Element *node,Canvas::Handle canvas);
	BLinePoint parse_bline_point(xmlpp::Element *node,Canvas::Handle canvas);

	Keyframe parse_keyframe(xmlpp::Element *node,Canvas::Handle canvas);

	// ValueNode Parsing Functions

	etl::handle<ValueNode_Animated> parse_animated(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<ValueNode_Subtract> parse_subtract(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<ValueNode_Animated> parse_timedswap(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<LinkableValueNode> parse_linkable_value_node(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<ValueNode_Composite> parse_composite(xmlpp::Element *node,Canvas::Handle canvas);
	etl::handle<ValueNode_DynamicList> parse_dynamic_list(xmlpp::Element *node,Canvas::Handle canvas);

}; // END of CanvasParser

/* === E X T E R N S ======================================================= */

//!	Loads a canvas from \a filename
/*!	\return	The Canvas's handle on success, an empty handle on failure */
extern Canvas::Handle open_canvas(const String &filename);
extern Canvas::Handle open_canvas_as(const String &filename,const String &as);

//! Retrieves a Canvas from a string in XML format
extern Canvas::Handle string_to_canvas(const String &data);

std::map<synfig::String, etl::loose_handle<Canvas> >& get_open_canvas_map();

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
