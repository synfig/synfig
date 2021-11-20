/* === S Y N F I G ========================================================= */
/*!	\file svg_parser.cpp
**	\brief Implementation of the Svg parser
**	\brief Based on SVG XML specification 1.1
**	\brief See: http://www.w3.org/TR/xml11/ for deatils
**
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>

#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/general.h>
#include <synfig/loadcanvas.h>
#include <synfig/localization.h>
#include <synfig/string_helper.h>
#include <unordered_map>

#include "svg_parser.h"
#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === G L O B A L S ======================================================= */

//PARSER PREFERENCES

//Separate transformations: apply transformations on a per-layer basis, rather than on canvases
#define SVG_SEP_TRANSFORMS 1

//Resolve BLine transformations: resolve transformations instead of creating transformation layers
#define SVG_RESOLVE_BLINE 1

/* === P R O C E D U R E S ================================================= */

//attributes
static std::vector<String> get_tokens_path(const String& path);
static int getRed(const String& hex);
static int getGreen(const String& hex);
static int getBlue(const String& hex);
static int hextodec(const std::string& hex);
static int getColor(const String& name, int position);
static double getDimension(const String& ac, bool use_90_ppi = false);
static float getRadian(float sexa);
//string functions
static std::vector<String> tokenize(const String& str,const String& delimiters);

static float get_inkscape_version(const xmlpp::Element* svgNodeElement);

/* === M E T H O D S ======================================================= */

Canvas::Handle
synfig::open_svg(std::string _filepath, String &errors, String &warnings)
{
	Canvas::Handle canvas;
	Svg_parser parser;
	try
	{
		canvas=parser.load_svg_canvas(_filepath,errors,warnings);
		//canvas->set_id(parser.get_id());
	}catch(...){
		synfig::error("SVG Parser: error loading SVG");
	}
	return canvas;
}

Canvas::Handle
Svg_parser::load_svg_canvas(const std::string& filepath, String &errors, String &warnings)
{
	ChangeLocale locale(LC_NUMERIC, "C");

	#ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  	try{
  	#endif //LIBXMLCPP_EXCEPTIONS_ENABLED
		//load parser
		xmlpp::DomParser parser;
		parser.set_substitute_entities();
		parser.parse_file(filepath);
		//set_id(filepath);
		if(parser){
		  	const xmlpp::Node* pNode = parser.get_document()->get_root_node();
		  	parser_node(pNode);
		}
	#ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  	}catch(const std::exception& ex){
    	synfig::error("SVG Parser: exception caught: %s", ex.what());
  	}
  	#endif //LIBXMLCPP_EXCEPTIONS_ENABLED
	Canvas::Handle canvas;
	if(nodeRoot){
		//canvas=synfig::open_canvas(nodeRoot,_filepath,errors,warnings);
		canvas=synfig::open_canvas(nodeRoot,errors,warnings);
	}
	return canvas;
}

Svg_parser::Svg_parser(const Gamma &gamma):
	gamma(gamma),
	nodeRoot(NULL),
	width(0),
	height(0),
	kux(60),
	set_canvas(false), //we must run parser_canvas method
	ox(0),
	oy(0)
{
}
/*
String
Svg_parser::get_id(){
	if(!id_name.empty()) return id_name;
	return "random_id";
}
void
Svg_parser::set_id(String source){
	const char bad_chars[]=" :#@$^&()*";
	int start= 	source.find_last_of('/')+1;
	int end=	source.find_last_of('.');
	String x=source.substr(start,end-start);
	if(!x.empty()){
		for(unsigned int i=0;i<sizeof(bad_chars);i++){
			unsigned int pos=x.find_first_of(bad_chars[i]);
			if(pos!=String::npos)
				x.erase(pos,1);
		}
	}
	if(!x.empty()){
		id_name=x;
	}else{
		id_name="id_arbitrario";
	}
}
*/
//UPDATE


/* === PARSERS ============================================================= */

void
Svg_parser::parser_node(const xmlpp::Node* node)
{
  	const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  	const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  	const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  	if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
    	return;

  	Glib::ustring nodename = node->get_name();
  	if(!nodeText && !nodeComment && !nodename.empty()){
		if(nodename.compare("svg")==0){
			parser_svg (node);
		}else if(nodename.compare("namedview")==0){
			parser_canvas(node);
		}else if(nodename.compare("defs")==0){
			parser_defs (node);
		}else{
			if(!set_canvas) parser_canvas(node);
			parser_graphics(node,nodeRoot,Style(),SVGMatrix::identity);
			if(nodename.compare("g")==0) return;
		}
  	}
  	if(!nodeContent){
    	xmlpp::Node::NodeList list = node->get_children();
    	for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
      		parser_node(*iter); //recursive
    	}
  	}
}

//parser elements
void
Svg_parser::parser_svg(const xmlpp::Node* node)
{
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		float inkscape_version = get_inkscape_version(nodeElement);

		width = getDimension(nodeElement->get_attribute_value("width"), inkscape_version < 0.92f && approximate_not_zero(inkscape_version));
		height = getDimension(nodeElement->get_attribute_value("height"), inkscape_version < 0.92f && approximate_not_zero(inkscape_version));
	}
}

void
Svg_parser::parser_canvas(const xmlpp::Node* node)
{
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){

		if(approximate_zero(width)){
			try {
				width=std::stod(nodeElement->get_attribute_value("width",""));
			} catch (...)
			{}
		}
		if(approximate_zero(height)){
			try {
				height=std::stod(nodeElement->get_attribute_value("height",""));
			} catch (...)
			{}
		}
		if(approximate_zero(width) && approximate_not_zero(height)){
			width=height;
		}
		if(approximate_not_zero(width) && approximate_zero(height)){
			height=width;
		}
		if(approximate_zero(height) && approximate_zero(width)){
			width=1024;
			height=768;
		}
		//build
		nodeRoot=document.create_root_node("canvas", "", "");
		nodeRoot->set_attribute("version","0.5");
		nodeRoot->set_attribute("width",etl::strprintf("%lf", width));
		nodeRoot->set_attribute("height",etl::strprintf("%lf", height));
		nodeRoot->set_attribute("xres","2834.645752");
		nodeRoot->set_attribute("yres","2834.645752");
		double view_x = width/kux;
		double view_y = height/kux;
		view_x /= 2.0;
		view_y /= 2.0;
		char attr_view_box[60];
		sprintf(attr_view_box,"%f %f %f %f",-1.0*view_x,view_y,view_x,-1.0*view_y);
		nodeRoot->set_attribute("view-box",attr_view_box);
		ox = width/2;
		oy = height/2;
		nodeRoot->set_attribute("antialias","1");
		nodeRoot->set_attribute("fps","24.000");
		nodeRoot->set_attribute("begin-time","0f");
		nodeRoot->set_attribute("end-time","5s");
		nodeRoot->set_attribute("bgcolor","0.500000 0.500000 0.500000 1.000000");
		if(!id_name.empty()) nodeRoot->add_child("name")->set_child_text(id_name);
		else nodeRoot->add_child("name")->set_child_text(_("Synfig Animation 1"));
	}
	set_canvas=true;
}

void
Svg_parser::parser_graphics(const xmlpp::Node* node, xmlpp::Element* root, Style style, const SVGMatrix& mtx_parent)
{
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		const Glib::ustring nodename = node->get_name();

		// Is element known ?
		const std::vector<const char*> valid_elements = {"g", "path", "polygon", "rect", "circle", "ellipse", "line", "polyline"};
		if (valid_elements.end() == std::find(valid_elements.begin(), valid_elements.end(), nodename))
			return;

		// Resolve transformations
		SVGMatrix mtx;
		{
			Glib::ustring transform	= nodeElement->get_attribute_value("transform");
			if(!transform.empty())
				mtx.parser_transform(transform);
			if (SVG_SEP_TRANSFORMS)
				mtx.compose(mtx_parent, mtx);
		}

		// Resolve styles
		style.merge(nodeElement);

		// Is it a group element?
		if(nodename.compare("g")==0){
			parser_layer(node,root->add_child("layer"),style,mtx);
			return;
		}

		// Shape elements

		enum FillType {FILL_TYPE_NONE, FILL_TYPE_SIMPLE, FILL_TYPE_GRADIENT};

		//load sub-attributes
		Glib::ustring id  = nodeElement->get_attribute_value("id");

		//style
		String fill       = style.get("fill", "#000");
		String stroke     = style.get("stroke", "none");

		//Fill
		FillType typeFill = FILL_TYPE_NONE;

		if (nodename.compare("line") == 0)
			fill = "none";

		if(fill.compare("none")!=0){
			typeFill = FILL_TYPE_SIMPLE;

			if(fill.compare(0,3,"url")==0)
				typeFill = FILL_TYPE_GRADIENT;
		}

		//Stroke
		int typeStroke = FILL_TYPE_NONE;

		if(stroke.compare("none")!=0){
			typeStroke = FILL_TYPE_SIMPLE;

			if (stroke.compare(0,3,"url")==0)
				typeStroke = FILL_TYPE_GRADIENT;
		}

		// What?
		if (typeFill == FILL_TYPE_NONE && typeStroke == FILL_TYPE_NONE)
			return;

		xmlpp::Element* child_layer = root;
		xmlpp::Element* child_fill = nullptr;
		xmlpp::Element* child_stroke = nullptr;

		// Only fill and a simple geometric shape? Render as Synfig primitive.
		// If it has stroke, render as a region, instead of standard shape, to be able to link to outline
		if(typeFill != FILL_TYPE_NONE && typeStroke == FILL_TYPE_NONE) {
			if (nodename.compare("rect") == 0 || nodename.compare("circle") == 0) {
				if (!mtx.is_identity())
					child_layer = initializeGroupLayerNode(root->add_child("layer"), id);
				child_fill=child_layer;

				if (nodename.compare("rect") == 0)
					parser_rect(nodeElement,child_fill,style);
				else if (nodename.compare("circle") == 0)
					parser_circle(nodeElement,child_fill,style);

				if(typeFill == FILL_TYPE_GRADIENT){
					build_fill (child_fill,fill,SVGMatrix::identity);
				}
				parser_effects(nodeElement,child_layer,style,mtx);
				return;
			}
		}

		// We will create a non-primitive shape

		if (!SVG_RESOLVE_BLINE)
			child_layer = initializeGroupLayerNode(root->add_child("layer"), id);
		child_fill=child_layer;
		child_stroke=child_layer;

		// bline list
		std::list<BLine> k;
		// transformation matrix for bline vertices
		const SVGMatrix& bline_matrix = SVG_RESOLVE_BLINE ? mtx : SVGMatrix::identity;

		//First, create the list of vertices
		if(nodename.compare("path")==0)
			k = parser_path_d(nodeElement->get_attribute_value("d"), bline_matrix);
		else if(nodename.compare("polygon")==0)
			k = parser_path_polygon(nodeElement->get_attribute_value("points"), bline_matrix);
		else if(nodename.compare("rect")==0)
			k = parser_path_rect(nodeElement, style, mtx);
		else if(nodename.compare("circle")==0)
			k = parser_path_circle(nodeElement, style, mtx);
		else if(nodename.compare("ellipse")==0)
			k = parser_path_ellipse(nodeElement, style, mtx);
		else if(nodename.compare("line")==0)
			k = parser_line(nodeElement, mtx);
		else if(nodename.compare("polyline")==0)
			k = parser_polyline(nodeElement, mtx);

		if (k.empty())
			return;

		// Region layer
		auto build_region_func = [&]() {
			if (typeFill == FILL_TYPE_NONE)
				return;

			if(typeFill==FILL_TYPE_GRADIENT){
				child_fill=initializeGroupLayerNode(child_fill->add_child("layer"),"fill");
			}

			build_region(child_fill, style, k, id);

			if(typeFill==FILL_TYPE_GRADIENT){ //gradient in onto mode (fill)
				build_fill(child_fill, fill, bline_matrix);
			}
		};

		// Outline layer
		auto build_outline_func = [&]() {
			if (typeStroke == FILL_TYPE_NONE)
				return;

			if(typeStroke==FILL_TYPE_GRADIENT){
				child_stroke=initializeGroupLayerNode(child_stroke->add_child("layer"),"stroke");
			}

			build_outline(child_stroke, style, k, id, bline_matrix);

			if(typeStroke==FILL_TYPE_GRADIENT){ //gradient in onto mode (stroke)
				build_fill(child_stroke, stroke, bline_matrix);
			}
		};

		bool fill_under_stroke = true;
		if (typeFill != FILL_TYPE_NONE && typeStroke != FILL_TYPE_NONE) {
			std::vector<std::string> paint_order_tokens = {"fill", "stroke", "markers"};
			std::string paint_order_str = style.get("paint-order", "normal");
			if (paint_order_str != "normal")
				paint_order_tokens = tokenize(paint_order_str, ", \x09\x0a\x0d");

			// if any element is missing they are appended at end in the default order after the specified ones
			auto fill_iter = std::find(paint_order_tokens.begin(), paint_order_tokens.end(), "fill");
			auto stroke_iter = std::find(paint_order_tokens.begin(), paint_order_tokens.end(), "stroke");
//			auto markers_iter = std::find(paint_order_tokens.begin(), paint_order_tokens.end(), "markers");

			fill_under_stroke = (fill_iter == paint_order_tokens.end() && stroke_iter == paint_order_tokens.end()) || fill_iter < stroke_iter;
		}

		if (fill_under_stroke) {
			build_region_func();
			build_outline_func();
		} else {
			build_outline_func();
			build_region_func();
		}

		if (SVG_RESOLVE_BLINE)
			parser_effects(nodeElement,child_layer,style,SVGMatrix::identity);
		else
			parser_effects(nodeElement,child_layer,style,mtx);
	}
}

