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

typedef struct stop_t{
	 float r,g,b;
	 float a;
	 float pos;
}ColorStop;
typedef struct linear_g{
	char name[80];
	float x1,x2,y1,y2;
	std::list<ColorStop*> *stops;
	SVGMatrix *transform;
}LinearGradient;
typedef struct radial_g{
	char name[80];
	float cx,cy;//center point
	//float fx,fy; //not supported by Synfig
	float r; //radius
	std::list<ColorStop*> *stops;
	SVGMatrix *transform;
}RadialGradient;

typedef struct url_t{
	int type;
	void* data;
}URL;

typedef struct Vertex_t{
   	float x,y;
	float radius1,angle1;
	float radius2,angle2;
	bool split;
}Vertex;

typedef struct bline_t{
	std::list<Vertex*> *points;
	bool loop;
	String* bline_id;
	String* offset_id;
}BLine;

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
		Glib::ustring width;
		Glib::ustring height;
		Glib::ustring docname;
		int uid;
		int kux,set_canvas;
		float ox,oy;
		//urls
		std::list<LinearGradient*> lg;
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
		std::list<BLine *> parser_path_polygon(Glib::ustring polygon_points, SVGMatrix* mtx);
		std::list<BLine *> parser_path_d(String path_d,SVGMatrix* mtx);

		/* === EFFECTS PARSERS ============================ */
		void parser_effects(const xmlpp::Element* nodeElement,xmlpp::Element* root,String parent_style,SVGMatrix* mtx);

		/* === DEFS PARSERS =============================== */
		void parser_defs(const xmlpp::Node* node);
		void parser_linearGradient(const xmlpp::Node* node);
		void parser_radialGradient(const xmlpp::Node* node);
		ColorStop* newColorStop(String color,float opacity,float pos);
		LinearGradient* newLinearGradient(String name,float x1,float y1, float x2,float y2,std::list<ColorStop*> *stops, SVGMatrix* transform);
		RadialGradient* newRadialGradient(String name,float cx,float cy,float r,std::list<ColorStop*> *stops, SVGMatrix* transform);
		BLine* newBLine(std::list<Vertex*> *points,bool loop);

		/* === BUILDS ===================================== */
		void build_transform(xmlpp::Element* root,SVGMatrix* mtx);
		std::list<ColorStop*>* find_colorStop(String name);
		void build_fill(xmlpp::Element* root, String name,SVGMatrix *mtx);
		void build_linearGradient(xmlpp::Element* root,LinearGradient* data,SVGMatrix* mtx);
		void build_radialGradient(xmlpp::Element* root,RadialGradient* data,SVGMatrix* mtx);
		void build_stop_color(xmlpp::Element* root, std::list<ColorStop*> *stops);
		void build_stop_color(xmlpp::Element* root, std::list<ColorStop*> *stops,String name);
		Color adjustGamma(float r,float g,float b,float a);

		void build_gamma(xmlpp::Element* root,float gamma);
		void build_rotate(xmlpp::Element* root,float dx,float dy,float angle);
		void build_translate(xmlpp::Element* root,float dx,float dy);
		void build_points(xmlpp::Element* root,std::list<Vertex*> p);
		void build_vertex(xmlpp::Element* root , Vertex *p);
		void build_bline(xmlpp::Element* root,std::list<Vertex*> p,bool loop,String blineguid);
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
		void setTg2(Vertex* p,float p1x,float p1y,float p2x,float p2y);
		void setTg1(Vertex *p,float p1x,float p1y,float p2x,float p2y);
		void setSplit(Vertex* p,bool val);
		int isFirst(Vertex* nodo,float a, float b);
		Vertex* newVertex(float x,float y);

		//matrix operations
		SVGMatrix* parser_transform(const String transform);
		SVGMatrix* newSVGMatrix(float a,float b,float c,float d,float e,float f);
		SVGMatrix* newSVGMatrix(const String mvector);
		SVGMatrix* newSVGMatrix(SVGMatrix *a);
		void transformPoint2D(SVGMatrix *mtx,float *a,float *b);
		bool matrixIsNull(SVGMatrix* mtx);
		void composeSVGMatrix(SVGMatrix **mtx,SVGMatrix *mtx1,SVGMatrix *mtx2);
		void multiplySVGMatrix(SVGMatrix **mtx1,SVGMatrix *mtx2);
		float getRadian(float sexa);

		/* === EXTRA METHODS ============================== */

		//attributes
		int extractSubAttribute(const String attribute, String name,String* value);
		String loadAttribute(String name,const String path_style,const String master_style,const String subattribute,const String defaultVal);
		String loadAttribute(String name,const String path_style,const String master_style,const String defaultVal);
		std::vector<String> get_tokens_path(String path);
		int randomLetter();
		int getRed(String hex);
		int getGreen(String hex);
		int getBlue(String hex);
		int hextodec(const std::string& hex);
		int getColor(String name, int position);
		float getDimension(const String ac);
		//string functions
		void removeS(String *input);
		void removeIntoS(String *input);
		std::vector<String> tokenize(const String& str,const String& delimiters);
		void get_canvas_name(String _filepath);
		String new_guid();
};
// END of Svg_parser class

/* === E X T E R N S ======================================================= */

extern Canvas::Handle open_svg(std::string _filepath,String &errors, String &warnings);

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
