/* === S Y N F I G ========================================================= */
/*!	\file svg_parser.h
**	\brief Implementation of the Svg parser
**	\brief Based on SVG XML specification 1.1
**	\brief See: http://www.w3.org/TR/xml11/ for deatils
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Carlos A. Sosa Navarro
**	Copyright (c) 2009 Nikita Kitaev
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

#ifndef __SVG_PARSER_H
#define __SVG_PARSER_H

/* === H E A D E R S ======================================================= */

#include <glibmm/ustring.h>

#include <libxml++/libxml++.h>
#include <synfig/canvas.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig{

struct SVGMatrix {
	float a,c,e;
	float b,d,f;

	SVGMatrix();
	SVGMatrix(float a, float b, float c, float d, float e, float f);
	SVGMatrix(const String& mvector);

	static const SVGMatrix identity;
	static const SVGMatrix zero;

	bool is_identity() const;

	void transformPoint2D(float &x, float &y) const;
	void compose(const SVGMatrix& mtx1, const SVGMatrix& mtx2);
	void multiply(const SVGMatrix &mtx2);

	void parser_transform(String transform);
};

struct ColorStop {
	 float r,g,b;
	 float a;
	 float pos;

	 ColorStop(const String& color, float opacity, const Gamma& gamma, float pos);
};

struct LinearGradient{
	char name[80];
	float x1,x2,y1,y2;
	std::list<ColorStop> stops;
	SVGMatrix transform;

	LinearGradient(const String &name, float x1, float y1, float x2, float y2, std::list<ColorStop> stops, SVGMatrix transform);
};

struct RadialGradient{
	char name[80];
	float cx,cy;//center point
	//float fx,fy; //not supported by Synfig
	float r; //radius
	std::list<ColorStop> stops;
	SVGMatrix transform;

	RadialGradient(const String& name, float cx, float cy, float r, std::list<ColorStop> stops, SVGMatrix transform);
};

typedef struct url_t{
	int type;
	void* data;
}URL;

struct Vertex{
   	float x,y;
	float radius1,angle1;
	float radius2,angle2;
	bool split_radius;
	bool split_angle;

	void setTg2(float p2x, float p2y);
	void setTg1(float p2x, float p2y);
	void setSplit(bool val);
	void setSplitRadius(bool val);
	void setSplitAngle(bool val);
	bool isFirst(float a, float b) const;
	Vertex(float x,float y);
};

struct BLine {
	std::list<Vertex> points;
	bool loop;
	String bline_id;
	String offset_id;

	BLine(std::list<Vertex> points, bool loop);
};

struct Style {
	/// Merge with style definitions from elem (style attribute and presentation attributes)
	void merge(const xmlpp::Element* elem);
	/// Merge with style property
	void merge(const std::string& property, const std::string& value);

	/// Retrieve a style property
	/// \param property Property name.
	/// \param default_value The value to be used, if this Style does not have this property explicitly set
	std::string get(const std::string& property, std::string default_value) const;
	/// This method retrieve a property value, like get() does, but it returns
	/// its numeric value (and it should be used only for numeric properties.
	///
	/// If property is a numerical number, its value should be parsed and computed.
	/// \param property Property name.
	/// \param default_value The value to be used, if this Style does not have this property explicitly set
	/// \param reference_value If property may be percentual, what is the percentage reference (what '100%' means)
	/// \return the property value as a real number
	double compute(const std::string& property, std::string default_value, double reference_value = 1.0) const;
private:
	 std::map<std::string, std::string> data;

	 void merge_presentation_attributes(const xmlpp::Element* elem);
	 void merge_style_string(const std::string& style_str);

	 void push(const std::string& property, const std::string& value);
};

