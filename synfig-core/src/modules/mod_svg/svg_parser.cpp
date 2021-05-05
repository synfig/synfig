/* === S Y N F I G ========================================================= */
/*!	\file svg_parser.cpp
**	\brief Implementation of the Svg parser
**	\brief Based on SVG XML specification 1.1
**	\brief See: http://www.w3.org/TR/xml11/ for deatils
**
**	$Id:$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>

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
static int extractSubAttribute(const String& attribute, const String& name, String& value);
static String loadAttribute(const String& name, const String& path_style, const String& master_style, String subattribute, String defaultVal);
static String loadAttribute(const String& name, const String& path_style, const String& master_style, String defaultVal);
static std::vector<String> get_tokens_path(const String& path);
static int getRed(const String& hex);
static int getGreen(const String& hex);
static int getBlue(const String& hex);
static int hextodec(const std::string& hex);
static int getColor(const String& name, int position);
static double getDimension(const String& ac);
static float getRadian(float sexa);
//string functions
static void removeIntoS(String& input);
static std::vector<String> tokenize(const String& str,const String& delimiters);

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
		std::cout<<"error"<<std::endl;
	}
	return canvas;
}

Canvas::Handle
Svg_parser::load_svg_canvas(std::string _filepath, String &errors, String &warnings)
{
	ChangeLocale locale(LC_NUMERIC, "C");

	filepath = _filepath;
	#ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  	try{
  	#endif //LIBXMLCPP_EXCEPTIONS_ENABLED
		//load parser
		parser.set_substitute_entities();
		parser.parse_file(filepath);
		//set_id(filepath);
		if(parser){
		  	const xmlpp::Node* pNode = parser.get_document()->get_root_node();
		  	parser_node(pNode);
		}
	#ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  	}catch(const std::exception& ex){
    	std::cout << "Exception caught: " << ex.what() << std::endl;
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
	uid(0),
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
			parser_graphics(node,nodeRoot,"",SVGMatrix::indentity);
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
		width = getDimension(nodeElement->get_attribute_value("width"));
		height = getDimension(nodeElement->get_attribute_value("height"));
		docname=nodeElement->get_attribute_value("docname","");
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
Svg_parser::parser_graphics(const xmlpp::Node* node, xmlpp::Element* root, String parent_style, const SVGMatrix& mtx_parent)
{
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring nodename = node->get_name();
		if (nodename.compare("g")==0 || nodename.compare("path")==0 || nodename.compare("polygon")==0 || nodename.compare("rect")==0){} else return;

		//load sub-attributes
		Glib::ustring id			=nodeElement->get_attribute_value("id");
		Glib::ustring transform	=nodeElement->get_attribute_value("transform");

		//resolve transformations
		SVGMatrix mtx;
		if(!transform.empty())
			mtx.parser_transform(transform);
		if (SVG_SEP_TRANSFORMS)
		{
			mtx.compose(mtx_parent, mtx);
		}
		if(nodename.compare("g")==0){
			parser_layer (node,root->add_child("layer"),parent_style,mtx);
			return;
		}

		Glib::ustring obj_style	=nodeElement->get_attribute_value("style");
		Glib::ustring obj_fill			=nodeElement->get_attribute_value("fill");

		//style
		String fill			    =loadAttribute("fill",obj_style,parent_style,obj_fill,"none");
		String fill_rule		=loadAttribute("fill-rule",obj_style,parent_style,"evenodd");
		String stroke			=loadAttribute("stroke",obj_style,parent_style,"none");
		String stroke_width		=loadAttribute("stroke-width",obj_style,parent_style,"1px");
		String stroke_linecap	=loadAttribute("stroke-linecap",obj_style,parent_style,"butt");
		String stroke_linejoin	=loadAttribute("stroke-linejoin",obj_style,parent_style,"miter");
		String stroke_opacity	=loadAttribute("stroke-opacity",obj_style,parent_style,"1");
		String fill_opacity		=loadAttribute("fill-opacity",obj_style,parent_style,"1");
		String opacity			=loadAttribute("opacity",obj_style,parent_style,"1");


		//Fill
		int typeFill=0; //nothing

		if(fill.compare("none")!=0){
			typeFill=1; //simple
		}
		if(typeFill==1 && fill.compare(0,3,"url")==0){
			typeFill=2;	//gradient
		}
		//Stroke
		int typeStroke=0;//nothing

		if(stroke.compare("none")!=0){
			typeStroke=1; //simple
		}
		if(typeStroke==1 && stroke.compare(0,3,"url")==0){
			typeStroke=2;	//gradient
		}
		
		xmlpp::Element* child_layer = root;
		xmlpp::Element* child_fill;
		xmlpp::Element* child_stroke;

		//make simple fills
		if(nodename.compare("rect")==0 && typeFill!=0){
			if (!mtx.is_identity())
				child_layer = nodeStartBasicLayer(root->add_child("layer"), id);
			child_fill=child_layer;
			parser_rect(nodeElement,child_fill,fill,fill_opacity,opacity);
			if(typeFill==2){
				build_fill (child_fill,fill,SVGMatrix::indentity);
			}
			parser_effects(nodeElement,child_layer,parent_style,mtx);
			return;
		}
		if ((!SVG_RESOLVE_BLINE) || typeFill==2 || typeStroke==2)
			child_layer = nodeStartBasicLayer(root->add_child("layer"), id);
		child_fill=child_layer;
		child_stroke=child_layer;

		//=======================================================================

		std::list<BLine> k;
		//if we are creating a bline

		//First, create the list of Verteces
		if (SVG_RESOLVE_BLINE) {
			if(nodename.compare("path")==0){
				k = parser_path_d(nodeElement->get_attribute_value("d"),mtx);
			} else if(nodename.compare("polygon")==0){
				k = parser_path_polygon(nodeElement->get_attribute_value("points"),mtx);
			}
		} else {
			if(nodename.compare("path")==0){
				k = parser_path_d(nodeElement->get_attribute_value("d"),SVGMatrix::indentity);
			} else if(nodename.compare("polygon")==0){
				k = parser_path_polygon(nodeElement->get_attribute_value("points"),SVGMatrix::indentity);
			}
		}
		
		std::list<BLine>::iterator aux;
		//int n = k.size();

		if(typeFill!=0){//region layer
			/*if(typeFill==2){
				child_fill=nodeStartBasicLayer(child_fill->add_child("layer"));
			}*/
			for (aux = k.begin(); aux!=k.end(); aux++){
				xmlpp::Element *child_region=child_fill->add_child("layer");
				child_region->set_attribute("type","region");
				child_region->set_attribute("active","true");
				child_region->set_attribute("version","0.1");
				child_region->set_attribute("desc",id);
				build_param (child_region->add_child("param"),"z_depth","real","0.0000000000");
				build_param (child_region->add_child("param"),"amount","real","1.0000000000");
				build_param (child_region->add_child("param"),"blend_method","integer","0");
				build_color (child_region->add_child("param"),getRed(fill),getGreen(fill),getBlue(fill),atof(fill_opacity.data())*atof(opacity.data()));
				build_vector (child_region->add_child("param"),"offset",0,0, aux->offset_id );
				build_param (child_region->add_child("param"),"invert","bool","false");
				build_param (child_region->add_child("param"),"antialias","bool","true");
				build_param (child_region->add_child("param"),"feather","real","0.0000000000");
				build_param (child_region->add_child("param"),"blurtype","integer","1");
				if(fill_rule.compare("evenodd")==0) build_param (child_region->add_child("param"),"winding_style","integer","1");
				else build_param (child_region->add_child("param"),"winding_style","integer","0");

				build_bline(child_region->add_child("param"), aux->points, aux->loop, aux->bline_id);
			}
		}
		if(typeFill==2){ //gradient in onto mode (fill)
			if (SVG_RESOLVE_BLINE)
				build_fill(child_fill,fill,mtx);
			else
				build_fill(child_fill,fill,SVGMatrix::indentity);
		}

		if(typeStroke!=0){//outline layer
			if(typeStroke==2){
				child_stroke=nodeStartBasicLayer(child_stroke->add_child("layer"),"stroke");
			}
			for (aux=k.begin(); aux!=k.end(); aux++){
				xmlpp::Element *child_outline=child_stroke->add_child("layer");
				child_outline->set_attribute("type","outline");
				child_outline->set_attribute("active","true");
				child_outline->set_attribute("version","0.2");
				child_outline->set_attribute("desc",id);
				build_param (child_outline->add_child("param"),"z_depth","real","0.0000000000");
				build_param (child_outline->add_child("param"),"amount","real","1.0000000000");
				build_param (child_outline->add_child("param"),"blend_method","integer","0");
				build_color (child_outline->add_child("param"),getRed(stroke),getGreen(stroke),getBlue(stroke),atof(stroke_opacity.data())*atof(opacity.data()));
				build_vector (child_outline->add_child("param"),"offset",0,0,aux->offset_id);
				build_param (child_outline->add_child("param"),"invert","bool","false");
				build_param (child_outline->add_child("param"),"antialias","bool","true");
				build_param (child_outline->add_child("param"),"feather","real","0.0000000000");
				build_param (child_outline->add_child("param"),"blurtype","integer","1");
				//outline in nonzero
				build_param (child_outline->add_child("param"),"winding_style","integer","0");

				build_bline(child_outline->add_child("param"), aux->points, aux->loop, aux->bline_id);

				stroke_width=etl::strprintf("%f",getDimension(stroke_width)/kux);
				build_param (child_outline->add_child("param"),"width","real",stroke_width);
				build_param (child_outline->add_child("param"),"expand","real","0.0000000000");
				if(stroke_linejoin.compare("miter")==0) build_param (child_outline->add_child("param"),"sharp_cusps","bool","true");
				else build_param (child_outline->add_child("param"),"sharp_cusps","bool","false");
				if(stroke_linecap.compare("butt")==0){
					build_param (child_outline->add_child("param"),"round_tip[0]","bool","false");
					build_param (child_outline->add_child("param"),"round_tip[1]","bool","false");
				}else{
					build_param (child_outline->add_child("param"),"round_tip[0]","bool","true");
					build_param (child_outline->add_child("param"),"round_tip[1]","bool","true");
				}
				build_param (child_outline->add_child("param"),"loopyness","real","1.0000000000");
				build_param (child_outline->add_child("param"),"homogeneous_width","bool","true");
			}

			if(typeStroke==2){ //gradient in onto mode (stroke)
				if (SVG_RESOLVE_BLINE)
					build_fill(child_stroke,stroke,mtx);
				else
					build_fill(child_stroke,stroke,SVGMatrix::indentity);
			}	
		}

		if (SVG_RESOLVE_BLINE)
			parser_effects(nodeElement,child_layer,parent_style,SVGMatrix::indentity);
		else
			parser_effects(nodeElement,child_layer,parent_style,mtx);
	}
}