bool
Svg_parser::parser_rxry_property(const Style& style, double width_reference, double height_reference, double &rx, double &ry)
{
	rx = 0.;
	ry = 0.;

	String rx_str = style.get("rx", "auto");
	String ry_str = style.get("ry", "auto");

	if (rx_str != "auto" || ry_str != "auto") {

		if (rx_str != "auto" && !rx_str.empty()) {
			rx = std::stod(rx_str);
			if (rx < 0) {
				synfig::error("SVG Parser: Invalid rx value: it cannot be negative!");
				return false;
			}
			if (rx_str.back() == '%')
				rx *= 0.01 * width_reference;
		}

		if (ry_str == "auto") {
			ry = rx;
		} else if (!ry_str.empty()){
			ry = std::stod(ry_str);
			if (ry < 0) {
				synfig::error("SVG Parser: Invalid ry value: it cannot be negative!");
				return false;
			}
			if (ry_str.back() == '%')
				ry *= 0.01 * height_reference;
		}

		if (rx_str == "auto") {
			rx = ry;
		}
	}
	return true;
}


void
Svg_parser::build_region(xmlpp::Node* root, Style style, const std::list<BLine>& k, const String& desc)
{
	String fill          = style.get("fill", "none");
	String fill_rule     = style.get("fill-rule", "evenodd");
	String fill_opacity  = style.get("fill-opacity", "1");
	String opacity       = style.get("opacity", "1");

	for (const BLine& bline : k) {
		xmlpp::Element *child_region=root->add_child("layer");
		child_region->set_attribute("type","region");
		child_region->set_attribute("active","true");
		child_region->set_attribute("version","0.1");
		child_region->set_attribute("desc",desc);
		build_param (child_region->add_child("param"),"z_depth","real","0.0000000000");
		build_param (child_region->add_child("param"),"amount","real","1.0000000000");
		build_param (child_region->add_child("param"),"blend_method","integer","0");
		build_color (child_region->add_child("param"),getRed(fill),getGreen(fill),getBlue(fill),atof(fill_opacity.data())*atof(opacity.data()));
		build_vector (child_region->add_child("param"),"offset",0,0, bline.offset_id );
		build_param (child_region->add_child("param"),"invert","bool","false");
		build_param (child_region->add_child("param"),"antialias","bool","true");
		build_param (child_region->add_child("param"),"feather","real","0.0000000000");
		build_param (child_region->add_child("param"),"blurtype","integer","1");
		if(fill_rule.compare("evenodd")==0) build_param (child_region->add_child("param"),"winding_style","integer","1");
		else build_param (child_region->add_child("param"),"winding_style","integer","0");

		build_bline(child_region->add_child("param"), bline.points, bline.loop, bline.bline_id);
	}
}

void
Svg_parser::build_outline(xmlpp::Node* root, Style style, const std::list<BLine>& k, const String& desc, const SVGMatrix& mtx)
{
	String stroke           = style.get("stroke", "none");
	String stroke_width     = style.get("stroke-width", "1px");
	String stroke_linecap   = style.get("stroke-linecap", "butt");
	String stroke_linejoin  = style.get("stroke-linejoin", "miter");
	String stroke_dasharray = style.get("stroke-dasharray", "none");
	String stroke_dashoffset= style.get("stroke-dashoffset", "0");
	String stroke_opacity   = style.get("stroke-opacity", "1");
	String opacity          = style.get("opacity", "1");

	const float scale_factor = sqrt(mtx.a*mtx.a + mtx.b*mtx.b);
	stroke_width=etl::strprintf("%f",getDimension(stroke_width)/kux * scale_factor);

	const float total_opacity = atof(stroke_opacity.data())*atof(opacity.data());

	const bool is_advanced_outline = stroke_linecap == "square" || stroke_linejoin == "bevel" || stroke_dasharray != "none";

	auto create_layer_node_with_common_info = [=] (const BLine& bline) -> xmlpp::Element*
	{
		xmlpp::Element *child_outline=root->add_child("layer");
		child_outline->set_attribute("active","true");
		child_outline->set_attribute("version","0.3");
		child_outline->set_attribute("desc",desc);
		build_param (child_outline->add_child("param"),"z_depth","real","0.0000000000");
		build_param (child_outline->add_child("param"),"amount","real","1.0000000000");
		build_param (child_outline->add_child("param"),"blend_method","integer","0");
		build_color (child_outline->add_child("param"),getRed(stroke),getGreen(stroke),getBlue(stroke),total_opacity);
		build_vector (child_outline->add_child("param"),"origin",0,0,bline.offset_id);
		build_param (child_outline->add_child("param"),"invert","bool","false");
		build_param (child_outline->add_child("param"),"antialias","bool","true");
		build_param (child_outline->add_child("param"),"feather","real","0.0000000000");
		build_param (child_outline->add_child("param"),"blurtype","integer","1");
		//outline in nonzero
		build_param (child_outline->add_child("param"),"winding_style","integer","0");

		build_bline(child_outline->add_child("param"), bline.points, bline.loop, bline.bline_id);

		build_param (child_outline->add_child("param"),"width","real",stroke_width);
		build_param (child_outline->add_child("param"),"expand","real","0.0000000000");
		return child_outline;
	};
	if (!is_advanced_outline) {
		for (const BLine& bline : k) {
			xmlpp::Element *child_outline = create_layer_node_with_common_info(bline);
			child_outline->set_attribute("type","outline");
			if(stroke_linejoin.compare("miter")==0) build_param (child_outline->add_child("param"),"sharp_cusps","bool","true");
			else build_param (child_outline->add_child("param"),"sharp_cusps","bool","false");
			if(stroke_linecap.compare("butt")==0){
				build_param (child_outline->add_child("param"),"round_tip[0]","bool","false");
				build_param (child_outline->add_child("param"),"round_tip[1]","bool","false");
			}else{
				build_param (child_outline->add_child("param"),"round_tip[0]","bool","true");
				build_param (child_outline->add_child("param"),"round_tip[1]","bool","true");
			}
			build_param (child_outline->add_child("param"),"homogeneous_width","bool","true");
		}
	} else {
		const std::map<std::string, int> linejoin_map = {
			{"miter", 0}, // Advanced_Outline::CuspType::TYPE_SHARP
			{"round", 1}, // Advanced_Outline::CuspType::TYPE_ROUNDED
			{"bevel", 2}, // Advanced_Outline::CuspType::TYPE_BEVEL
			{"arcs", 0},       // not equivalent
			{"miter-clip", 0}, // not equivalent
		};
		const std::map<std::string, int> linecap_map = {
			{"round", 1}, // WidthPoint::TYPE_ROUNDED
			{"square", 2},// WidthPoint::TYPE_SQUARED
			{"butt", 4}   // WidthPoint::TYPE_FLAT
		};
		int linejoin_value = linejoin_map.at("miter");
		{
			auto iter = linejoin_map.find(stroke_linejoin);
			if (iter != linejoin_map.end())
				linejoin_value = iter->second;
		}
		int linecap_value = linecap_map.at("butt");
		{
			auto iter = linecap_map.find(stroke_linecap);
			if (iter != linecap_map.end())
				linecap_value = iter->second;
		}

		float path_length = 0.f;
		if (!k.empty())
		{
			std::vector<synfig::BLinePoint> v;
			for (const auto& point : k.front().points) {
				v.push_back(BLinePoint());
				v.back().set_vertex(synfig::Point(point.x, point.y));
				v.back().set_tangent1(synfig::Point(point.radius1, point.angle1));
				v.back().set_tangent2(synfig::Point(point.radius2, point.angle2));
				v.back().set_split_tangent_angle(point.split_angle);
				v.back().set_split_tangent_radius(point.split_radius);
			}

			path_length = scale_factor * bline_length(v, k.front().loop, nullptr);
		}

		std::vector<float> dashes;

		float dash_sum = 0.f;
		for (String token : tokenize(stroke_dasharray, " ,\x09\x0a\x0d"))
		{
			if (token.empty())
				continue;

			float value = atof(token.c_str());
			if (value < 0.f) {
				// According to 13.5.6 of SVG2 specs, it leads to invalidate the dasharray
				// Section 11.4 of SVG 1.1 specs says it is an 'error'.
				// I'll follow SVG 2
				dashes.clear();
				break;
			}

			value *= scale_factor;

			// parse '%'
			// How to get pathLength attribute of <path> ?
			if (token.back() == '%') {
				value *= 0.01f * path_length;
			}

			dash_sum += value;
			dashes.push_back(value);
		}

		if (approximate_zero(dash_sum)) {
			// 'If all of the values in the list are zero, then the stroke is rendered as a solid line without any dashing.'
			dashes.clear();
		}

		if (dashes.size() % 2 == 1) {
			// 'If the list has an odd number of values, then it is repeated to yield an even number of values'
			dashes.insert(dashes.end(), dashes.begin(), dashes.end());
		}

		float dash_offset = atof(stroke_dashoffset.c_str());
		if (stroke_dashoffset.back() == '%') {
			// How to get pathLength attribute of <path> ?
			dash_offset *= 0.01f * path_length;
		}
		if (dash_offset < 0.f) {
			dash_offset = dash_sum - std::fabs(dash_offset);
		}
		dash_offset = std::fmod(dash_offset, dash_sum);
		if (stroke_dashoffset.back() != '%') {
			dash_offset /= kux;
		}

		for (const BLine& bline : k) {
			xmlpp::Element *child_outline = create_layer_node_with_common_info(bline);
			child_outline->set_attribute("type","advanced_outline");
			build_param (child_outline->add_child("param"),"cusp_type","integer",linejoin_value);
			build_param (child_outline->add_child("param"),"start_tip","integer",linecap_value);
			build_param (child_outline->add_child("param"),"end_tip","integer",linecap_value);

			// dash

			build_param (child_outline->add_child("param"),"dash_enabled","bool", dashes.empty() ? "false" : "true");
			if (!dashes.empty()) {
				build_dilist( child_outline->add_child("param"), dashes, linecap_value);
			}
			build_param (child_outline->add_child("param"),"dash_offset","real", dash_offset);

			build_param (child_outline->add_child("param"),"homogeneous","bool","true");
		}
	}
}

/* === LAYER PARSERS ======================================================= */

void
Svg_parser::parser_layer(const xmlpp::Node* node, xmlpp::Element* root, Style style, const SVGMatrix& mtx)
{
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring label		=nodeElement->get_attribute_value("label", "inkscape");
		Glib::ustring id		=nodeElement->get_attribute_value("id");

		style.merge(nodeElement);

		// group attributes
		root->set_attribute("type","group");
		root->set_attribute("active","true");
		root->set_attribute("version","0.1");
		if(label.empty()) label = !id.empty() ? id : _("Inline Canvas");
		root->set_attribute("desc", label);

		build_real(root->add_child("param"),"z_depth",0.0);
		build_real(root->add_child("param"),"amount",1.0);
		build_integer(root->add_child("param"),"blend_method",0);
		build_vector (root->add_child("param"),"origin",0,0);

		// canvas attributes
		xmlpp::Element *child_canvas=root->add_child("param");
		child_canvas->set_attribute("name","canvas");
		child_canvas=child_canvas->add_child("canvas");
		const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
		if(!nodeContent){
    		xmlpp::Node::NodeList list = node->get_children();
    		for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
				Glib::ustring name =(*iter)->get_name();
				parser_graphics (*iter,child_canvas,style,mtx);
    		}
  		}
		if (SVG_SEP_TRANSFORMS) parser_effects(nodeElement,child_canvas,style,SVGMatrix::identity);
		else parser_effects(nodeElement,child_canvas,style,mtx);
	}
}

void
Svg_parser::parser_rect(const xmlpp::Element* nodeElement,xmlpp::Element* root, const Style& style)
{
	Glib::ustring rect_id		=nodeElement->get_attribute_value("id");
	Glib::ustring rect_x		=nodeElement->get_attribute_value("x");
	Glib::ustring rect_y		=nodeElement->get_attribute_value("y");
	Glib::ustring rect_width	=nodeElement->get_attribute_value("width");
	Glib::ustring rect_height	=nodeElement->get_attribute_value("height");
	Glib::ustring fill       	= style.get("fill", "#000");

	xmlpp::Element *child_rect=root->add_child("layer");
	child_rect->set_attribute("type","rectangle");
	child_rect->set_attribute("active","true");
	child_rect->set_attribute("version","0.2");
	child_rect->set_attribute("desc",rect_id);

	build_real(child_rect->add_child("param"),"z_depth",0.0);
	build_real(child_rect->add_child("param"),"amount",1.0);
	build_integer(child_rect->add_child("param"),"blend_method",0);
	build_color(child_rect->add_child("param"),getRed(fill),getGreen(fill),getBlue(fill),style.compute("opacity", "1")*style.compute("fill_opacity", "1"));

	float auxx=atof(rect_x.c_str());
	float auxy=atof(rect_y.c_str());
	coor2vect(&auxx,&auxy);
	build_vector (child_rect->add_child("param"),"point1",auxx,auxy);
	auxx= atof(rect_x.c_str()) + atof(rect_width.c_str());
	auxy= atof(rect_y.c_str()) + atof(rect_height.c_str());
	coor2vect(&auxx,&auxy);
	build_vector (child_rect->add_child("param"),"point2",auxx,auxy);


}

