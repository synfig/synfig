/* === S Y N F I G ========================================================= */
/*!	\file loadcanvas.h
**	\brief Implementation for the Synfig Canvas Loader (canvas file parser)
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Carlos A. Sosa Navarro
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
#include "valuenodes/valuenode_subtract.h"
#include "valuenodes/valuenode_animated.h"
#include "valuenodes/valuenode_composite.h"
#include "valuenodes/valuenode_staticlist.h"
#include "valuenodes/valuenode_dynamiclist.h"
#include "keyframe.h"
#include "guid.h"
#include "filesystemnative.h"
#include "weightedvalue.h"
#include "pair.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace xmlpp { class Node; class Element; };

namespace synfig {

/*!	\class CanvasParser
**	\brief Class that handles xmlpp elements from a sif file and converts
* them into Synfig objects
*/
class CanvasParser
{
	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

private:
	//! Maximum number of allowed warnings before fatal error is thrown
	int max_warnings_;
	//! Total number of warning during canvas parsing
    int total_warnings_;
	//! Total number of errors during canvas parsing
    int total_errors_;
	//! True if errors doesn't stop canvas parsing
	bool allow_errors_;
	//! File name to parse
	String filename;
	//! Path of the file name to parse
	String path;
	//! Error text when errors found
	String errors_text;
	//! Warning text when warnings found
	String warnings_text;
	//! Seems not to be used
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

	//! Sets allow errors variable
	CanvasParser &set_allow_errors(bool x) { allow_errors_=x; return *this; }

	//! Sets the maximum number of warnings before a fatal error is thrown
	CanvasParser &set_max_warnings(int i) { max_warnings_=i; return *this; }

	//! Returns the maximum number of warnings before a fatal_error is thrown
	int get_max_warnings() { return max_warnings_; }

	//! Returns the number of errors in the last parse
	int error_count()const { return total_errors_; }

	//! Returns the number of warnings in the last parse
	int warning_count()const { return total_warnings_; }

	//! Sets the path of the file to parse
	void set_path(const synfig::String& x) { path=x; }

	//! Gets the path of the file to parse
	const synfig::String& get_path()const { return path; }

	//! Gets error text string
	const synfig::String& get_errors_text()const { return errors_text; }
	//! Gets warning text string
	const synfig::String& get_warnings_text()const { return warnings_text; }

	//! Register a canvas in the canvas map
	/*! \param canvas The handle to the canvas to register
	 *  \param as The absolute path to the file that represents the canvas
	 * Apart of store the pair canvas and */
	static void register_canvas_in_map(Canvas::Handle canvas, String as);

#ifdef _DEBUG
	static void show_canvas_map(String file, int line, String text);
#endif	// _DEBUG

	//! Parse a Cavnas from a file with absolute path.
	Canvas::Handle parse_from_file_as(const FileSystem::Identifier &identifier,const String &as,String &errors);
	//! Parse a Canvas from a xmlpp root node
	Canvas::Handle parse_as(xmlpp::Element* node,String &errors);

	//! Set of absolute file names of the canvases currently being parsed
	static std::set<FileSystem::Identifier> loading_;

private:

	//! Error handling function
	void error(xmlpp::Node *node,const String &text);
	//! Fatal Error handling function
	void fatal_error(xmlpp::Node *node,const String &text);
	//! Warning handling function
	void warning(xmlpp::Node *node,const String &text);
	//! Unexpected element error handling function
	void error_unexpected_element(xmlpp::Node *node,const String &got, const String &expected);
	//! Unexpected element error handling function
	void error_unexpected_element(xmlpp::Node *node,const String &got);

	//! Canvas Parsing Function
	Canvas::Handle parse_canvas(xmlpp::Element *node,Canvas::Handle parent=0,bool inline_=false,const FileSystem::Identifier &identifier = FileSystemNative::instance()->get_identifier(std::string()),String path=".");
	//! Canvas definitions Parsing Function (exported value nodes and exported canvases)
	void parse_canvas_defs(xmlpp::Element *node,Canvas::Handle canvas);

	std::list<ValueNode::Handle> parse_canvas_bones(xmlpp::Element *node,Canvas::Handle canvas);