/* === LAYER PARSERS ======================================================= */

void
Svg_parser::parser_layer(const xmlpp::Node* node, xmlpp::Element* root, String parent_style, const SVGMatrix& mtx)
{
	if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		Glib::ustring label		=nodeElement->get_attribute_value("label");
		Glib::ustring style		=nodeElement->get_attribute_value("style");
		Glib::ustring fill		=nodeElement->get_attribute_value("fill");

		String layer_style;
		if(!style.empty()){
			layer_style=style;
		}else if(!fill.empty()){
			layer_style.append("fill:");
			layer_style.append(fill);
		}else if(!parent_style.empty()){
			layer_style=parent_style;
		}
		//build
		root->set_attribute("type","group");
		root->set_attribute("active","true");
		root->set_attribute("version","0.1");
		if(!label.empty())	root->set_attribute("desc",label);
		else root->set_attribute("desc",_("Inline Canvas"));

		build_real(root->add_child("param"),"z_depth",0.0);
		build_real(root->add_child("param"),"amount",1.0);
		build_integer(root->add_child("param"),"blend_method",0);
		build_vector (root->add_child("param"),"origin",0,0);

		//printf(" canvas attributes ");
		//canvas
		xmlpp::Element *child_canvas=root->add_child("param");
		child_canvas->set_attribute("name","canvas");
		child_canvas=child_canvas->add_child("canvas");
		const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
		if(!nodeContent){
    		xmlpp::Node::NodeList list = node->get_children();
    		for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter){
				Glib::ustring name =(*iter)->get_name();
				parser_graphics (*iter,child_canvas,layer_style,mtx);
    		}
  		}
		if (SVG_SEP_TRANSFORMS) parser_effects(nodeElement,child_canvas,parent_style,SVGMatrix::indentity);
		else parser_effects(nodeElement,child_canvas,parent_style,mtx);
	}
}