void
Svg_parser::parser_circle(const xmlpp::Element* nodeElement, xmlpp::Element* root, const Style& style)
{
	Glib::ustring circle_id = nodeElement->get_attribute_value("id");
	float circle_x      = style.compute("cx", "0", style.compute("width", "0"));
	float circle_y      = style.compute("cy", "0", style.compute("height", "0"));
	float circle_radius = atof(style.get("r", "0").c_str()); // FIXME compute for %

	Glib::ustring fill  = style.get("fill", "#000");
	float fill_opacity  = style.compute("fill_opacity", "1");
	float opacity       = style.compute("opacity", "1");

	xmlpp::Element *child_circle=root->add_child("layer");
	child_circle->set_attribute("type","circle");
	child_circle->set_attribute("active","true");
	child_circle->set_attribute("version","0.2");
	child_circle->set_attribute("desc",circle_id);

	build_real(child_circle->add_child("param"),"z_depth",0.0);
	build_real(child_circle->add_child("param"),"amount",1.0);
	build_integer(child_circle->add_child("param"),"blend_method",0);
	build_color(child_circle->add_child("param"),getRed (fill),getGreen (fill),getBlue(fill),opacity*fill_opacity);

	float cx = circle_x;
	float cy = circle_y;
	coor2vect(&cx,&cy);
	build_vector (child_circle->add_child("param"),"origin",cx,cy);
	float r = circle_radius;
	r /= kux;
	build_real(child_circle->add_child("param"),"radius",r);
}

/* === CONVERT TO PATH PARSERS ============================================= */

std::list<BLine>
Svg_parser::parser_path_polygon(const Glib::ustring& polygon_points, const SVGMatrix& mtx)
{
	std::list<BLine> k0;
	if(polygon_points.empty())
		return k0;
	std::list<Vertex> points;
	std::vector<String> tokens=get_tokens_path (polygon_points);

	for(unsigned int i=0;i<tokens.size();i++){
		float ax=atof(tokens.at(i).data());
		i++;
		float ay=atof(tokens.at(i).data());
		//mtx
		mtx.transformPoint2D(ax,ay);
		//adjust
		coor2vect(&ax,&ay);
		//save
		points.push_back(Vertex(ax,ay));
	}
	k0.push_front(BLine(points, true));
	return k0;
}

