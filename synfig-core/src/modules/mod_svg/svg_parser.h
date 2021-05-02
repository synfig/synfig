/* === S Y N F I G ========================================================= */
/*!	\file svg_parser.h
**	\brief Implementation of the Svg parser
**	\brief Based on SVG XML specification 1.1
**	\brief See: http://www.w3.org/TR/xml11/ for deatils
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**	Copyright (c) 2009 Carlos A. Sosa Navarro
**	Copyright (c) 2009 Nikita Kitaev
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

typedef struct matrix_t{
	float a,c,e;
	float b,d,f;
}SVGMatrix;

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
	SVGMatrix *transform;

	LinearGradient(const String &name, float x1, float y1, float x2, float y2, std::list<ColorStop> stops, SVGMatrix* transform);
};

typedef struct radial_g{
	char name[80];
	float cx,cy;//center point
	//float fx,fy; //not supported by Synfig
	float r; //radius
	std::list<ColorStop> stops;
	SVGMatrix *transform;
}RadialGradient;

typedef struct url_t{
	int type;
	void* data;
}URL;

struct Vertex{
   	float x,y;
	float radius1,angle1;
	float radius2,angle2;
	bool split;

	void setTg2(float p2x, float p2y);
	void setTg1(float p2x, float p2y);
	void setSplit(bool val);
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
		std::list<RadialGradient*> rg;

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
		void parser_graphics(const xmlpp::Node* node,xmlpp::Element* root,String parent_style,SVGMatrix* mtx_parent);

		/* === LAYER PARSERS ============================== */
		void parser_layer(const xmlpp::Node* node,xmlpp::Element* root,String parent_style,SVGMatrix* mtx);
		void parser_rect(const xmlpp::Element* nodeElement,xmlpp::Element* root,String fill, String fill_opacity, String opacity);
		/* === CONVERT TO PATH PARSERS ==================== */
		std::list<BLine> parser_path_polygon(Glib::ustring polygon_points, SVGMatrix* mtx);
		std::list<BLine> parser_path_d(String path_d, SVGMatrix* mtx);

		/* === EFFECTS PARSERS ============================ */
		void parser_effects(const xmlpp::Element* nodeElement,xmlpp::Element* root,String parent_style,SVGMatrix* mtx);

		/* === DEFS PARSERS =============================== */
		void parser_defs(const xmlpp::Node* node);
		void parser_linearGradient(const xmlpp::Node* node);
		void parser_radialGradient(const xmlpp::Node* node);
		RadialGradient* newRadialGradient(String name, float cx, float cy, float r, std::list<ColorStop> stops, SVGMatrix* transform);

		/* === BUILDS ===================================== */
		void build_transform(xmlpp::Element* root,SVGMatrix* mtx);
		std::list<ColorStop> get_colorStop(String name);
		void build_fill(xmlpp::Element* root, String name,SVGMatrix *mtx);
		void build_radialGradient(xmlpp::Element* root,RadialGradient* data,SVGMatrix* mtx);
		void build_linearGradient(xmlpp::Element* root, const LinearGradient& data, SVGMatrix* mtx);
		void build_stop_color(xmlpp::Element* root, const std::list<ColorStop>& stops);
		Color adjustGamma(float r,float g,float b,float a);

		void build_gamma(xmlpp::Element* root,float gamma);
		void build_rotate(xmlpp::Element* root,float dx,float dy,float angle);
		void build_translate(xmlpp::Element* root,float dx,float dy);
		void build_points(xmlpp::Element* root,std::list<Vertex*> p);
		void build_vertex(xmlpp::Element* root, const Vertex& p);
		void build_bline(xmlpp::Element* root, std::list<Vertex> p, bool loop, String blineguid);
		void build_param (xmlpp::Element* root,String name,String type,String value);
		void build_param (xmlpp::Element* root,String name,String type,float value);
		void build_param (xmlpp::Element* root,String name,String type,int value);
		void build_integer (xmlpp::Element* root,String name, int value);
		void build_real (xmlpp::Element* root,String name,float value);
		void build_vector (xmlpp::Element* root,String name,float x,float y);
		void build_vector (xmlpp::Element* root,String name,float x,float y,String guid);
		void build_color(xmlpp::Element* root,float r,float g,float b,float a);
		xmlpp::Element* nodeStartBasicLayer(xmlpp::Element* root);
		xmlpp::Element* nodeStartBasicLayer(xmlpp::Element* root, String name);

		/* === COORDINATES & TRANSFORMATIONS ============== */

		//points,etc
		void coor2vect(float *x,float *y);

		//matrix operations
		SVGMatrix* parser_transform(const String transform);
		SVGMatrix* newSVGMatrix(float a,float b,float c,float d,float e,float f);
		SVGMatrix* newSVGMatrix(const String mvector);
		SVGMatrix* newSVGMatrix(SVGMatrix *a);
		void transformPoint2D(SVGMatrix *mtx,float *a,float *b);
		bool matrixIsNull(SVGMatrix* mtx);
		void composeSVGMatrix(SVGMatrix **mtx,SVGMatrix *mtx1,SVGMatrix *mtx2);
		void multiplySVGMatrix(SVGMatrix **mtx1,SVGMatrix *mtx2);

		/* === EXTRA METHODS ============================== */

		String new_guid();
};
// END of Svg_parser class

/* === E X T E R N S ======================================================= */

extern Canvas::Handle open_svg(std::string _filepath,String &errors, String &warnings);

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