void
Svg_parser::parser_rect(const xmlpp::Element* nodeElement,xmlpp::Element* root, const String& fill, const String& fill_opacity, const String& opacity)
{
	Glib::ustring rect_id		=nodeElement->get_attribute_value("id");
	Glib::ustring rect_x		=nodeElement->get_attribute_value("x");
	Glib::ustring rect_y		=nodeElement->get_attribute_value("y");
	Glib::ustring rect_width	=nodeElement->get_attribute_value("width");
	Glib::ustring rect_height	=nodeElement->get_attribute_value("height");

	xmlpp::Element *child_rect=root->add_child("layer");
	child_rect->set_attribute("type","rectangle");
	child_rect->set_attribute("active","true");
	child_rect->set_attribute("version","0.2");
	child_rect->set_attribute("desc",rect_id);

	build_real(child_rect->add_child("param"),"z_depth",0.0);
	build_real(child_rect->add_child("param"),"amount",1.0);
	build_integer(child_rect->add_child("param"),"blend_method",0);
	build_color (child_rect->add_child("param"),getRed (fill),getGreen (fill),getBlue(fill),atof(opacity.data())*atof(fill_opacity.data()));

	float auxx=atof(rect_x.c_str());
	float auxy=atof(rect_y.c_str());
	coor2vect(&auxx,&auxy);
	build_vector (child_rect->add_child("param"),"point1",auxx,auxy);
	auxx= atof(rect_x.c_str()) + atof(rect_width.c_str());
	auxy= atof(rect_y.c_str()) + atof(rect_height.c_str());
	coor2vect(&auxx,&auxy);
	build_vector (child_rect->add_child("param"),"point2",auxx,auxy);


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
		i++; if(tokens.at(i).compare(",")==0) i++;
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
	float ax,ay,tgx,tgy,tgx2,tgy2;//each method
	ax=ay=0;
	float actual_x=0,actual_y=0; //in svg coordinate space
	float old_x=0,old_y=0; //needed in rare cases
	float init_x=0,init_y=0; //for closepath commands

	for(unsigned int i=0;i<tokens.size();i++){
		//if the token is a command, change the current command
		if(tokens.at(i).compare("M")==0 || tokens.at(i).compare("m")==0 || tokens.at(i).compare("L")==0 || tokens.at(i).compare("l")==0 || tokens.at(i).compare("H")==0 || tokens.at(i).compare("h")==0 || tokens.at(i).compare("V")==0 || tokens.at(i).compare("v")==0 || tokens.at(i).compare("C")==0 || tokens.at(i).compare("c")==0 || tokens.at(i).compare("S")==0 || tokens.at(i).compare("s")==0 || tokens.at(i).compare("Q")==0 || tokens.at(i).compare("q")==0 || tokens.at(i).compare("T")==0 || tokens.at(i).compare("t")==0 || tokens.at(i).compare("A")==0 || tokens.at(i).compare("a")==0 || tokens.at(i).compare("z")==0) {
			command=tokens.at(i);
			i++;
		}
		
		old_x=actual_x;
		old_y=actual_y;
		//if command is absolute, set actual_x/y to zero
		if(command.compare("M")==0 || command.compare("L")==0 || command.compare("C")==0 || command.compare("S")==0 || command.compare("Q")==0 || command.compare("T")==0 || command.compare("A")==0 || command.compare("H")==0 || command.compare("V")==0) {
			actual_x=0;
			actual_y=0;
		}

		//now parse the commands
		if(command.compare("M")==0 || command.compare("m")==0){ //move to
			if(!k1.empty()) {
				k.push_front(BLine(k1, false));
				k1.clear();
			}
			//read
			actual_x+=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			actual_y+=atof(tokens.at(i).data());

			init_x=actual_x;
			init_y=actual_y;
			ax=actual_x;
			ay=actual_y;
			//operate and save
			mtx.transformPoint2D(ax,ay);
			coor2vect(&ax,&ay);
			k1.push_back(Vertex(ax,ay)); //first element
			k1.back().setSplit(true);
			//"If a moveto is followed by multiple pairs of coordinates,
			// the subsequent pairs are treated as implicit lineto commands."
			if (command.compare("M")==0)
				command="L";
			else
				command="l";
		}else if(command.compare("C")==0 || command.compare("c")==0){ //curve
			//tg2
			tgx2=actual_x+atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			tgy2=actual_y+atof(tokens.at(i).data());
			//tg1
			i++; tgx=actual_x+atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			tgy=actual_y+atof(tokens.at(i).data());
			//point
			i++; actual_x+=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			actual_y+=atof(tokens.at(i).data());

			ax=actual_x;
			ay=actual_y;
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
			if(k1.front().isFirst(ax,ay)){
				k1.front().setTg1(tgx,tgy);
			}else{
				k1.push_back(Vertex(ax,ay));
				k1.back().setTg1(tgx,tgy);
				k1.back().setSplit(true);
			}
		}else if(command.compare("Q")==0 || command.compare("q")==0){ //quadractic curve
			//tg1 and tg2
			tgx=actual_x+atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			tgy=actual_y+atof(tokens.at(i).data());
			//point
			i++; actual_x+=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			actual_y+=atof(tokens.at(i).data());

			ax=actual_x;
			ay=actual_y;
			//mtx
			mtx.transformPoint2D(ax,ay);
			mtx.transformPoint2D(tgx,tgy);
			//adjust
			coor2vect(&ax,&ay);
			coor2vect(&tgx,&tgy);
			//save
			k1.back().setTg1(tgx,tgy);
			k1.back().setSplit(false);
			k1.push_back(Vertex(ax,ay));
			k1.back().setTg1(tgx,tgy);
		}else if(command.compare("L")==0 || command.compare("l")==0){ //line to
			//point
			actual_x+=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			actual_y+=atof(tokens.at(i).data());

			ax=actual_x;
			ay=actual_y;
			//mtx
			mtx.transformPoint2D(ax,ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			k1.back().setTg2(k1.back().x,k1.back().y);
			if(k1.front().isFirst(ax,ay)){
				k1.front().setTg1(k1.front().x,k1.front().y);
			}else{
				k1.push_back(Vertex(ax,ay));
				k1.back().setTg1(k1.back().x,k1.back().y);
			}
		}else if(command.compare("H")==0 || command.compare("h")==0){// horizontal move
			//the same that L but only Horizontal movement
			//point
			actual_x+=atof(tokens.at(i).data());

			ax=actual_x;
			ay=old_y;
			//mtx
			mtx.transformPoint2D(ax,ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			k1.back().setTg2(k1.back().x,k1.back().y);
			if(k1.front().isFirst(ax,ay)){
				k1.front().setTg1(k1.front().x,k1.front().y);
			}else{
				k1.push_back(Vertex(ax,ay));
				k1.back().setTg1(k1.back().x,k1.back().y);
			}
		}else if(command.compare("V")==0 || command.compare("v")==0){//vertical
			//point
			actual_y+=atof(tokens.at(i).data());

			ax=old_x;
			ay=actual_y;
			//mtx
			mtx.transformPoint2D(ax,ay);
			//adjust
			coor2vect(&ax,&ay);
			//save
			k1.back().setTg2(k1.back().x,k1.back().y);
			if(k1.front().isFirst(ax,ay)){
				k1.front().setTg1(k1.front().x,k1.front().y);
			}else{
				k1.push_back(Vertex(ax,ay));
				k1.back().setTg1(k1.back().x,k1.back().y);
			}
		}else if(command.compare("T")==0 || command.compare("t")==0){// I don't know what does it
			actual_x+=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			actual_y+=atof(tokens.at(i).data());
		}else if(command.compare("A")==0 || command.compare("a")==0){//elliptic arc

			//isn't complete support, is only for circles

			//this curve have 6 parameters
			//radius
			float radius_x,radius_y;
			// todo: why 'angle' never used?
			//float angle;
			bool sweep,large;
			//radius
			radius_x=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			radius_y=atof(tokens.at(i).data());
			//angle
			// todo: why 'angle' never used?
			i++; // angle=atof(tokens.at(i).data());
			//flags
			i++; large=atoi(tokens.at(i).data());
			i++; sweep=atoi(tokens.at(i).data());
			//point
			i++; actual_x+=atof(tokens.at(i).data());
			i++; if(tokens.at(i).compare(",")==0) i++;
			actual_y+=atof(tokens.at(i).data());
			//how to draw?
			if(!large && !sweep){
				//points
				tgx2 = old_x + radius_x*0.5;
				tgy2 = old_y ;
				tgx  = actual_x;
				tgy  = actual_y + radius_y*0.5;

				ax=actual_x;
				ay=actual_y;
				//transformations
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
				if(k1.front().isFirst(ax,ay)){
					k1.front().setTg1(tgx,tgy);
				}else{
					k1.push_back(Vertex (ax,ay));
					k1.back().setTg1(tgx,tgy);
					k1.back().setSplit(true);
				}
			}else if(!large &&  sweep){
				//points
				tgx2 = old_x;
				tgy2 = old_y + radius_y*0.5;
				tgx  = actual_x + radius_x*0.5;
				tgy  = actual_y ;

				ax=actual_x;
				ay=actual_y;
				//transformations
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
				if(k1.front().isFirst(ax,ay)){
					k1.front().setTg1(tgx,tgy);
				}else{
					k1.push_back(Vertex(ax,ay));
					k1.back().setTg1(tgx,tgy);
					k1.back().setSplit(true);
				}
			}else if( large && !sweep){//rare
				//this need more than one vertex
			}else if( large &&  sweep){//circles in inkscape are made with this kind of arc
				//intermediate point
				int sense=1;
				if(old_x>actual_x) sense =-1;
				float in_x,in_y,in_tgx1,in_tgy1,in_tgx2,in_tgy2;
				in_x = (old_x+actual_x)/2;
				in_y = old_y - sense*radius_y;
				in_tgx1 = in_x - sense*(radius_x*0.5);
				in_tgx2 = in_x + sense*(radius_x*0.5);
				in_tgy1 = in_y;
				in_tgy2 = in_y;
				//start/end points
				tgx2=old_x;
				tgy2=actual_y - sense*(radius_y*0.5);
				tgx =actual_x;
				tgy =actual_y - sense*(radius_y*0.5);

				ax=actual_x;
				ay=actual_y;
				//transformations
				if(!mtx.is_identity()){
					mtx.transformPoint2D(tgx2,tgy2);
					mtx.transformPoint2D(tgx ,tgy );
					mtx.transformPoint2D(ax,ay);

					mtx.transformPoint2D(in_tgx2,in_tgy2);
					mtx.transformPoint2D(in_tgx1,in_tgy1);
					mtx.transformPoint2D(in_x,in_y);
				}
				//adjust
				coor2vect(&tgx2 , &tgy2);
				coor2vect(&ax   , &ay  );
				coor2vect(&tgx  , &tgy );

				coor2vect(&in_tgx2 , &in_tgy2);
				coor2vect(&in_tgx1 , &in_tgy1);
				coor2vect(&in_x    , &in_y   );

				//save the last tg2
				k1.back().setTg2(tgx2,tgy2);
				//save the intermediate point
				k1.push_back(Vertex(in_x,in_y));
				k1.back().setTg1( in_tgx1 , in_tgy1);
				k1.back().setTg2( in_tgx2 , in_tgy2);
				k1.back().setSplit(true); //this could be changed
				//save the new point
				if(k1.front().isFirst(ax,ay)){
					k1.front().setTg1(tgx,tgy);
				}else{
					k1.push_back(Vertex(ax,ay));
					k1.back().setTg1(tgx,tgy);
					k1.back().setSplit(true);
				}
			}
		}else if(command.compare("z")==0){
			k.push_front(BLine(k1, true));
			k1.clear();
			if (i<tokens.size() && tokens.at(i).compare("M")!=0 && tokens.at(i).compare("m")!=0) {
				//starting a new path, but not with a moveto
				actual_x=init_x;
				actual_y=init_y;
				ax=actual_x;
				ay=actual_y;
				//operate and save
				mtx.transformPoint2D(ax,ay);
				coor2vect(&ax,&ay);
				k1.push_back(Vertex(ax,ay)); //first element
				k1.back().setSplit(true);
			}
			i--; //decrement i to balance "i++" at command change
		}else{
			std::cout<<"unsupported path token: "<<tokens.at(i).c_str()<<std::endl;
		}
	}
	if(!k1.empty()) {
		k.push_front(BLine(k1, false)); //last element
	}
	return k;
}

/* === EFFECTS PARSERS ===================================================== */

void
Svg_parser::parser_effects(const xmlpp::Element* /*nodeElement*/, xmlpp::Element* root, const String& /*parent_style*/, const SVGMatrix& mtx)
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
		build_param (child_transform->add_child("param"),"horizon","real","4.0");
	}
}