std::list<BLine>
Svg_parser::parser_path_d(const String& path_d, const SVGMatrix& mtx)
{
	std::list<BLine> k;
	std::list<Vertex> k1;

	std::vector<String> tokens=get_tokens_path(path_d);
	String command="M"; //the current command
	int lower_command='m';
	float ax,ay,tgx,tgy,tgx2,tgy2;//each method
	ax=ay=0;
	float current_x=0,current_y=0; //in svg coordinate space
	float old_x=0,old_y=0; //needed in rare cases
	float init_x=0,init_y=0; //for closepath commands

	bool is_old_cubic_tg_valid = false;
	bool is_old_quadratic_tg_valid = false;
	float old_tgx=0, old_tgy=0; // for shorthand cubic or quadratic commands

	const std::string possible_commands {"MmLlHhVvCcSsQqTtAaZz"};
	auto report_incomplete = [](const std::string& command) {
		error("SVG Parser: incomplete <d> element path command: %c!", command[0]);
	};

	for(unsigned int i=0;i<tokens.size();i++){
		//if the token is a command, change the current command
		if(possible_commands.find(tokens[i]) != std::string::npos) {
			command = tokens[i];
			i++;
		}

		lower_command = std::tolower(command[0]);
		if (lower_command != 'z') {
			if (i >= tokens.size()) { report_incomplete(command); break; }
		}

		old_x=current_x;
		old_y=current_y;
		//if command is absolute, set actual_x/y to zero
		if(std::isupper(command[0])) {
			current_x=0;
			current_y=0;
		}

		if (lower_command != 'c' && lower_command != 's')
			is_old_cubic_tg_valid = false;
		if (lower_command != 'q' && lower_command != 't')
			is_old_quadratic_tg_valid = false;

		//now parse the commands
		switch (lower_command){
		case 'm':{ //move to
			if(!k1.empty()) {
				k.push_front(BLine(k1, false));
				k1.clear();
			}
			//read
			current_x+=atof(tokens.at(i).data());
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			current_y+=atof(tokens.at(i).data());

			init_x=current_x;
			init_y=current_y;
			ax=current_x;
			ay=current_y;
			//operate and save
			mtx.transformPoint2D(ax,ay);
			coor2vect(&ax,&ay);
			k1.push_back(Vertex(ax,ay)); //first element
			k1.back().setSplit(true);
			//"If a moveto is followed by multiple pairs of coordinates,
			// the subsequent pairs are treated as implicit lineto commands."
			if (command == "M")
				command="L";
			else
				command="l";
			break;
		}
		case 'c':
		case 's':{ //curveto
			if (lower_command == 'c') {
				//tg2
				tgx2=current_x+atof(tokens.at(i).data());
				i++; if (i >= tokens.size()) { report_incomplete(command); break; }
				tgy2=current_y+atof(tokens.at(i).data());
			} else { // 's'
				if (is_old_cubic_tg_valid) {
					tgx2 = 2*old_x - old_tgx;
					tgy2 = 2*old_y - old_tgy;
				} else {
					tgx2 = old_x;
					tgy2 = old_y;
				}
			}
			//tg1
			if (lower_command == 'c') {
				i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			}
			tgx=current_x+atof(tokens.at(i).data());
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			tgy=current_y+atof(tokens.at(i).data());
			//point
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			current_x+=atof(tokens.at(i).data());
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			current_y+=atof(tokens.at(i).data());

			old_tgx = tgx;
			old_tgy = tgy;
			is_old_cubic_tg_valid = true;

			ax=current_x;
			ay=current_y;
			//mtx
			if(!mtx.is_identity()){
				mtx.transformPoint2D(tgx2,tgy2);
				mtx.transformPoint2D(ax,ay);
				mtx.transformPoint2D(tgx,tgy);
			}
			//adjust
			coor2vect(&tgx2,&tgy2);
			coor2vect(&ax,&ay);
			coor2vect(&tgx,&tgy);
			//save
			k1.back().setTg2(tgx2,tgy2);
			if(k1.front().isEqualTo(ax,ay)){
				k1.front().setTg1(tgx,tgy);
			}else{
				k1.push_back(Vertex(ax,ay));
				k1.back().setTg1(tgx,tgy);
				k1.back().setSplit(true);
			}
			break;
		}
		case 'q':
		case 't':{ //quadractic curve
				//tg1 and tg2 : they must be decreased 2/3 to correct representation
			if (lower_command == 'q') {
				tgx=current_x+atof(tokens.at(i).data());
				i++; if (i >= tokens.size()) { report_incomplete(command); break; }
				tgy=current_y+atof(tokens.at(i).data());
			} else { // 't'
				if (is_old_quadratic_tg_valid) {
					tgx = 2*old_x - old_tgx;
					tgy = 2*old_y - old_tgy;
				} else {
					tgx = old_x;
					tgy = old_y;
				}
			}
			//point
			if (lower_command == 'q') {
				i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			}
			current_x+=atof(tokens.at(i).data());
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			current_y+=atof(tokens.at(i).data());

			old_tgx = tgx;
			old_tgy = tgy;
			is_old_quadratic_tg_valid = true;

			ax=current_x;
			ay=current_y;
			//mtx
			if (!mtx.is_identity()) {
				mtx.transformPoint2D(ax,ay);
				mtx.transformPoint2D(tgx,tgy);
			}
			//adjust
			coor2vect(&ax,&ay);
			coor2vect(&tgx,&tgy);
			//save
			k1.back().setTg2(tgx,tgy);
			k1.back().radius2 *= 2/3.;
			k1.back().setSplit(true);

			k1.push_back(Vertex(ax,ay));
			k1.back().setTg1(tgx,tgy);
			k1.back().radius1 *= 2/3.;

			break;
		}
		case 'l':
		case 'h':
		case 'v':{ //line to
			//point
			if (command == "L" || command == "l") {
				current_x+=atof(tokens.at(i).data());
				i++; if (i >= tokens.size()) { report_incomplete(command); break; }
				current_y+=atof(tokens.at(i).data());
			} else if (command == "H" || command == "h") { // horizontal move
				current_x+=atof(tokens.at(i).data());
				current_y=old_y;
			} else if (command == "V" || command == "v") { //vertical
				current_x=old_x;
				current_y+=atof(tokens.at(i).data());
			}

			ax=current_x;
			ay=current_y;
			//mtx
			mtx.transformPoint2D(ax,ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			k1.back().setTg2(k1.back().x,k1.back().y);
			if(k1.front().isEqualTo(ax,ay)){
				k1.front().setTg1(k1.front().x,k1.front().y);
			}else{
				k1.push_back(Vertex(ax,ay));
				k1.back().setTg1(k1.back().x,k1.back().y);
			}
			break;
		}
		case 'a':{//elliptic arc

			//isn't complete support, is only for circles

			//this curve have 6 parameters
			//radius
			float radius_x,radius_y;
			//angle
			Angle angle;
			// flags (larger or smaller arc) (clockwise sweep or not)
			bool large,sweep;
			//radius
			radius_x=atof(tokens.at(i).data());
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			radius_y=atof(tokens.at(i).data());
			//angle
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			angle=Angle::deg(atof(tokens.at(i).data()));
			//flags
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			large=atoi(tokens.at(i).data()) ? 1 : 0;
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			sweep=atoi(tokens.at(i).data()) ? 1 : 0;
			//point
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			current_x+=atof(tokens.at(i).data());
			i++; if (i >= tokens.size()) { report_incomplete(command); break; }
			current_y+=atof(tokens.at(i).data());

			// According to section F.6.2 of SVG 1.1 specs and section 9.5.1 of SVG 2 specs
			//    ("Out-of-range elliptical arc parameters")
			{
				if (approximate_equal(current_x, old_x) && approximate_equal(current_y, old_y)) {
					// If endpoint == current point, ignore arc segment entirely
					break;
				}
				if (approximate_zero(radius_x) || approximate_zero(radius_y)) {
					// If either rx or ry is 0, then this arc is treated as a straight line segment (a "lineto") joining the endpoints.
					// (copied from line_to above)

					ax=current_x;
					ay=current_y;
					//mtx
					mtx.transformPoint2D(ax,ay);
					//adjust
					coor2vect(&ax,&ay);
					//save
					k1.back().setTg2(k1.back().x,k1.back().y);
					if(k1.front().isEqualTo(ax,ay)){
						k1.front().setTg1(k1.front().x,k1.front().y);
					}else{
						k1.push_back(Vertex(ax,ay));
						k1.back().setTg1(k1.back().x,k1.back().y);
					}
					break;
				}
				// If either rx or ry have negative signs, these are dropped; the absolute value is used instead.
				radius_x = std::fabs(radius_x);
				radius_y = std::fabs(radius_y);
			}

			// convert from endpoint to center parameterization
			// Based on description in SVG 1.1 spec (F.6.5) and SVG 2 spec (B.2.4)
			const Point p1_ = Matrix2().set_rotate(-angle) * Point((old_x - current_x)/2., (old_y - current_y)/2.);

			// Ensure radius is large enough
			// Step 3 of Section F.6.6 (SVG 1.1) or B.2.5 (SVG 2)
			bool radius_enlarged = false;
			const Real el = p1_[0]*p1_[0]/(radius_x*radius_x) + p1_[1]*p1_[1]/(radius_y*radius_y);
			{
				if (el > 1.) {
					radius_enlarged = true;
					Real sqrt_el = sqrt(el);
					radius_x *= sqrt_el;
					radius_y *= sqrt_el;
				}
			}

			Point c_;
			if (!radius_enlarged)  {
				c_ = Point(radius_x/radius_y*p1_[1], -radius_y/radius_x*p1_[0]) * sqrt(1./el  - 1.);
				if (large == sweep)
					c_ = -c_;
			}
			Point c = Matrix2().set_rotate(angle) * c_ + Point((old_x + current_x)/2., (old_y + current_y)/2.);

			const Point v0(1,0);
			const Point v1((p1_[0]-c_[0])/(radius_x), (p1_[1]-c_[1])/(radius_y));
			const Point v2((-p1_[0]-c_[0])/(radius_x), (-p1_[1]-c_[1])/(radius_y));

			auto calc_angle = [] (const Point& p, const Point& q) -> Angle {
				Real num = p[0]*q[0] + p[1]*q[1];
				Real den = p.mag() * q.mag();
				bool negative = p[0]*q[1] < p[1]*q[0];
				Real a = num / den;
				// avoid precision error
				a = synfig::clamp(a, -1., 1.);
				a = acos(a);
				if ((a < 0 && !negative) || (a > 0 && negative))
					a = -a;
				return Angle::rad(a);
			};

			Angle theta1 = calc_angle(v0, v1);

			Angle delta_theta = calc_angle(v1, v2);
			if (!sweep && delta_theta > Angle::zero()) // clockwise
				delta_theta -= Angle::one();
			else if (sweep && delta_theta < Angle::zero()) // anti-clockwise
				delta_theta += Angle::one();

			Angle theta2 = theta1 + delta_theta;

			// From here on, it is based on the paper "Drawing an elliptical arc using polylines, quadraticor cubic Bezier curves"
			// by L. Maisonobe, initially available on https://www.spaceroots.org/documents/ellipse/elliptical-arc.pdf
			// but now can be retrieved from https://web.archive.org/web/20210414175418/https://www.spaceroots.org/documents/ellipse/elliptical-arc.pdf
			// Thanks to him, I was able to derive the approximate cubic Bezier curve control points for any ellipsis arc.

			auto calc_eta = [] (const Angle& theta, float radius_x, float radius_y) -> Angle {
				return Angle::rad(atan2(Angle::sin(theta).get()/radius_y, Angle::cos(theta).get()/radius_x));
			};
			auto approx_greater = [] (const Angle& a, const Angle& b) {
				return approximate_greater_custom(Angle::deg(a).get(), Angle::deg(b).get(), 4.f);
			};
			auto approx_less = [] (const Angle& a, const Angle& b) {
				return approximate_less_custom(Angle::deg(a).get(), Angle::deg(b).get(), 4.f);
			};

			// Calculate the theoretical angles eta (for point and tangent computation)
			// Split great arcs at vertices multiples of 90 degrees
			std::vector<Angle> etas;
			etas.push_back(calc_eta(theta1, radius_x, radius_y));
			if (theta2 > theta1) {
				for (Angle a = -Angle::one()*3./4.; approx_less(a, theta2); a += Angle::half()/2)
					if (approx_greater(a, theta1))
						etas.push_back(calc_eta(a, radius_x, radius_y));
			} else {
				for (Angle a = Angle::one()*3./4.; approx_greater(a, theta2); a -= Angle::half()/2)
					if (approx_less(a, theta1))
						etas.push_back(calc_eta(a, radius_x, radius_y));
			}
			etas.push_back(calc_eta(theta2, radius_x, radius_y));

			std::vector<Point> points;
			points.push_back(Point(old_x, old_y));
			std::vector<std::pair<float,float>> tangents;
			for (size_t i = 1; i < etas.size(); i++) {
				const Angle &eta1 = etas[i-1];
				const Angle &eta2 = etas[i];

				Real x = c[0] +radius_x*Angle::cos(angle).get()*Angle::cos(eta2).get()-radius_y*Angle::sin(angle).get()*Angle::sin(eta2).get();
				Real y = c[1] +radius_x*Angle::sin(angle).get()*Angle::cos(eta2).get()+radius_y*Angle::cos(angle).get()*Angle::sin(eta2).get();
				points.push_back(Point(x, y));

				const Real alpha = Angle::sin(eta2 - eta1).get() * (sqrt(4+3*Angle::tan((eta2-eta1)/2.).get()*Angle::tan((eta2-eta1)/2.).get()) - 1) / 3.;

				if (i == 1) {
					Real tgx = old_x +alpha*(-radius_x*Angle::cos(angle).get()*Angle::sin(eta1).get()-radius_y*Angle::sin(angle).get()*Angle::cos(eta1).get());
					Real tgy = old_y +alpha*(-radius_x*Angle::sin(angle).get()*Angle::sin(eta1).get()+radius_y*Angle::cos(angle).get()*Angle::cos(eta1).get());
					tangents.push_back(std::pair<float,float>(tgx, tgy));
				}

				Real tgx = x -alpha*(-radius_x*Angle::cos(angle).get()*Angle::sin(eta2).get()-radius_y*Angle::sin(angle).get()*Angle::cos(eta2).get());
				Real tgy = y -alpha*(-radius_x*Angle::sin(angle).get()*Angle::sin(eta2).get()+radius_y*Angle::cos(angle).get()*Angle::cos(eta2).get());
				tangents.push_back(std::pair<float,float>(tgx, tgy));
			}

			// converting everything to Synfig coordinates
			for (auto &p : points) {
				float x = p[0];
				float y = p[1];
				//mtx
				mtx.transformPoint2D(x, y);
				//adjust
				coor2vect(&x,&y);
				p[0] = x;
				p[1] = y;
			}
			for (auto &p : tangents) {
				float x = p.first;
				float y = p.second;
				//mtx
				mtx.transformPoint2D(x, y);
				//adjust
				coor2vect(&x,&y);
				p.first = x;
				p.second = y;
			}

			// Explicit for end point, for avoiding precision error
			ax = current_x;
			ay = current_y;
			//mtx
			mtx.transformPoint2D(ax, ay);
			//adjust
			coor2vect(&ax,&ay);

			// Create Bline. First and last point are specially handled.

			// First arc point
			k1.back().setTg2(tangents.front().first, tangents.front().second);
			k1.back().setSplit(true);

			// Intermediate arc points
			for (size_t i = 1; i < tangents.size()-1; i++) {
				k1.push_back(Vertex(points[i][0], points[i][1]));
				k1.back().setTg1(tangents[i].first, tangents[i].second);
			}

			// Last arc point
			if(k1.front().isEqualTo(ax,ay)){
				k1.front().setTg1(tangents.back().first, tangents.back().second);
			}else{
				k1.push_back(Vertex(ax, ay));
				k1.back().setTg1(tangents.back().first, tangents.back().second);
				k1.back().setSplit(true);
			}
			break;
		}
		case 'z':{
			k.push_front(BLine(k1, true));
			k1.clear();
			current_x=init_x;
			current_y=init_y;
			if (i<tokens.size() && tokens[i] != "M" && tokens[i] != "m") {
				//starting a new path, but not with a moveto, so it uses the same initial point
				ax=current_x;
				ay=current_y;
				//operate and save
				mtx.transformPoint2D(ax,ay);
				coor2vect(&ax,&ay);
				k1.push_back(Vertex(ax,ay)); //first element
				k1.back().setSplit(true);
			}
			i--; //decrement i to balance "i++" at command change
			break;
		}
		default:
			synfig::warning("SVG Parser: unsupported path token: %s", tokens.at(i).c_str());
		}
	}
	if(!k1.empty()) {
		k.push_front(BLine(k1, false)); //last element
	}
	return k;
}

std::list<BLine>
Svg_parser::parser_path_rect(const xmlpp::Element* nodeElement, const Style& style, const SVGMatrix& mtx)
{
	std::list<BLine> k;
	if (!nodeElement)
		return k;
	try {
		double rect_x      = style.compute("x", "0");
		double rect_y      = style.compute("y", "0");
		double rect_width  = style.compute("width", "0");
		double rect_height = style.compute("height", "0");

		if (approximate_zero(rect_width) || approximate_zero(rect_height))
			return k;

		if (rect_width < 0 || rect_height < 0) {
			synfig::error("SVG Parser: Invalid width or height value for <rect>: it cannot be negative!");
			return k;
		}

		double rect_rx = 0.;
		double rect_ry = 0.;
		parser_rxry_property(style, rect_width, rect_height, rect_rx, rect_ry);
		{
			if (rect_rx > rect_width/2)
				rect_rx = rect_width/2;
			if (rect_ry > rect_height/2)
				rect_ry = rect_height/2;
		}

		std::string path;
		if (rect_rx > 0 && rect_ry > 0)
			path = etl::strprintf("M %lf %lf H %lf A %lf %lf 0 0,1 %lf %lf V %lf A %lf %lf 0 0,1 %lf %lf H %lf "
												  "A %lf %lf 0 0,1 %lf %lf V %lf A %lf %lf 0 0,1 %lf %lf z",
								  rect_x + rect_rx, rect_y, rect_x + rect_width - rect_rx,
								  rect_rx, rect_ry, rect_x + rect_width, rect_y + rect_ry,
								  rect_y + rect_height - rect_ry,
								  rect_rx, rect_ry, rect_x + rect_width - rect_rx, rect_y + rect_height,
								  rect_x + rect_rx,
								  rect_rx, rect_ry, rect_x, rect_y + rect_height - rect_ry,
								  rect_y + rect_ry,
								  rect_rx, rect_ry, rect_x + rect_rx, rect_y
								  );
		else
			path = etl::strprintf("M %lf %lf h %lf v %lf h %lf z",
								  rect_x, rect_y, rect_width, rect_height, -rect_width
								  );
		k = parser_path_d(path,mtx);
	} catch(...) {
		synfig::error("SVG Parser: Invalid coordinate value: it should be a real value!");
	}
	return k;
}

std::list<BLine>
Svg_parser::parser_path_circle(const xmlpp::Element* nodeElement, const Style& style, const SVGMatrix& mtx)
{
	std::list<BLine> k;
	if (!nodeElement)
		return k;
	float circle_x = style.compute("cx", "0", style.compute("width", "0"));
	float circle_y = style.compute("cy", "0", style.compute("height", "0"));
	float circle_radius = atof(style.get("r", "0").c_str()); // FIXME compute for %

	if (approximate_less(circle_radius, 0.f)) {
		synfig::error("SVG Parser: Invalid circle r value: it cannot be negative!");
		return k;
	}

	if (approximate_zero(circle_radius)) {
		return k;
	}

	std::string path = etl::strprintf("M %lf %lf A %lf %lf 0 0,1 %lf %lf A %lf %lf 0 0,1 %lf %lf "
												"A %lf %lf 0 0,1 %lf %lf A %lf %lf 0 0,1 %lf %lf z",
							  circle_x + circle_radius, circle_y,
							  circle_radius, circle_radius, circle_x, circle_y + circle_radius,
							  circle_radius, circle_radius, circle_x - circle_radius, circle_y,
							  circle_radius, circle_radius, circle_x, circle_y - circle_radius,
							  circle_radius, circle_radius, circle_x + circle_radius, circle_y
							  );
	k = parser_path_d(path,mtx);

	return k;
}

std::list<BLine>
Svg_parser::parser_path_ellipse(const xmlpp::Element *nodeElement, const Style &style, const SVGMatrix &mtx)
{
	std::list<BLine> k;
	if (!nodeElement)
		return k;
	float ellipse_x = style.compute("cx", "0", style.compute("width", "0"));
	float ellipse_y = style.compute("cy", "0", style.compute("height", "0"));;

	double ellipse_rx = 0, ellipse_ry = 0;
	if (!parser_rxry_property(style, style.compute("width", "0"), style.compute("height", "0"), ellipse_rx, ellipse_ry))
		return k;

	if (approximate_zero(ellipse_rx) || approximate_zero(ellipse_ry))
		return k;
	std::string path = etl::strprintf("M %lf %lf A %lf %lf 0 0,1 %lf %lf A %lf %lf 0 0,1 %lf %lf "
												"A %lf %lf 0 0,1 %lf %lf A %lf %lf 0 0,1 %lf %lf z",
							  ellipse_x + ellipse_rx, ellipse_y,
							  ellipse_rx, ellipse_ry, ellipse_x, ellipse_y + ellipse_ry,
							  ellipse_rx, ellipse_ry, ellipse_x - ellipse_rx, ellipse_y,
							  ellipse_rx, ellipse_ry, ellipse_x, ellipse_y - ellipse_ry,
							  ellipse_rx, ellipse_ry, ellipse_x + ellipse_rx, ellipse_y
							  );
	k = parser_path_d(path,mtx);

	return k;
}

std::list<BLine>
Svg_parser::parser_line(const xmlpp::Element *nodeElement, const SVGMatrix &mtx)
{
	std::list<BLine> k;
	if (!nodeElement)
		return k;

	double x1 = 0;
	double x2 = 0;
	double y1 = 0;
	double y2 = 0;

	try {
		x1 = std::stod(nodeElement->get_attribute_value("x1"));
		x2 = std::stod(nodeElement->get_attribute_value("x2"));
		y1 = std::stod(nodeElement->get_attribute_value("y1"));
		y2 = std::stod(nodeElement->get_attribute_value("y2"));
	} catch (...) {
		synfig::error("SVG Parser: Invalid <line> attribute: x1,y1,x2,y2 should be real values or percentages!");
		return k;
	}

	std::string path = etl::strprintf("M %lf %lf L %lf %lf", x1, y1, x2, y2);
	k = parser_path_d(path,mtx);

	return k;
}

std::list<BLine>
Svg_parser::parser_polyline(const xmlpp::Element *nodeElement, const SVGMatrix &mtx)
{
	std::list<BLine> k;
	if (!nodeElement)
		return k;

	std::string points_str = trim(nodeElement->get_attribute_value("points"));
	if (points_str.empty() || points_str == "none")
		return k;

	std::vector<String> points = tokenize(points_str, ", \x09\x0a\x0d");

	if (points.size() % 2 == 1) {
		error("SVG Parser: incomplete <polyline> element: points have an odd number of coordinate components %zu! Ignoring last number", points.size());
		points.pop_back();
	}

	std::string path = etl::strprintf("M %lf %lf", atof(points[0].c_str()), atof(points[1].c_str()));

	for (size_t i = 2; i < points.size(); i+=2)	 {
		path.append(etl::strprintf(" %lf %lf", atof(points[i].c_str()), atof(points[i+1].c_str())));
	}

	k = parser_path_d(path,mtx);

	return k;
}

/* === EFFECTS PARSERS ===================================================== */

void
Svg_parser::parser_effects(const xmlpp::Element* /*nodeElement*/, xmlpp::Element* root, const Style & /*parent_style*/, const SVGMatrix& mtx)
{
	build_transform(root, mtx);
}

/* === DEFS PARSERS ======================================================== */

void
Svg_parser::parser_defs(const xmlpp::Node* node)
{
	const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
	if(!nodeContent){
		xmlpp::Node::NodeList list = node->get_children();
		for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
			Glib::ustring name =(*iter)->get_name();
			if(name.compare("linearGradient")==0){
				parser_linearGradient(*iter);
			}else if(name.compare("radialGradient")==0){
				parser_radialGradient(*iter);
			}
 		}
  	}
}