class Svg_parser
{
		//this is inkscape oriented in some cases
public:

private:
		Gamma gamma;
	 	String filepath;
	 	String id_name;
		xmlpp::DomParser parser;
		xmlpp::Document document;
		xmlpp::Element* nodeRoot;//output
		double width;
		double height;
		Glib::ustring docname;
		int uid;
		int kux;
		bool set_canvas;
		double ox,oy;
		//urls
		std::list<LinearGradient> lg;
		std::list<RadialGradient> rg;

public:
		explicit Svg_parser(const Gamma &gamma = Gamma());
		Canvas::Handle load_svg_canvas(std::string _filepath,String &errors, String &warnings);
		//String get_id();
		//void set_id(String source);

private:
		/* === PARSERS ==================================== */
		void parser_node(const xmlpp::Node* node);
		//parser headers
		void parser_svg(const xmlpp::Node* node);
		void parser_canvas(const xmlpp::Node* node);
		void parser_graphics(const xmlpp::Node* node, xmlpp::Element* root, Style style, const SVGMatrix& mtx_parent);

		/* === LAYER PARSERS ============================== */
		void parser_layer(const xmlpp::Node* node, xmlpp::Element* root, Style style, const SVGMatrix& mtx);
		void parser_rect(const xmlpp::Element* nodeElement, xmlpp::Element* root, const Style& style);
		/* === CONVERT TO PATH PARSERS ==================== */
		std::list<BLine> parser_path_polygon(const Glib::ustring& polygon_points, const SVGMatrix& mtx);
		std::list<BLine> parser_path_d(const String& path_d, const SVGMatrix& mtx);

		/* === EFFECTS PARSERS ============================ */
		void parser_effects(const xmlpp::Element* nodeElement, xmlpp::Element* root, const Style& parent_style, const SVGMatrix& mtx);

		/* === DEFS PARSERS =============================== */
		void parser_defs(const xmlpp::Node* node);
		void parser_linearGradient(const xmlpp::Node* node);
		void parser_radialGradient(const xmlpp::Node* node);

		/* === BUILDS ===================================== */
		void build_region(xmlpp::Node* root, Style style, const std::list<BLine>& k, const String& desc);
		void build_outline(xmlpp::Node* root, Style style, const std::list<BLine>& k, const String& desc, const SVGMatrix& mtx);
		void build_transform(xmlpp::Element* root, const SVGMatrix& mtx);
		std::list<ColorStop> get_colorStop(String name);
		void build_fill(xmlpp::Element* root, String name, const SVGMatrix& mtx);
		void build_linearGradient(xmlpp::Element* root, const LinearGradient& data, const SVGMatrix& mtx);
		void build_radialGradient(xmlpp::Element* root, const RadialGradient& data, const SVGMatrix& mtx);
		void build_stop_color(xmlpp::Element* root, const std::list<ColorStop>& stops);
		Color adjustGamma(float r,float g,float b,float a);

		void build_gamma(xmlpp::Element* root,float gamma);
		void build_rotate(xmlpp::Element* root,float dx,float dy,float angle);
		void build_translate(xmlpp::Element* root,float dx,float dy);
		void build_points(xmlpp::Element* root, const std::list<Vertex>& p);
		void build_vertex(xmlpp::Element* root, const Vertex& p);
		void build_bline(xmlpp::Element* root, const std::list<Vertex>& p, bool loop, const String& blineguid);
		void build_param (xmlpp::Element* root, const String& name, const String& type, const String& value);
		void build_param (xmlpp::Element* root, const String& name, const String& type, float value);
		void build_param (xmlpp::Element* root, const String& name, const String& type, int value);
		void build_integer (xmlpp::Element* root, const String& name, int value);
		void build_real (xmlpp::Element* root, const String& name, float value);
		void build_vector (xmlpp::Element* root, const String& name, float x, float y);
		void build_vector (xmlpp::Element* root, const String& name, float x, float y, const String& guid);
		void build_color(xmlpp::Element* root,float r,float g,float b,float a);
		xmlpp::Element* initializeGroupLayerNode(xmlpp::Element* root, const String& name);

		/* === COORDINATES & TRANSFORMATIONS ============== */

		//points,etc
		void coor2vect(float *x,float *y);

		/* === EXTRA METHODS ============================== */

		String new_guid();
};
// END of Svg_parser class

/* === E X T E R N S ======================================================= */

extern Canvas::Handle open_svg(std::string _filepath,String &errors, String &warnings);

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