	//! Layer Parsing Function
	etl::handle<Layer> parse_layer(xmlpp::Element *node,Canvas::Handle canvas);
	//! Generic Value Base Parsing Function
	ValueBase parse_value(xmlpp::Element *node,Canvas::Handle canvas);
	//! Generic Value Node Parsing Function
	etl::handle<ValueNode> parse_value_node(xmlpp::Element *node,Canvas::Handle canvas);

	//! Real Value Base Parsing Function
	Real parse_real(xmlpp::Element *node);
	//! Time Value Base Parsing Function
	Time parse_time(xmlpp::Element *node,Canvas::Handle canvas);
	//! Integer Value Base Parsing Function
	int parse_integer(xmlpp::Element *node);
	//! Vector Value Base Parsing Function
	Vector parse_vector(xmlpp::Element *node);
	//! Color Value Base Parsing Function
	Color parse_color(xmlpp::Element *node);
	//! Angle Value Base Parsing Function
	Angle parse_angle(xmlpp::Element *node);
	//! String Value Base Parsing Function
	String parse_string(xmlpp::Element *node);
	//! Bool Value Base Parsing Function
	bool parse_bool(xmlpp::Element *node);
	//! Segment Value Base Parsing Function
	Segment parse_segment(xmlpp::Element *node);
	//! List Value Base Parsing Function
	ValueBase parse_list(xmlpp::Element *node,Canvas::Handle canvas);
	//! Weighted Value Base Parsing Function
	ValueBase parse_weighted_value(xmlpp::Element *node, types_namespace::TypeWeightedValueBase &type, Canvas::Handle canvas);
	//! Pair Value Base Parsing Function
	ValueBase parse_pair(xmlpp::Element *node, types_namespace::TypePairBase &type, Canvas::Handle canvas);
	//! Gradient Value Base Parsing Function
	Gradient parse_gradient(xmlpp::Element *node);
	//! Bline Point Value Base Parsing Function
	BLinePoint parse_bline_point(xmlpp::Element *node);
	//! Transformation Value Base Parsing Function
	Transformation parse_transformation(xmlpp::Element *node);

	GUID parse_guid(xmlpp::Element *node);

	//! Width Point Value Base Parsing Function
	WidthPoint parse_width_point(xmlpp::Element *node);
	//! Dash Item Value Base Parsing Function
	DashItem parse_dash_item(xmlpp::Element *node);

	//! Keyframe Parsing Function
	Keyframe parse_keyframe(xmlpp::Element *node,Canvas::Handle canvas);

	//! ValueNode Animated Parsing Function
	etl::handle<ValueNode_Animated> parse_animated(xmlpp::Element *node,Canvas::Handle canvas);
	//! Linkable ValueNode Parsing Function
	etl::handle<LinkableValueNode> parse_linkable_value_node(xmlpp::Element *node,Canvas::Handle canvas);

	//! Static List Parsnig Function
	etl::handle<ValueNode_StaticList> parse_static_list(xmlpp::Element *node,Canvas::Handle canvas);

	//! Dynamic List Parsnig Function
	etl::handle<ValueNode_DynamicList> parse_dynamic_list(xmlpp::Element *node,Canvas::Handle canvas);

	//! Interpolation option for ValueBase parsing function
	Interpolation parse_interpolation(xmlpp::Element *node, String attribute);
	//! Static option for ValueBase parsing function
	bool parse_static(xmlpp::Element *node);

}; // END of CanvasParser

/* === E X T E R N S ======================================================= */

//!	Loads a canvas from current xmlpp Element
/*!	\return	The Canvas's handle on success, an empty handle on failure */
extern Canvas::Handle open_canvas(xmlpp::Element* node,String &errors,String &warnings);
//!	Loads a canvas from \a filename and its absolute path
/*!	\return	The Canvas's handle on success, an empty handle on failure */
extern Canvas::Handle open_canvas_as(const FileSystem::Identifier &identifier,const String &as,String &errors,String &warnings);

//! Returns the Open Canvases Map.
//! \see open_canvas_map_
std::map<String, etl::loose_handle<Canvas> >& get_open_canvas_map();

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