/* === BUILDS ============================================================== */

void
Svg_parser::build_transform(xmlpp::Element* root, const SVGMatrix& mtx)
{
	if (!mtx.is_identity()) {
		xmlpp::Element *child_transform=root->add_child("layer");
		child_transform->set_attribute("type","warp");
		child_transform->set_attribute("active","true");
		child_transform->set_attribute("version","0.1");
		child_transform->set_attribute("desc","Transform");

		float x,y;
		x=100;y=100;coor2vect(&x,&y);
		build_vector (child_transform->add_child("param"),"src_tl",x,y);

		x=200;y=200;coor2vect(&x,&y);
		build_vector (child_transform->add_child("param"),"src_br",x,y);
		

		x=100;y=100;
		mtx.transformPoint2D(x,y);coor2vect(&x,&y);
		build_vector (child_transform->add_child("param"),"dest_tl",x,y);

		x=200;y=100;
		mtx.transformPoint2D(x,y);coor2vect(&x,&y);
		build_vector (child_transform->add_child("param"),"dest_tr",x,y);

		x=200;y=200;
		mtx.transformPoint2D(x,y);coor2vect(&x,&y);
		build_vector (child_transform->add_child("param"),"dest_br",x,y);

		x=100;y=200;
		mtx.transformPoint2D(x,y);coor2vect(&x,&y);
		build_vector (child_transform->add_child("param"),"dest_bl",x,y);

		build_param (child_transform->add_child("param"),"clip","bool","false");
	}
}

std::list<ColorStop>
Svg_parser::get_colorStop(String name)
{
	const std::list<ColorStop> none;

	if(!name.empty()){
		if(lg.empty()&& rg.empty())
			return none;

		String target_name = name;
		if(target_name.at(0)=='#') target_name.erase(0,1);
		else return none;
		for (const LinearGradient& linear_gradient : lg) {
			//only find into linear gradients
			if(linear_gradient.name == target_name)
				return linear_gradient.stops;
		}
	}
	return none;
}

void
Svg_parser::build_fill(xmlpp::Element* root, String name, const SVGMatrix& mtx)
{
	if(!name.empty()){
		int start=name.find_first_of("#")+1;
		int end=name.find_first_of(")");
		String target_name = name.substr(start,end-start);

		for (const LinearGradient& linear_gradient : lg) {
			if (linear_gradient.name == target_name) {
				build_linearGradient(root, linear_gradient, mtx);
				return;
			}
		}

		for (const RadialGradient& radial_gradient : rg) {
			if (radial_gradient.name == target_name) {
				build_radialGradient(root, radial_gradient, mtx);
				return;
			}
		}
	}
}

void
Svg_parser::build_stop_color(xmlpp::Element* root, const std::list<ColorStop>& stops)
{
	for (const auto& aux_stop : stops) {
		xmlpp::Element *child=root->add_child("color");
		child->set_attribute("pos",etl::strprintf("%f",aux_stop.pos));
		child->add_child("r")->set_child_text(etl::strprintf("%f",aux_stop.r));
		child->add_child("g")->set_child_text(etl::strprintf("%f",aux_stop.g));
		child->add_child("b")->set_child_text(etl::strprintf("%f",aux_stop.b));
		child->add_child("a")->set_child_text(etl::strprintf("%f",aux_stop.a));
	}
}

void
Svg_parser::build_linearGradient(xmlpp::Element* root, const LinearGradient& data, const SVGMatrix& mtx)
{
	xmlpp::Element* gradient=root->add_child("layer");

	gradient->set_attribute("type","linear_gradient");
	gradient->set_attribute("active","true");
	gradient->set_attribute("desc",data.name);
	build_param (gradient->add_child("param"),"z_depth","real","0");
	build_param (gradient->add_child("param"),"amount","real","1");
	//straight onto
	build_param (gradient->add_child("param"),"blend_method","integer","21");
	float x1,y1,x2,y2;
	x1=data.x1;
	y1=data.y1;
	x2=data.x2;
	y2=data.y2;


	{
		SVGMatrix mtx2;
		mtx2.compose(mtx,data.transform);

		//matrix transforms the gradient as a whole
		//it does not preserve angles, so we can't simply transform both points
		float x3, y3, k;
		//set point (x3,y3) on the same gradient line as (x2,y2)
		//the gradient line is perpendicular to (x1,y1)(x2,y2)
		x3=x2+(y2-y1);
		y3=y2-(x2-x1);
		//transform everything
		mtx2.transformPoint2D(x1,y1);
		mtx2.transformPoint2D(x2,y2);
		mtx2.transformPoint2D(x3,y3);

		if (x2!=x3 && y2!=y3) {//divide by zero check

			//set k as slope between (x2,y2) and (x3,y3)
			//k is the slope of gradient lines post-transformation
			k=(y3-y2)/(x3-x2);
			//set point (x2,y2) on the gradient line passing through (x3,y3)
			//so that the line (x1,y1)(x2,y2) is perpendicular to (x2,y2)(x3,y3)
			x2= (x3*k+x1/k+y1-y3)/(k+(1/k));
			y2= k*(x2-x3)+y3;
		} else if (x2==x3 && y2!=y3) {
			y2=y1;
		} else if (x2!=x3 && y2==y3) {
			x2=x1;
		} else {
			synfig::warning("SVG Parser: gradient points equal each other");
		}
	}

	coor2vect (&x1,&y1);
	coor2vect (&x2,&y2);

	build_vector (gradient->add_child("param"),"p1",x1,y1);
	build_vector (gradient->add_child("param"),"p2",x2,y2);
	//gradient link
	xmlpp::Element *child_stops=gradient->add_child("param");
	child_stops->set_attribute("name","gradient");
	child_stops->set_attribute("guid",GUID::hasher(data.name).get_string());
	build_stop_color (child_stops->add_child("gradient"),data.stops);
	build_param (gradient->add_child("param"),"loop","bool","false");
	build_param (gradient->add_child("param"),"zigzag","bool","false");
}

void
Svg_parser::build_radialGradient(xmlpp::Element* root, const RadialGradient& data, const SVGMatrix& mtx)
{
	xmlpp::Element* gradient;

	if (!mtx.is_identity() || !data.transform.is_identity()) {
		xmlpp::Element* layer=root->add_child("layer");

		layer->set_attribute("type","group");
		layer->set_attribute("active","true");
		layer->set_attribute("version","0.1");
		layer->set_attribute("desc",data.name);
		build_param (layer->add_child("param"),"z_depth","real","0");
		build_param (layer->add_child("param"),"amount","real","1");
		build_param (layer->add_child("param"),"blend_method","integer","21"); //straight onto
		build_vector (layer->add_child("param"),"origin",0,0);
		xmlpp::Element *child=layer->add_child("param");
		child->set_attribute("name","canvas");
		xmlpp::Element* child_layer=child->add_child("canvas");

		gradient=child_layer->add_child("layer");
		gradient->set_attribute("desc",data.name);
		build_param (gradient->add_child("param"),"blend_method","integer","0"); //composite
		SVGMatrix mtx2;
		mtx2.compose(mtx,data.transform);

		build_transform(child_layer,mtx2);

	} else {
		gradient=root->add_child("layer");
		gradient->set_attribute("desc",data.name);
		build_param (gradient->add_child("param"),"blend_method","integer","21"); //straight onto
	}

	gradient->set_attribute("type","radial_gradient");
	gradient->set_attribute("active","true");
	build_param (gradient->add_child("param"),"z_depth","real","0");
	build_param (gradient->add_child("param"),"amount","real","1");
	//gradient link
	xmlpp::Element *child_stops=gradient->add_child("param");
	child_stops->set_attribute("name","gradient");
	child_stops->set_attribute("guid",GUID::hasher(data.name).get_string());
	build_stop_color (child_stops->add_child("gradient"),data.stops);

	//here the center point and radius
	float cx=data.cx;
	float cy=data.cy;
	float r =data.r;

	//adjust
	coor2vect (&cx,&cy);
	r=r/kux;
	build_vector (gradient->add_child("param"),"center",cx,cy);
	build_param (gradient->add_child("param"),"radius","real",r);

	build_param (gradient->add_child("param"),"loop","bool","false");
	build_param (gradient->add_child("param"),"zigzag","bool","false");
}

void
Svg_parser::parser_linearGradient(const xmlpp::Node* node)
{
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring id	=nodeElement->get_attribute_value("id");
		float x1			=atof(nodeElement->get_attribute_value("x1").data());
		float y1			=atof(nodeElement->get_attribute_value("y1").data());
		float x2			=atof(nodeElement->get_attribute_value("x2").data());
		float y2			=atof(nodeElement->get_attribute_value("y2").data());
		Glib::ustring link	=nodeElement->get_attribute_value("href");
		Glib::ustring transform	=nodeElement->get_attribute_value("gradientTransform");

		if(link.empty())
			link = nodeElement->get_attribute_value("href","xlink");			

		//resolve transformations
		SVGMatrix mtx;
		if(!transform.empty())
			mtx.parser_transform(transform);

		std::list<ColorStop> stops;
		if(!link.empty()){
			stops = get_colorStop(link);
		}else{
			//color stops
			const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
			if(!nodeContent){
    			xmlpp::Node::NodeList list = node->get_children();
    			for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
					Glib::ustring name =(*iter)->get_name();
					if(name.compare("stop")==0){
						const xmlpp::Element* nodeIter = dynamic_cast<const xmlpp::Element*>(*iter);
						Style style;
						style.merge(nodeIter);
						float offset=atof(nodeIter->get_attribute_value("offset").data());
						String stop_color = style.get("stop-color", "#000000");
						String opacity = style.get("stop-opacity", "1");
						stops.push_back(ColorStop(stop_color, atof(opacity.data()), gamma, offset));
					}
    			}
			}
		}
		if (!stops.empty())
			lg.push_back(LinearGradient(id,x1,y1,x2,y2,stops,mtx));
	}
}

void
Svg_parser::parser_radialGradient(const xmlpp::Node* node)
{
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring id	=nodeElement->get_attribute_value("id");
		float cx			=atof(nodeElement->get_attribute_value("cx").data());
		float cy			=atof(nodeElement->get_attribute_value("cy").data());
		float fx			=atof(nodeElement->get_attribute_value("fx").data());
		float fy			=atof(nodeElement->get_attribute_value("fy").data());
		float r				=atof(nodeElement->get_attribute_value("r").data());
		Glib::ustring link	=nodeElement->get_attribute_value("href");//basic
		Glib::ustring transform	=nodeElement->get_attribute_value("gradientTransform");

		if(link.empty())
			link = nodeElement->get_attribute_value("href","xlink");

		if (cx!=fx || cy!=fy)
			synfig::warning("SVG Parser: ignoring focus attributes for radial gradient");

		//resolve transformations
		SVGMatrix mtx;
		if(!transform.empty())
			mtx.parser_transform(transform);

		if(!link.empty()){
			//inkscape always use link, i don't need parser stops here, but it's possible
			std::list<ColorStop> stops = get_colorStop(link);
			if (!stops.empty())
				rg.push_back(RadialGradient(id,cx,cy,r,stops,mtx));
		}
	}
}

ColorStop::ColorStop(const String& color, float opacity, const Gamma& gamma, float pos)
	: a(opacity), pos(pos)
{
	int red = getRed(color);
	int green = getGreen(color);
	int blue = getBlue(color);
	Color c = gamma.apply(Color(red/255.f, green/255.f, blue/255.f,a));
	this->r = c.get_r();
	this->g = c.get_g();
	this->b = c.get_b();
	this->a = c.get_a();
}

Color
Svg_parser::adjustGamma(float r, float g, float b, float a)
{
	return gamma.apply(Color(r,g,b,a));
}

LinearGradient::LinearGradient(const String& name, float x1, float y1, float x2, float y2, std::list<ColorStop> stops, SVGMatrix transform)
	: x1(x1), x2(x2),
	  y1(y1), y2(y2),
	  stops(stops), transform(transform)
{
	sprintf(this->name,"%s",name.data());
}

RadialGradient::RadialGradient(const String& name, float cx, float cy, float r, std::list<ColorStop> stops, SVGMatrix transform)
	: cx(cx), cy(cy), r(r),
	  stops(stops), transform(transform)
{
	sprintf(this->name,"%s",name.data());
}

BLine::BLine(std::list<Vertex> points, bool loop)
	: points(points), loop(loop),
	  bline_id(GUID().get_string()),
	  offset_id(GUID().get_string())
{
}

void
Svg_parser::build_gamma(xmlpp::Element* root, float gamma){
	root->set_attribute("type","colorcorrect");
	root->set_attribute("active","true");
	root->set_attribute("version","0.1");
	root->set_attribute("desc","Gamma");
	build_real (root->add_child("param"),"gamma",gamma);
}

void
Svg_parser::build_translate(xmlpp::Element* root, float dx, float dy)
{
	root->set_attribute("type","translate");
	root->set_attribute("active","true");
	root->set_attribute("version","0.1");
	build_vector (root->add_child("param"),"origin",dx,dy);
}

void
Svg_parser::build_rotate(xmlpp::Element* root, float dx, float dy, float angle)
{
	root->set_attribute("type","rotate");
	root->set_attribute("active","true");
	root->set_attribute("version","0.1");
	build_vector (root->add_child("param"),"origin",dx,dy);
	build_real   (root->add_child("param"),"amount",angle);
}