std::list<ColorStop>
Svg_parser::get_colorStop(String name)
{
	const std::list<ColorStop> none;

	if(!name.empty()){
		if(lg.empty()&& rg.empty())
			return none;

		String find= name;
		if(find.at(0)=='#') find.erase(0,1);
		else return none;
		std::list<LinearGradient>::iterator aux=lg.begin();
		while(aux!=lg.end()){//only find into linear gradients
			if(find.compare(aux->name)==0){
				return aux->stops;
			}
			aux++;
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
		String find= name.substr(start,end-start);
		bool encounter=false;
		if(!lg.empty()){
			std::list<LinearGradient>::iterator aux=lg.begin();
			while(aux!=lg.end()){
				if(find.compare(aux->name)==0){
					build_linearGradient(root, *aux, mtx);
					encounter=true;
				}
				aux++;
			}
		}
		if(!encounter && !rg.empty()){
			std::list<RadialGradient>::iterator aux=rg.begin();
			while(aux!=rg.end()){
				if(find.compare(aux->name)==0){
					build_radialGradient(root, *aux, mtx);
					encounter=true;
				}
				aux++;
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
			std::cout<<"SVG Import warning: gradient points equal each other"<<std::endl;
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
						Glib::ustring style	=nodeIter->get_attribute_value("style");
						float offset=atof(nodeIter->get_attribute_value("offset").data());
						String stop_color;
						String opacity;
						if(!style.empty()){
							extractSubAttribute (style,"stop-color",stop_color);
							extractSubAttribute (style,"stop-opacity",opacity);
						}
						if(opacity.empty()) opacity="1";
						if(stop_color.empty()) stop_color="#000000";//black for default :S
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
			std::cout<<"SVG Parser: ignoring focus attributes for radial gradient";

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
	if(p.split) build_param (child_comp->add_child("split"),"","bool","true");
	else build_param (child_comp->add_child("split"),"","bool","false");
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
		printf("Color aborted\n");
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
Svg_parser::nodeStartBasicLayer(xmlpp::Element* root, const String& name)
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
	if(dx>0 && dy>0){
		ag=PI + atan(dy/dx);
	}else if(dx>0 && dy<0){
		ag=PI + atan(dy/dx);
	}else if(dx<0 && dy<0){
		ag=atan(dy/dx);
	}else if(dx<0 && dy>0){
		ag= 2*PI+atan(dy/dx);
	}else if(approximate_zero(dx) && dy>0){
		ag=-1*PI/2;
	}else if(approximate_zero(dx) && dy<0){
		ag=PI/2;
	}else if(approximate_zero(dx) && approximate_zero(dy)){
		ag=0;
	}else if(dx<0 && approximate_zero(dy)){
		ag=0;
	}else if(dx>0 && approximate_zero(dy)){
		ag=PI;
	}
	ag= (ag*180)/PI;
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
	if(dx>0 && dy>0){
		ag=PI + atan(dy/dx);
	//	printf("case 180-270\n");
	}else if(dx>0 && dy<0){
		ag=PI + atan(dy/dx);
	//	printf("case 90-180\n");
	}else if(dx<0 && dy<0){
		ag=atan(dy/dx);
	//	printf("case 0-90\n");
	}else if(dx<0 && dy>0){
		ag= 2*PI+atan(dy/dx);
	//	printf("case 270-360\n");
	}else if(approximate_zero(dx) && dy>0){
		ag=-1*PI/2;
	}else if(approximate_zero(dx) && dy<0){
		ag=PI/2;
	}else if(approximate_zero(dx) && approximate_zero(dy)){
		ag=0;
	}else if(dx<0 && approximate_zero(dy)){
		ag=0;
	}else if(dx>0 && approximate_zero(dy)){
		ag=PI;
	}
	ag= (ag*180)/PI;
	ag=ag-180;

	this->radius2=rd;
	this->angle2=ag;
}

void
Vertex::setSplit(bool val)
{
	split=val;
}

bool
Vertex::isFirst(float a, float b) const
{
	return approximate_equal(x, a) && approximate_equal(y, b);
}

Vertex::Vertex(float x,float y)
	: x(x), y(y),
	  radius1(0), angle1(0),
	  radius2(0), angle2(0),
	  split(false)
{
}

//matrices
void
SVGMatrix::parser_transform(String transform)
{
	bool first_iteration = true;
	SVGMatrix a;

	String tf(transform);
	removeIntoS(tf);
	std::vector<String> tokens=tokenize(tf," ");
	for (const String& token : tokens) {
		if(token.compare(0,9,"translate")==0){
			float dx,dy;
			int start,end;
			start	=token.find_first_of("(")+1;
			end		=token.find_first_of(",");
			dx		=atof(token.substr(start,end-start).data());
			start	=token.find_first_of(",")+1;
			end		=token.size()-1;
			dy		=atof(token.substr(start,end-start).data());
			if (first_iteration)
				a = SVGMatrix(1,0,0,1,dx,dy);
			else
				a.multiply(SVGMatrix(1,0,0,1,dx,dy));
		}else if(token.compare(0,5,"scale")==0){
			if (first_iteration)
				a = SVGMatrix(1,0,0,1,0,0);
		}else if(token.compare(0,6,"rotate")==0){
			float angle,seno,coseno;
			int start,end;
			start	=token.find_first_of("(")+1;
			end		=token.size()-1;
			angle=getRadian (atof(token.substr(start,end-start).data()));
			seno   =sin(angle);
			coseno =cos(angle);
			if (first_iteration)
				a = SVGMatrix(coseno,seno,-1*seno,coseno,0,0);
			else
				a.multiply(SVGMatrix(coseno,seno,-1*seno,coseno,0,0));
		}else if(token.compare(0,6,"matrix")==0){
			int start	=token.find_first_of('(')+1;
			int end		=token.find_first_of(')');
			if (first_iteration)
				a = SVGMatrix(token.substr(start,end-start));
			else
				a.multiply(SVGMatrix(token.substr(start,end-start)));
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

const SVGMatrix SVGMatrix::indentity(1,0,0,1,0,0);
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

static int
extractSubAttribute(const String& attribute, const String& name, String& value)
{
	int encounter=0;
	if(!attribute.empty()){
		String str = synfig::trim(attribute);
		std::vector<String> tokens=tokenize(str,";");
		std::vector<String>::iterator aux=tokens.begin();
		while(aux!=tokens.end()){
			int mid= (*aux).find_first_of(":");
			if((*aux).substr(0,mid).compare(name)==0){
				int end=(*aux).size();
				value=(*aux).substr(mid+1,end-mid);
				return 1;
			}
			aux++;
		}
	}
	return encounter;
}

static String
loadAttribute(const String& name, const String& path_style, const String& master_style, String defaultVal)
{
	String value;
	int fnd=0;
	if(!path_style.empty())
		fnd=extractSubAttribute(path_style,name,value);
	if(fnd==0){
		if(!master_style.empty())
			fnd=extractSubAttribute(master_style,name,value);
		if(fnd==0)
			value=defaultVal;
	}
	return value;
}
static String
loadAttribute(const String& name, const String& path_style, const String& master_style, String subattribute, String defaultVal)
{
	String value;
	int fnd=0;
	if(!path_style.empty())
		fnd=extractSubAttribute(path_style,name,value);
	if(fnd==0 && !master_style.empty())
			fnd=extractSubAttribute(master_style,name,value);
	if(fnd==0){
		if(!subattribute.empty())
			value=subattribute;
		else
			value=defaultVal;
	}
	return value;
}

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
					else if(a==','){ e=19; i++;}
					else if(a==' '){i++;}
					else {
						synfig::warning("unknown token in SVG path '%c'", a);
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
						e=20;
					}
					break;
			case 19: tokens.push_back(","); e=0; break;
			case 20: tokens.push_back(buffer);
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
		case 19: tokens.push_back(","); break;
		case 20: tokens.push_back(buffer); break;
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

static double
getDimension(const String& ac)
{
	if(ac.empty()){
		return 0;
	}
	auto length=ac.size();
	double af=0;
	if(isdigit(ac.at(length-1))){
		af=atof(ac.data());
	}else if(ac.at(length-1)=='%'){
			return 1024;
	}else{
		String mtc=ac.substr(length-2,length);
		String nmc=ac.substr(0,length-2);
		if(mtc.compare("px")==0){
			af=atof(nmc.data());
		}else if(mtc.compare("pt")==0){
			af=atof(nmc.data())*1.25;
		}else if(mtc.compare("em")==0){
			af=atof(nmc.data())*16;
		}else if(mtc.compare("mm")==0){
			af=atof(nmc.data())*3.54;
		}else if(mtc.compare("pc")==0){
			af=atof(nmc.data())*15;
		}else if(mtc.compare("cm")==0){
			af=atof(nmc.data())*35.43;
		}else if(mtc.compare("in")==0){
			af=atof(nmc.data())*90;
		}else{
			return 1024;
		}
	}
	return af;
}

static float
getRadian(float sexa){
	return (sexa*2*PI)/360;
}

static void
removeIntoS(String& input){
	bool into=false;
	for(unsigned int i=0;i<input.size();i++){
		if(input.at(i)=='('){
			into=true;
		}else if(input.at(i)==')'){
			into=false;
		}else if(into && input.at(i)==' '){
			input.erase(i,1);
			i--;
		}
	}
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
String
Svg_parser::new_guid(){
	uid++;
	return GUID::hasher(uid).get_string();
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