void
Svg_parser::build_points(xmlpp::Element* root, const std::list<Vertex>& p)
{
	root->set_attribute("name","vector_list");
	xmlpp::Element *child=root->add_child("dynamic_list");
	child->set_attribute("type","vector");
	for (const Vertex& vertex : p){
		xmlpp::Element *child_entry=child->add_child("entry");
		xmlpp::Element *child_vector=child_entry->add_child("vector");
		child_vector->add_child("x")->set_child_text(etl::strprintf("%f",vertex.x));
		child_vector->add_child("y")->set_child_text(etl::strprintf("%f",vertex.y));
	}
}

void
Svg_parser::build_vertex(xmlpp::Element* root, const Vertex &p)
{
	xmlpp::Element *child_comp=root->add_child("composite");
	child_comp->set_attribute("type","bline_point");
	build_vector (child_comp->add_child("param"),"point",p.x,p.y);
	build_param (child_comp->add_child("width"),"","real","1.0000000000");
	build_param (child_comp->add_child("origin"),"","real","0.5000000000");
	// ??????????
	build_param (child_comp->add_child("split"),"","bool", p.split_radius || p.split_angle ? "true" : "false");
	build_param (child_comp->add_child("split_radius"),"","bool", p.split_radius? "true" : "false");
	build_param (child_comp->add_child("split_angle"),"","bool", p.split_angle? "true" : "false");
	//tangent 1
	xmlpp::Element *child_t1=child_comp->add_child("t1");
	xmlpp::Element *child_rc=child_t1->add_child("radial_composite");
	child_rc->set_attribute("type","vector");
	build_param (child_rc->add_child("radius"),"","real",p.radius1);
	build_param (child_rc->add_child("theta"),"","angle",p.angle1);
	//tangent 2
	xmlpp::Element *child_t2=child_comp->add_child("t2");
	xmlpp::Element *child_rc2=child_t2->add_child("radial_composite");
	child_rc2->set_attribute("type","vector");
	build_param (child_rc2->add_child("radius"),"","real",p.radius2);
	build_param (child_rc2->add_child("theta"),"","angle",p.angle2);

}

void
Svg_parser::build_bline(xmlpp::Element* root, const std::list<Vertex>& p, bool loop, const String& blineguid)
{
	root->set_attribute("name","bline");
	xmlpp::Element *child=root->add_child("bline");
	child->set_attribute("type","bline_point");
	child->set_attribute("loop", loop? "true" : "false");
	if(!blineguid.empty())	child->set_attribute("guid",blineguid);
	for (const Vertex& vertex : p){
		build_vertex (child->add_child("entry"), vertex);
	}
}

void
Svg_parser::build_dilist(xmlpp::Element *root, const std::vector<float>& p, int linecap)
{
	root->set_attribute("name","dilist");
	xmlpp::Element *child=root->add_child("dilist");
	child->set_attribute("type","dash_item");
	child->set_attribute("loop", "false");
	for (size_t i = 0; i < p.size(); i++){
		xmlpp::Element *entry = child->add_child("entry")->add_child("composite");
		entry->set_attribute("type","dash_item");
		build_integer( entry->add_child("side_before"), "", linecap);
		build_integer( entry->add_child("side_after"), "", linecap);
		build_real( entry->add_child("length"), "", p[i]/kux);
		build_real( entry->add_child("offset"), "", p[++i]/kux);
	}
}

void
Svg_parser::build_param(xmlpp::Element* root, const String& name, const String& type, const String& value)
{
	if(!type.empty() && !value.empty()){
		if(!name.empty())	root->set_attribute("name",name);
		xmlpp::Element *child=root->add_child(type);
		child->set_attribute("value",value);
	}else{
		root->get_parent()->remove_child(root);
	}
}

void
Svg_parser::build_param(xmlpp::Element* root, const String& name, const String& type, float value)
{
	if(!type.empty()){
		if(!name.empty()) root->set_attribute("name",name);
		xmlpp::Element *child=root->add_child(type);
		child->set_attribute("value", etl::strprintf("%f",value));
	}else{
		root->get_parent()->remove_child(root);
	}
}

void
Svg_parser::build_param(xmlpp::Element* root, const String& name, const String& type, int value)
{
	if(!type.empty()){
			if(!name.empty()) root->set_attribute("name",name);
			xmlpp::Element *child=root->add_child(type);
			child->set_attribute("value", etl::strprintf("%d", value));
	}else{
		root->get_parent()->remove_child(root);
	}
}

void
Svg_parser::build_integer(xmlpp::Element* root, const String& name, int value)
{
	if(!name.empty()) root->set_attribute("name",name);
	xmlpp::Element *child=root->add_child("integer");
	child->set_attribute("value",etl::strprintf("%d", value));
}

void
Svg_parser::build_real(xmlpp::Element* root, const String& name, float value)
{
	if(!name.empty()) root->set_attribute("name",name);
	xmlpp::Element *child=root->add_child("real");
	child->set_attribute("value",etl::strprintf("%f", value));
}

void
Svg_parser::build_color(xmlpp::Element* root, float r, float g, float b, float a)
{
	if(r>255 || g>255 || b>255 || a>1 || r<0 || g<0 || b<0 || a<0){
		root->get_parent()->remove_child(root);
		synfig::warning("SVG Parser: color aborted - invalid data");
		return;
	}
	Color ret=adjustGamma(r/255,g/255,b/255,a);

	root->set_attribute("name","color");
	xmlpp::Element *child=root->add_child("color");
	child->add_child("r")->set_child_text(etl::strprintf("%f",ret.get_r()));
	child->add_child("g")->set_child_text(etl::strprintf("%f",ret.get_g()));
	child->add_child("b")->set_child_text(etl::strprintf("%f",ret.get_b()));
	child->add_child("a")->set_child_text(etl::strprintf("%f",ret.get_a()));
}

void
Svg_parser::build_vector(xmlpp::Element* root, const String& name, float x, float y)
{
	if(!name.empty()) root->set_attribute("name",name);
	xmlpp::Element *child=root->add_child("vector");
	child->add_child("x")->set_child_text(etl::strprintf("%f",x));
	child->add_child("y")->set_child_text(etl::strprintf("%f",y));
}

void
Svg_parser::build_vector (xmlpp::Element* root, const String& name, float x, float y, const String& guid)
{
	if(!name.empty()) root->set_attribute("name",name);
	xmlpp::Element *child=root->add_child("vector");
	if(!guid.empty()) child->set_attribute("guid",guid);
	child->add_child("x")->set_child_text(etl::strprintf("%f",x));
	child->add_child("y")->set_child_text(etl::strprintf("%f",y));
}

xmlpp::Element*
Svg_parser::initializeGroupLayerNode(xmlpp::Element* root, const String& name)
{
	root->set_attribute("type","group");
	root->set_attribute("active","true");
	root->set_attribute("version","0.1");
	root->set_attribute("desc",name);
	build_param (root->add_child("param"),"z_depth","real","0");
	build_param (root->add_child("param"),"amount","real","1");
	build_param (root->add_child("param"),"blend_method","integer","0");
	build_vector (root->add_child("param"),"origin",0,0);
	xmlpp::Element *child=root->add_child("param");
	child->set_attribute("name","canvas");
	return child->add_child("canvas");
}

/* === COORDINATES & TRANSFORMATIONS ======================================= */
void
Svg_parser::coor2vect(float *x,float *y){
	float sx, sy;
	sx=*x;
	sy=*y;
	sy= height-sy;
	sx= sx - ox;
	sy= sy - oy;
	sx= sx / kux;
	sy= sy / kux;
	*x=sx; *y=sy;
}

void
Vertex::setTg1(float p2x,float p2y)
{
	float rd=0,ag=0;
	float dx=(p2x-x)*3;
	float dy=(p2y-y)*3;
	rd=sqrt(dx*dx + dy*dy);
	if(approximate_zero(dx) && approximate_zero(dy))
		ag = 0;
	else
		ag = atan2(dy,dx);
	ag = ag*180/PI;
	ag += 180;

	this->radius1=rd;
	this->angle1=ag;
}

void
Vertex::setTg2(float p2x,float p2y)
{
	float rd=0,ag=0;
	float dx=(p2x-x)*3;
	float dy=(p2y-y)*3;

	rd=sqrt(dx*dx + dy*dy);
	if(approximate_zero(dx) && approximate_zero(dy))
		ag = 0;
	else
		ag = atan2(dy,dx);

	ag = ag*180/PI;

	this->radius2=rd;
	this->angle2=ag;
}

void
Vertex::setSplit(bool val)
{
	split_radius = val;
	split_angle = val;
}

void Vertex::setSplitRadius(bool val)
{
	split_radius = val;
}

void Vertex::setSplitAngle(bool val)
{
	split_angle = val;
}

bool
Vertex::isEqualTo(float a, float b) const
{
	return approximate_equal(x, a) && approximate_equal(y, b);
}

Vertex::Vertex(float x,float y)
	: x(x), y(y),
	  radius1(0), angle1(0),
	  radius2(0), angle2(0),
	  split_radius(false),
	  split_angle(false)
{
}

//matrices
void
SVGMatrix::parser_transform(String transform)
{
	bool first_iteration = true;
	SVGMatrix a;

	transform = trim(transform);

	if (transform.empty() || transform == "none") {
		*this = a;
		return;
	}

	std::vector<String> tokens=tokenize(transform,")");
	for (String token : tokens) {
		token = trim(token);
		if(token.compare(0,9,"translate")==0){
			int start = token.find_first_of("(")+1;
			std::vector<String> args = tokenize(token.substr(start), ", \x09\x0a\x0d");

			float dx = 0, dy = 0;
			if (args.size() > 0) {
				dx = atof(args[0].c_str());
				if (args.size() > 1)
					dy = atof(args[1].c_str());
			}

			const SVGMatrix translation_matrix = SVGMatrix(1,0,0,1,dx,dy);
			if (first_iteration)
				a = translation_matrix;
			else
				a.multiply(translation_matrix);
		}else if(token.compare(0,5,"scale")==0){
			int start = token.find_first_of("(")+1;
			std::vector<String> args = tokenize(token.substr(start), ", \x09\x0a\x0d");

			float sx = 1, sy = 1;
			if (args.size() > 0) {
				sx = atof(args[0].c_str());
				if (args.size() > 1)
					sy = atof(args[1].c_str());
				else
					sy = sx;
			}

			const SVGMatrix scale_matrix = SVGMatrix(sx,0,0,sy,0,0);
			if (first_iteration)
				a = scale_matrix;
			else
				a.multiply(scale_matrix);
		}else if(token.compare(0,6,"rotate")==0){
			int start = token.find_first_of("(")+1;
			std::vector<String> args = tokenize(token.substr(start), ", \x09\x0a\x0d");

			float angle = 0, cx = 0, cy = 0;
			if (args.size() > 0) {
				angle = getRadian(atof(args[0].c_str()));
				if (args.size() == 3) {
					cx = atof(args[1].c_str());
					cy = atof(args[2].c_str());
				}
			}

			const float seno   = sin(angle);
			const float coseno = cos(angle);

			const SVGMatrix rot_matrix = SVGMatrix(coseno,seno,-seno,coseno,0,0);
			if (approximate_zero(cx) && approximate_zero(cy)) {
				if (first_iteration)
					a = rot_matrix;
				else
					a.multiply(rot_matrix);
			} else {
				SVGMatrix aux(1,0,0,1,cx,cy);
				aux.multiply(rot_matrix);
				aux.multiply(SVGMatrix(1,0,0,1,-cx,-cy));
				if (first_iteration)
					a = aux;
				else
					a.multiply(aux);
			}
		}else if(token.compare(0,6,"matrix")==0){
			int start = token.find_first_of('(')+1;
			if (first_iteration)
				a = SVGMatrix(token.substr(start));
			else
				a.multiply(SVGMatrix(token.substr(start)));
		}else if(token.compare(0,4,"skew")==0){
			int start = token.find_first_of("(")+1;
			std::vector<String> args = tokenize(token.substr(start), ", \x09\x0a\x0d");

			float angle = 0;
			if (args.size() > 0) {
				angle = getRadian(atof(args[0].c_str()));
			}

			const float tgx = token[4] == 'X' ? tan(angle) : 0;
			const float tgy = token[4] == 'Y' ? tan(angle) : 0;

			const SVGMatrix skew_matrix = SVGMatrix(1,tgy,tgx,1,0,0);
			if (first_iteration)
				a = skew_matrix;
			else
				a.multiply(skew_matrix);
		}else{
			a = SVGMatrix(1,0,0,1,0,0);
		}
		first_iteration = false;
	}
	*this = a;
}

SVGMatrix::SVGMatrix()
	: a(1), c(0), e(0),
	  b(0), d(1), f(0)
{
}

SVGMatrix::SVGMatrix(float a, float b, float c, float d, float e, float f)
	: a(a), c(c), e(e),
	  b(b), d(d), f(f)
{
}

SVGMatrix::SVGMatrix(const String& mvector)
	: SVGMatrix()
{
	if(!mvector.empty()){
		std::vector<String> tokens=tokenize(mvector,",");
		if(tokens.size()!=6) return;

		a=atof(tokens.at(0).data());
		b=atof(tokens.at(1).data());
		c=atof(tokens.at(2).data());
		d=atof(tokens.at(3).data());
		e=atof(tokens.at(4).data());
		f=atof(tokens.at(5).data());
	}
}

const SVGMatrix SVGMatrix::identity(1,0,0,1,0,0);
const SVGMatrix SVGMatrix::zero(0,0,0,0,0,0);

bool
SVGMatrix::is_identity() const
{
	return a == 1.f && d == 1.f &&
			b == 0.f && e == 0.f &&
			c == 0.f && f == 0.f;
}

void
SVGMatrix::transformPoint2D(float &x,float &y) const
{
	float new_x= x*a + y*c + e;
	float new_y= x*b + y*d + f;

	x = new_x;
	y = new_y;
}

void
SVGMatrix::compose(const SVGMatrix& mtx1, const SVGMatrix& mtx2)
{
	SVGMatrix aux;
	aux.a = mtx1.a*mtx2.a + mtx1.c*mtx2.b;
	aux.b = mtx1.b*mtx2.a + mtx1.d*mtx2.b;
	aux.c = mtx1.a*mtx2.c + mtx1.c*mtx2.d;
	aux.d = mtx1.b*mtx2.c + mtx1.d*mtx2.d;
	aux.e = mtx1.a*mtx2.e + mtx1.c*mtx2.f + mtx1.e;
	aux.f = mtx1.b*mtx2.e + mtx1.d*mtx2.f + mtx1.f;
	*this = aux;
}

void
SVGMatrix::multiply(const SVGMatrix &mtx2)
{
	SVGMatrix aux;
	aux.a = a*mtx2.a + c*mtx2.b;
	aux.b = b*mtx2.a + d*mtx2.b;
	aux.c = a*mtx2.c + c*mtx2.d;
	aux.d = b*mtx2.c + d*mtx2.d;
	aux.e = a*mtx2.e + c*mtx2.f + e;
	aux.f = b*mtx2.e + d*mtx2.f + f;
	*this = aux;
}

/* === EXTRA METHODS ======================================================= */

static std::vector<String>
get_tokens_path(const String& path) //mini path lexico-parser
{
	std::vector<String> tokens;
	String buffer;
	int e=0;
	unsigned int i=0;
	char a;
	while(i<path.size()){
		a=path.at(i);
		switch(e){
			case 0: //initial state
					if(a=='m'){ e=1; i++;}
					else if(a=='c'){ e= 2; i++;}
					else if(a=='q'){ e= 3; i++;}
					else if(a=='t'){ e= 4; i++;}
					else if(a=='a'){ e= 5; i++;}
					else if(a=='l'){ e= 6; i++;}
					else if(a=='v'){ e= 7; i++;}
					else if(a=='h'){ e= 8; i++;}
					else if(a=='M'){ e= 9; i++;}
					else if(a=='C'){ e=10; i++;}
					else if(a=='Q'){ e=11; i++;}
					else if(a=='T'){ e=12; i++;}
					else if(a=='A'){ e=13; i++;}
					else if(a=='L'){ e=14; i++;}
					else if(a=='V'){ e=15; i++;}
					else if(a=='H'){ e=16; i++;}
					else if(a=='z' || a=='Z'){ e=17; i++;}
					else if(a=='-' || a=='.' || a=='e' || a=='E' || isdigit (a)){ e=18;}
					else if(a=='s'){ e=19; i++;}
					else if(a=='S'){ e=20; i++;}
					else if(a==',' || a==' ' || a==0x09 || a==0x0a || a==0x0d){ i++;}
					else {
						synfig::warning("SVG Parser: unknown token in SVG path '%c'", a);
						i++;
					}
					break;
			//relative
			case 1 : tokens.push_back("m"); e=0; break;//move
			case 2 : tokens.push_back("c"); e=0; break;//curve
			case 3 : tokens.push_back("q"); e=0; break;//quadratic
			case 4 : tokens.push_back("t"); e=0; break;//smooth quadratic
			case 5 : tokens.push_back("a"); e=0; break;//elliptic arc
			case 6 : tokens.push_back("l"); e=0; break;//line to
			case 7 : tokens.push_back("v"); e=0; break;//vertical
			case 8 : tokens.push_back("h"); e=0; break;//horizontal
			//absolute
			case 9 : tokens.push_back("M"); e=0; break;
			case 10: tokens.push_back("C"); e=0; break;
			case 11: tokens.push_back("Q"); e=0; break;
			case 12: tokens.push_back("T"); e=0; break;
			case 13: tokens.push_back("A"); e=0; break;
			case 14: tokens.push_back("L"); e=0; break;
			case 15: tokens.push_back("V"); e=0; break;
			case 16: tokens.push_back("H"); e=0; break;

			case 17: tokens.push_back("z"); e=0; break;//loop
			case 18: if(a=='-' || a=='.' || a=='e' || a=='E' || isdigit (a)){
						buffer.append(path.substr(i,1));i++;
					}else{
						e=21;
					}
					break;
			case 19: tokens.push_back("s"); e=0; break;
			case 20: tokens.push_back("S"); e=0; break;
			case 21: tokens.push_back(buffer);
					buffer.clear();
					e=0; break;
			default: break;
		}
	}
	switch(e){//last element
		case 1 : tokens.push_back("m"); break;
		case 2 : tokens.push_back("c"); break;
		case 3 : tokens.push_back("q"); break;
		case 4 : tokens.push_back("t"); break;
		case 5 : tokens.push_back("a"); break;
		case 6 : tokens.push_back("l"); break;
		case 7 : tokens.push_back("v"); break;
		case 8 : tokens.push_back("h"); break;
		case 9 : tokens.push_back("M"); break;
		case 10: tokens.push_back("C"); break;
		case 11: tokens.push_back("Q"); break;
		case 12: tokens.push_back("T"); break;
		case 13: tokens.push_back("A"); break;
		case 14: tokens.push_back("L"); break;
		case 15: tokens.push_back("V"); break;
		case 16: tokens.push_back("H"); break;
		case 17: tokens.push_back("z"); break;
		case 18: tokens.push_back(buffer); break;
		case 19: tokens.push_back("s"); break;
		case 20: tokens.push_back("S"); break;
		case 21: tokens.push_back(buffer); break;
		default: break;
	}
	return tokens;
}

static int
getRed(const String& hex)
{
	if(hex.at(0)=='#'){
		//allow for 3-digit hex codes (#rgb = #rrggbb)
		if (hex.length()<7) return (16+1) * hextodec(hex.substr(1,1));
		return hextodec(hex.substr(1,2));
	}else if(hex.compare(0,3,"rgb")==0 || hex.compare(0,3,"RGB")==0){
		int start=hex.find_first_of("(")+1;
		int end	=hex.find_last_of(")");
		String aux=tokenize(hex.substr(start,end-start),",").at(0);
		return atoi(aux.data());
	}
	return getColor(hex,1);
}
static int
getGreen(const String& hex)
{
	if(hex.at(0)=='#'){
		if (hex.length()<7) return (16+1) * hextodec(hex.substr(2,1));
		return hextodec(hex.substr(3,2));
	}else if(hex.compare(0,3,"rgb")==0 || hex.compare(0,3,"RGB")==0){
		int start=hex.find_first_of("(")+1;
		int end	=hex.find_last_of(")");
		String aux=tokenize(hex.substr(start,end-start),",").at(1);
		return atoi(aux.data());
	}
	return getColor(hex,2);
}
static int
getBlue(const String& hex)
{
	if(hex.at(0)=='#'){
		if (hex.length()<7) return (16+1) * hextodec(hex.substr(3,1));
		return hextodec(hex.substr(5,2));
	}else if(hex.compare(0,3,"rgb")==0 || hex.compare(0,3,"RGB")==0){
		int start=hex.find_first_of("(")+1;
		int end	=hex.find_last_of(")");
		String aux=tokenize(hex.substr(start,end-start),",").at(2);
		return atoi(aux.data());
	}
	return getColor(hex,3);
}
static int
hextodec(const std::string& hex)
{
	if (hex.empty()) {
		return 0;
	}
	unsigned long long result = 0;
	for (size_t i = 0, v = 0; i < hex.size(); ++i)
	{
		const char c = hex[i];
		if      ('a' <= c && c <='f') {	v = c - 'a' + 10; }
		else if ('A' <= c && c <='F') {	v = c - 'A' + 10; }
		else if ('0' <= c && c <='9') {	v = c - '0'; }
		else break;
		result*=16;
		result+=v;
	}
	return result;
}

/// \param use_90_ppi : old Inkscape files (<= 0.91) used 90ppi. SVG docs states for 96 pixels per inch
static double
getDimension(const String& ac, bool use_90_ppi)
{
	if(ac.empty())
		return 0;

	const int ppi = use_90_ppi ? 90 : 96;
	auto length=ac.size();
	double af=0;
	if (isdigit(ac[length-1]) || ac[length-1] == '.') {
		af = atof(ac.c_str());
	} else if(ac[length-1]=='%') {
		return 1024;
	} else {
		String unit = ac.substr(length-2,length);
		String nmc = ac.substr(0,length-2);
		af = atof(nmc.c_str());
		if (unit == "px"){
			af *= 1;
		} else if (unit == "pt"){
			af *= ppi/72.0;
		} else if (unit == "em"){
			af *= 16;
		} else if (unit == "mm"){
			af *= ppi/25.4;
		} else if (unit == "pc"){
			af *= ppi/6;
		} else if (unit == "cm"){
			af *= ppi/2.54;
		} else if (unit == "in"){
			af *= ppi;
		} else {
			return 1024;
		}
	}
	return af;
}

static float
getRadian(float sexa){
	return (sexa*2*PI)/360;
}

static std::vector<String>
tokenize(const String& str,const String& delimiters){
	std::vector<String> tokens;
	String::size_type lastPos = str.find_first_not_of(delimiters, 0);
	String::size_type pos = str.find_first_of(delimiters, lastPos);
	while (String::npos != pos || String::npos != lastPos){
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
	return tokens;
}

#define COLOR_NAME(color, r, g, b) {color, {r, g, b}},

static int
getColor(const String& name, int position) {
	if (position < 1 || position > 3) return 0;

	struct RGB {
		std::uint8_t color[3];
	};

	static std::unordered_map<std::string, RGB> color_names = {
		COLOR_NAME("aliceblue", 240, 248, 255)
		COLOR_NAME("antiquewhite", 250, 235, 215)
		COLOR_NAME("aqua", 0, 255, 255)
		COLOR_NAME("aquamarine", 127, 255, 212)
		COLOR_NAME("azure", 240, 255, 255)
		COLOR_NAME("beige", 245, 245, 220)
		COLOR_NAME("bisque", 255, 228, 196)
		COLOR_NAME("black", 0, 0, 0)
		COLOR_NAME("blanchedalmond", 255, 235, 205)
		COLOR_NAME("blue", 0, 0, 255)
		COLOR_NAME("blueviolet", 138, 43, 226)
		COLOR_NAME("brown", 165, 42, 42)
		COLOR_NAME("burlywood", 222, 184, 135)
		COLOR_NAME("cadetblue", 95, 158, 160)
		COLOR_NAME("chartreuse", 127, 255, 0)
		COLOR_NAME("chocolate", 210, 105, 30)
		COLOR_NAME("coral", 255, 127, 80)
		COLOR_NAME("cornflowerblue", 100, 149, 237)
		COLOR_NAME("cornsilk", 255, 248, 220)
		COLOR_NAME("crimson", 220, 20, 60)
		COLOR_NAME("cyan", 0, 255, 255)
		COLOR_NAME("darkblue", 0, 0, 139)
		COLOR_NAME("darkcyan", 0, 139, 139)
		COLOR_NAME("darkgoldenrod", 184, 134, 11)
		COLOR_NAME("darkgray", 169, 169, 169)
		COLOR_NAME("darkgreen", 0, 100, 0)
		COLOR_NAME("darkgrey", 169, 169, 169)
		COLOR_NAME("darkkhaki", 189, 183, 107)
		COLOR_NAME("darkmagenta", 139, 0, 139)
		COLOR_NAME("darkolivegreen", 85, 107, 47)
		COLOR_NAME("darkorange", 255, 140, 0)
		COLOR_NAME("darkorchid", 153, 50, 204)
		COLOR_NAME("darkred", 139, 0, 0)
		COLOR_NAME("darksalmon", 233, 150, 122)
		COLOR_NAME("darkseagreen", 143, 188, 143)
		COLOR_NAME("darkslateblue", 72, 61, 139)
		COLOR_NAME("darkslategray", 47, 79, 79)
		COLOR_NAME("darkslategrey", 47, 79, 79)
		COLOR_NAME("darkturquoise", 0, 206, 209)
		COLOR_NAME("darkviolet", 148, 0, 211)
		COLOR_NAME("deeppink", 255, 20, 147)
		COLOR_NAME("deepskyblue", 0, 191, 255)
		COLOR_NAME("dimgray", 105, 105, 105)
		COLOR_NAME("dimgrey", 105, 105, 105)
		COLOR_NAME("dodgerblue", 30, 144, 255)
		COLOR_NAME("firebrick", 178, 34, 34)
		COLOR_NAME("floralwhite", 255, 250, 240)
		COLOR_NAME("forestgreen", 34, 139, 34)
		COLOR_NAME("fuchsia", 255, 0, 255)
		COLOR_NAME("gainsboro", 220, 220, 220)
		COLOR_NAME("ghostwhite", 248, 248, 255)
		COLOR_NAME("gold", 255, 215, 0)
		COLOR_NAME("goldenrod", 218, 165, 32)
		COLOR_NAME("gray", 128, 128, 128)
		COLOR_NAME("grey", 128, 128, 128)
		COLOR_NAME("green", 0, 128, 0)
		COLOR_NAME("greenyellow", 173, 255, 47)
		COLOR_NAME("honeydew", 240, 255, 240)
		COLOR_NAME("hotpink", 255, 105, 180)
		COLOR_NAME("indianred", 205, 92, 92)
		COLOR_NAME("indigo", 75, 0, 130)
		COLOR_NAME("ivory", 255, 255, 240)
		COLOR_NAME("khaki", 240, 230, 140)
		COLOR_NAME("lavender", 230, 230, 250)
		COLOR_NAME("lavenderblush", 255, 240, 245)
		COLOR_NAME("lawngreen", 124, 252, 0)
		COLOR_NAME("lemonchiffon", 255, 250, 205)
		COLOR_NAME("lightblue", 173, 216, 230)
		COLOR_NAME("lightcoral", 240, 128, 128)
		COLOR_NAME("lightcyan", 224, 255, 255)
		COLOR_NAME("lightgoldenrodyellow", 250, 250, 210)
		COLOR_NAME("lightgray", 211, 211, 211)
		COLOR_NAME("lightgreen", 144, 238, 144)
		COLOR_NAME("lightgrey", 211, 211, 211)
		COLOR_NAME("lightpink", 255, 182, 193)
		COLOR_NAME("lightsalmon", 255, 160, 122)
		COLOR_NAME("lightseagreen", 32, 178, 170)
		COLOR_NAME("lightskyblue", 135, 206, 250)
		COLOR_NAME("lightslategray", 119, 136, 153)
		COLOR_NAME("lightslategrey", 119, 136, 153)
		COLOR_NAME("lightsteelblue", 176, 196, 222)
		COLOR_NAME("lightyellow", 255, 255, 224)
		COLOR_NAME("lime", 0, 255, 0)
		COLOR_NAME("limegreen", 50, 205, 50)
		COLOR_NAME("linen", 250, 240, 230)
		COLOR_NAME("magenta", 255, 0, 255)
		COLOR_NAME("maroon", 128, 0, 0)
		COLOR_NAME("mediumaquamarine", 102, 205, 170)
		COLOR_NAME("mediumblue", 0, 0, 205)
		COLOR_NAME("mediumorchid", 186, 85, 211)
		COLOR_NAME("mediumpurple", 147, 112, 219)
		COLOR_NAME("mediumseagreen", 60, 179, 113)
		COLOR_NAME("mediumslateblue", 123, 104, 238)
		COLOR_NAME("mediumspringgreen", 0, 250, 154)
		COLOR_NAME("mediumturquoise", 72, 209, 204)
		COLOR_NAME("mediumvioletred", 199, 21, 133)
		COLOR_NAME("midnightblue", 25, 25, 112)
		COLOR_NAME("mintcream", 245, 255, 250)
		COLOR_NAME("mistyrose", 255, 228, 225)
		COLOR_NAME("moccasin", 255, 228, 181)
		COLOR_NAME("navajowhite", 255, 222, 173)
		COLOR_NAME("navy", 0, 0, 128)
		COLOR_NAME("oldlace", 253, 245, 230)
		COLOR_NAME("olive", 128, 128, 0)
		COLOR_NAME("olivedrab", 107, 142, 35)
		COLOR_NAME("orange", 255, 165, 0)
		COLOR_NAME("orangered", 255, 69, 0)
		COLOR_NAME("orchid", 218, 112, 214)
		COLOR_NAME("palegoldenrod", 238, 232, 170)
		COLOR_NAME("palegreen", 152, 251, 152)
		COLOR_NAME("paleturquoise", 175, 238, 238)
		COLOR_NAME("palevioletred", 219, 112, 147)
		COLOR_NAME("papayawhip", 255, 239, 213)
		COLOR_NAME("peachpuff", 255, 218, 185)
		COLOR_NAME("peru", 205, 133, 63)
		COLOR_NAME("pink", 255, 192, 203)
		COLOR_NAME("plum", 221, 160, 221)
		COLOR_NAME("powderblue", 176, 224, 230)
		COLOR_NAME("purple", 128, 0, 128)
		COLOR_NAME("red", 255, 0, 0)
		COLOR_NAME("rosybrown", 188, 143, 143)
		COLOR_NAME("royalblue", 65, 105, 225)
		COLOR_NAME("saddlebrown", 139, 69, 19)
		COLOR_NAME("salmon", 250, 128, 114)
		COLOR_NAME("sandybrown", 244, 164, 96)
		COLOR_NAME("seagreen", 46, 139, 87)
		COLOR_NAME("seashell", 255, 245, 238)
		COLOR_NAME("sienna", 160, 82, 45)
		COLOR_NAME("silver", 192, 192, 192)
		COLOR_NAME("skyblue", 135, 206, 235)
		COLOR_NAME("slateblue", 106, 90, 205)
		COLOR_NAME("slategray", 112, 128, 144)
		COLOR_NAME("slategrey", 112, 128, 144)
		COLOR_NAME("snow", 255, 250, 250)
		COLOR_NAME("springgreen", 0, 255, 127)
		COLOR_NAME("steelblue", 70, 130, 180)
		COLOR_NAME("tan", 210, 180, 140)
		COLOR_NAME("teal", 0, 128, 128)
		COLOR_NAME("thistle", 216, 191, 216)
		COLOR_NAME("tomato", 255, 99, 71)
		COLOR_NAME("turquoise", 64, 224, 208)
		COLOR_NAME("violet", 238, 130, 238)
		COLOR_NAME("wheat", 245, 222, 179)
		COLOR_NAME("white", 255, 255, 255)
		COLOR_NAME("whitesmoke", 245, 245, 245)
		COLOR_NAME("yellow", 255, 255, 0)
		COLOR_NAME("yellowgreen", 154, 205, 50)
	};

	const auto found = color_names.find(name);
	if (found == color_names.end()) {
		return 0;
	}

	return found->second.color[position - 1];
}
#undef COLOR_NAME

static float
get_inkscape_version(const xmlpp::Element* svgNodeElement)
{
	try {
		std::string inkscape_full_version = svgNodeElement->get_attribute_value("version", "inkscape");
		std::vector<std::string> inkscape_version_tokens = tokenize(inkscape_full_version, " ");
		if (!inkscape_version_tokens.empty())
			return stod(inkscape_version_tokens[0]);
	} catch (...) {
	}
	return 0; // not inkscape ;)
}

void
Style::merge(const xmlpp::Element *elem)
{
	if (elem->get_name() == "style")
		return;

	// stores the style defined for this element
	// lower priority: style property values defined by presentation-kind attributes
	// medium priority: those values from CSS selectors (TODO)
	// highest priority: those values in "style" attribute contents

	Style style;

	style.merge_presentation_attributes(elem);

	// now CSS info - TODO

	// and then "style" attribute
	std::string style_str = elem->get_attribute_value("style");

	if (!style_str.empty()) {
		style.merge_style_string(style_str);
	}

	// merge properties with current style
	// Normally, we just replace them with new values
	// However, there are some special cases (SVG 1.1 6.14):
	//   - relative values will have the same units as the value to which it is relative
	//      and percentage lengths are relative to current viewport or bounding box dimensions
	//   - if a 'clip-path' property is specified on an ancestor element, and the current element
	//       has a 'clip-path' of none, the ancestor's clipping path still applies to current element

	for (auto prop : style.data) {
		if (prop.first == "clip-path" && prop.second == "none")
			continue;
		// TODO: Relative values?
		data[prop.first] = prop.second;
	}
}

std::string
Style::get(const std::string &property, std::string default_value) const
{
	auto item = data.find(property);
	if (item == data.end() || item->second.empty())
		return default_value;
	return item->second;
}

static bool
parse_number_or_percent(const std::string& value, double& out)
{
	std::size_t pos;
	try {
		ChangeLocale l(LC_NUMERIC, "C");
		out = std::stod(value, &pos);
		if (pos && value[pos] == '%') {
			out = out * 0.01;
		}
		return true;
	} catch (...) {
		return false;
	}
}

double
Style::compute(const std::string &property, std::string default_value, double reference_value) const
{
	std::string value = get(property, default_value);

	double d_value;
	if (parse_number_or_percent(value, d_value)) {
		if (!value.empty() && value.back() == '%')
			return d_value * reference_value;
		else
			return d_value;
	}

	warning("Layer_Svg: %s",
		etl::strprintf(_("Invalid number for '%s': %s. Trying default value..."), property.c_str(), value.c_str()).c_str());

	if (parse_number_or_percent(default_value, d_value)) {
		if (!value.empty() && value.back() == '%')
			return d_value * reference_value;
		else
			return d_value;
	}

	error("Layer_Svg: %s", etl::strprintf(_("... No, invalid number for '%s': %s"), property.c_str(), default_value.c_str()).c_str());
	return 0;
}

void
Style::merge_presentation_attributes(const xmlpp::Element *elem)
{
	// presentation attributes: https://www.w3.org/TR/SVG/styling.html#ElementSpecificStyling
	// this list won't increase in new versions. phew!

	static const std::vector<std::string> circle_ellipse_props {"cx","cy"};
	// geometry_props is for 'foreignObject', 'image', 'rect', 'svg', 'symbol', and 'use'
	static const std::vector<std::string> geometry_props {"height", "width", "x", "y"};
	//static const std::vector<std::string> circle_props {"r"};
	static const std::vector<std::string> ellipse_rect_props {"rx", "ry"};

	//static const std::vector<std::string> path_props {"d"};

	//static const std::vector<std::string> non_animation_elem_props {"fill"};

	//static const std::vector<std::string> almost_all_elem_props {"transform"};
	//static const std::vector<std::string> pattern_elem_props {"patternTransform"};
	//static const std::vector<std::string> linear_radial_elem_props {"gradientTransform"};

	static const std::vector<std::string> general_props {"alignment-baseline", "baseline-shift", "clip-path", "clip-rule",
		"color", "color-interpolation", "color-interpolation-filters", "color-rendering",
		"cursor", "direction", "display", "dominant-baseline",
		"fill-opacity", "fill-rule", "filter", "flood-color", "flood-opacity",
		"font-family", "font-size", "font-size-adjust", "font-stretch", "font-style", "font-variant", "font-weight",
		"glyph-orientation-horizontal", "glyph-orientation-vertical",
		"image-rendering", "letter-spacing", "lighting-color",
		"marker-end", "marker-mid", "marker-start", "mask", "opacity", "overflow", "paint-order", "pointer-events",
		"shape-rendering", "stop-color", "stop-opacity",
		"stroke", "stroke-dasharray", "stroke-dashoffset", "stroke-linecap", "stroke-linejoin", "stroke-miterlimit", "stroke-opacity", "stroke-width",
		"text-anchor", "text-decoration", "text-overflow", "text-rendering", "unicode-bidi",
		"vector-effect", "visibility", "white-space", "word-spacing", "writing-mode"};

	for (const auto& prop : general_props)
		push(prop, elem->get_attribute_value(prop));

	const std::string elem_name = elem->get_name();
	if (elem_name == "circle" || elem_name == "ellipse") {
		for (const auto& prop : circle_ellipse_props)
			push(prop, elem->get_attribute_value(prop));
	}
	if (elem_name == "foreignObject" || elem_name == "image" || elem_name == "rect" || elem_name == "svg" || elem_name == "symbol" || elem_name == "use") {
		for (const auto& prop : geometry_props)
			push(prop, elem->get_attribute_value(prop));
	}
	if (elem_name == "circle") {
		push("r", elem->get_attribute_value("r"));
	}
	if (elem_name == "ellipse" || elem_name == "rect") {
		for (const auto& prop : ellipse_rect_props)
			push(prop, elem->get_attribute_value(prop));
	}
	if (elem_name == "path") {
		push("d", elem->get_attribute_value("d"));
	}
	// 'fill' for all non-animation elements
	if (elem_name != "animate" && elem_name != "animateMotion" && elem_name != "animateTransform" && elem_name != "discard" && elem_name != "set") {
		push("fill", elem->get_attribute_value("fill"));
	}
	// transform
	if (elem_name == "pattern") {
		push("transform", elem->get_attribute_value("patternTransform"));
	} else if (elem_name == "linearGradient" || elem_name == "radialGradient") {
		push("transform", elem->get_attribute_value("gradientTransform"));
	} else {
		push("transform", elem->get_attribute_value("transform"));
	}
}

void
Style::merge_style_string(const std::string &style_str)
{
	size_t previous_pos = 0;
	size_t pos = 0;
	auto push_property = [&] (size_t& pos) {
		std::string token = style_str.substr(previous_pos, pos-previous_pos);
		size_t separator_pos = token.find(':');
		if (separator_pos != std::string::npos && separator_pos != token.size()-1) {
			std::string prop = synfig::trim(token.substr(0, separator_pos));
			std::string value = synfig::trim(token.substr(separator_pos+1));
			if (!prop.empty() && !value.empty())
				push(prop, value);
		}
		pos++;
		previous_pos = pos;
	};

	while ((pos = style_str.find(';', pos)) != std::string::npos) {
		push_property(pos);
	}

	// last item
	pos = style_str.length();
	push_property(pos);
}

void
Style::push(const std::string &property, const std::string &value)
{
	if (!value.empty() && value != "inherit" && !property.empty())
		data[property] = value;
}
